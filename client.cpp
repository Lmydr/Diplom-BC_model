#include "client.h"

#include <openssl/ecdsa.h>
#include <openssl/crypto.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#include <QCryptographicHash>
#include <QThread>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDateTime>


#include <QDebug>

#include <QDataStream>

#include <QTimer>



QDataStream & operator << (QDataStream & in, Transaction & str){
    in<<str.sender.addr;
    in<<str.sender.count;
    for(int i =0; i<str.reseivers.length();i++){
        in<<str.reseivers.at(i).addr;
        in<<str.reseivers.at(i).count;
    }
    return in;
}
/*
QDataStream & operator >> (QDataStream & out, Trasaction & str){

}
*/


Client::Client(QObject *parent): QObject(parent)
{
    qRegisterMetaType<Block>();
    qRegisterMetaType<SignTransaction>();
}

int Client::init(QByteArray sk, QByteArray pk, QByteArray folderName, int numNode)
{
    nodeNumber = numNode;
    folder = folderName;
    secretKey = sk;
    publicKey = pk;
    address = QCryptographicHash::hash(publicKey,QCryptographicHash::Algorithm::Sha256).toBase64();

    group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    key = EC_KEY_new_by_curve_name(NID_secp256k1);

    BIGNUM *k = BN_new();
    BN_hex2bn(&k,secretKey.data());
    EC_KEY_set_private_key(key,k);

    EC_POINT *p_key_point = EC_POINT_new(group);
    EC_POINT_hex2point(group,pk,p_key_point,NULL);
    EC_KEY_set_public_key(key,p_key_point);

    db = QSqlDatabase::addDatabase("QSQLITE",publicKey);
    db.setDatabaseName("./"+ folder+ "/" + publicKey + ".sqlite");

    if(!db.open()){
        qDebug()<<"Error! db was not opened";
    }

    WalletStr str;
    str.addr = address;
    str.balance = 0;
    str.p_key = publicKey;
    str.s_key = secretKey;

    hashThread = new QThread();
    creator = new HashBlockCreator();
    connect(this,&Client::stopCreateHashBlock,creator,&HashBlockCreator::stop);
    connect(this,&Client::startCreateHashBlock,creator,&HashBlockCreator::start);
    connect(creator,&HashBlockCreator::createdCorrectHash,this,&Client::blockHashCreated);

    hashThread->start();
    creator->moveToThread(hashThread);

    emit nodeCreated(nodeNumber,str);

    return 1;
}

QByteArray Client::makeSign(Transaction t)
{
    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream<<t;
    QByteArray hashTr = QCryptographicHash::hash(arr,QCryptographicHash::Algorithm::Sha256);

    //qDebug()<<"TrHash #"<<nodeNumber<<" "<<hashTr;
    QByteArray resultSign;


    int maxsize_sign = ECDSA_size(key);
    unsigned char* sign = new unsigned char[maxsize_sign];
    unsigned int sign_size;

    if(ECDSA_sign(0,(unsigned char*)hashTr.data(),hashTr.length(),sign,&sign_size,key)==1){
        //qDebug()<<"all right";
    }
    resultSign = resultSign.fromRawData((char*)sign,sign_size);
    //qDebug()<<"Signature: "<< resultSign.toBase64();

    if(ECDSA_verify(0,(unsigned char*)hashTr.data(),hashTr.length(),(unsigned char*)resultSign.data(),resultSign.length(),key) == 1){
        //qDebug()<<"true";
    }else{
        //qDebug()<<"false";
    }


    return resultSign;
}

bool Client::checkTrInBlock(QList<SignTransaction> transactions)
{
    for(int i=0;i<transactions.length();i++){
        if(!checkTr(transactions.at(i))){
            return false;
        }
    }
    return true;
}

bool Client::checkTr(SignTransaction tr)
{
    bool result = false;

    EC_GROUP *check_group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_KEY   *check_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_POINT *check_key_point = EC_POINT_new(check_group);

    const char* hexKey = tr.p_key.data();

    EC_POINT_hex2point(check_group,hexKey,check_key_point,NULL);
    EC_KEY_set_public_key(check_key,check_key_point);

    Transaction tmp;
    tmp.sender = tr.sender;
    tmp.reseivers = tr.reseivers;

    if(tmp.sender.addr != "CoinBase"){
        if(getBalance(tmp.sender.addr)< tmp.sender.count){
            return false;
        }
    }

    QByteArray tmpArr;
    QDataStream stream(&tmpArr,QIODevice::WriteOnly);
    stream<<tmp;
    QByteArray hashTr = QCryptographicHash::hash(tmpArr,QCryptographicHash::Algorithm::Sha256);

    const unsigned char* hash4check = (unsigned char*)hashTr.data();
    const unsigned char* sign4check = (unsigned char*)tr.sign.data();

    if(ECDSA_verify(0,hash4check,hashTr.length(),sign4check,tr.sign.length(),check_key) <= 0){
        qDebug()<<"Error";
        result = false;
    }else{
        qDebug()<<"Ok";
        result = true;
    }
    //qDebug()<<"code:"<<ECDSA_verify(0,hash4check,hashTr.length(),sign4check,tr.sign.length(),check_key);

    char *err_buff2 = new char[1024];
    ERR_error_string(ERR_get_error(),err_buff2);
    qDebug()<<err_buff2;

    return result;
}

QByteArray Client::makeMerkleRoot(QList<SignTransaction> transactions)
{
    if(transactions.length()%2!=0){
        transactions.append(transactions.last());
    }

    QByteArrayList hashes;

    for(int i = 0;i<transactions.length();i++){
        Transaction tmp;
        tmp.sender = transactions.at(i).sender;
        tmp.reseivers = transactions.at(i).reseivers;

        QByteArray tmpArr;
        QDataStream stream(&tmpArr,QIODevice::WriteOnly);
        stream<<tmp;
        QByteArray hashTr = QCryptographicHash::hash(tmpArr,QCryptographicHash::Algorithm::Sha256);
        hashes.append(hashTr);
    }

    do{
        if(hashes.length()%2==0){
            for(int i = 0;i<hashes.length();i++){
                hashes[i] = QCryptographicHash::hash(hashes.at(i) + hashes.at(i+1),QCryptographicHash::Algorithm::Sha256);
                hashes.removeAt(i+1);
            }
        }else{
            hashes.append(hashes.last());
        }
    }while(hashes.length()!=1);

    return hashes.at(0);
}

QByteArray Client::makeTransactionHash(Transaction tr)
{
    QByteArray tmpArr;
    QDataStream stream(&tmpArr,QIODevice::WriteOnly);
    stream<<tr;

    QByteArray hashTr = QCryptographicHash::hash(tmpArr,QCryptographicHash::Algorithm::Sha256);
    return hashTr;
}

QByteArray Client::makeTransactionHash(SignTransaction tr)
{
    Transaction t;
    t.sender = tr.sender;
    t.reseivers = tr.reseivers;
    return makeTransactionHash(t);
}

QByteArray Client::getHashLastBlock()
{
    QSqlQuery q = db.exec();
    q.exec("SELECT CurrentHash FROM Blocks ORDER BY Block_id DESC LIMIT 1");
    q.next();
    QByteArray hash = QByteArray::fromHex(q.value(0).toByteArray());
    return hash;
}

qint64 Client::getBlockID(QByteArray hash)
{
    QSqlQuery q = db.exec();
    q.exec("SELECT Block_ID FROM Blocks WHERE CurrentHash = "  "\'" + hash.toHex() + "\'");
    q.next();
    qint64 result = q.value(0).toInt();
    return result;

}

double Client::getSenderSum(QByteArray addr)
{
    QSqlQuery q = db.exec();
    q.exec("SELECT Sum (tokens_count) FROM Senders WHERE sender_address = \'" + addr + "\'");
    q.next();
    double result = q.value(0).toDouble();
    return result;
}

double Client::getReceiverSum(QByteArray addr)
{
    QSqlQuery q = db.exec();
    q.exec("SELECT Sum (tokens_count) FROM Receivers WHERE Receiver_address = \'" + addr + "\'");
    q.next();
    double result = q.value(0).toDouble();
    return result;
}

double Client::getBalance(QByteArray addr)
{
    return getReceiverSum(addr) - getSenderSum(addr);
}

void Client::insertTransactions(QList<SignTransaction> list, QByteArray blockHash)
{
    QSqlQuery query = db.exec();
    QByteArray numBlock = QByteArray::number(getBlockID(blockHash));

    for(int i = 0; i< list.length();i++){
        if(!query.exec("INSERT INTO Transactions("
                       "Block_id,"
                       "Transaction_Hash,"
                       "PublicKey,"
                       "Signature,"
                       "Summ"
                       ")"
                       "VALUES (" "\'" + numBlock + "\'"
                       ", \'" + QCryptographicHash::hash(makeTransactionHash(list.at(i))+blockHash,QCryptographicHash::Algorithm::Sha256).toHex() +"\'"
                       ", \'" + list.at(i).p_key +"\'"
                       ", \'" + list.at(i).sign.toHex() +"\'"
                       ", \'" + QByteArray::number(list.at(i).sender.count) +"\'"
                       ")"))
        {
            qDebug()<<query.lastError().text();
        };

        if(!query.exec("INSERT INTO Senders ("
                       "Transaction_Hash,"
                       "sender_address,"
                       "tokens_count"
                       ")"
                       "VALUES (" "\'" + QCryptographicHash::hash(makeTransactionHash(list.at(i))+blockHash,QCryptographicHash::Algorithm::Sha256).toHex() + "\'"
                       ", \'" + list.at(i).sender.addr +"\'"
                       ", \'" + QByteArray::number(list.at(i).sender.count)+"\'"
                       ")"))
        {
            qDebug()<<query.lastError().text();
        };

        for(int j = 0; j<list.at(i).reseivers.length();j++){
            if(!query.exec("INSERT INTO Receivers ("
                           "Transaction_Hash,"
                           "Receiver_address,"
                           "tokens_count"
                           ")"
                           "VALUES (" "\'" + QCryptographicHash::hash(makeTransactionHash(list.at(i))+blockHash,QCryptographicHash::Algorithm::Sha256).toHex() + "\'"
                           ", \'" + list.at(i).reseivers.at(j).addr +"\'"
                           ", \'" + QByteArray::number(list.at(i).reseivers.at(j).count)+"\'"
                           ")"))
            {
                qDebug()<<query.lastError().text();
            };
        }
    }

}

void Client::makeBlock()
{
    //makeTransaction(5,"anyAddr");

    block4send.prevBlockHash = getHashLastBlock();
    block4send.transactions = makePackTransactions();//add transactions
    block4send.merkleRoot = makeMerkleRoot(block4send.transactions);
    block4send.nonce = -1;

    headBlockForHash str;
    str.prevBlockHash = block4send.prevBlockHash;
    str.merkleRoot = block4send.merkleRoot;
    str.timeStamp = QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8();;
    str.nonce = -1;

    creator->init(str);
    emit startCreateHashBlock();


}

//void Client::makeTransaction(double count, QByteArray addrReceiver)
//{
//    if(count < getBalance(address)){
//        Transaction tr;
//        tr.sender.addr = address;
//        tr.sender.count = count;

//        Receiver r;
//        r.addr = addrReceiver;
//        r.count = count;
//        tr.reseivers.append(r);

//        SignTransaction t;
//        t.p_key = publicKey;
//        t.reseivers.append(r);
//        t.sender.addr = address;
//        t.sender.count = count;
//        t.sign = makeSign(tr);

//        emit transaction(t);
//    }

//}

QList<SignTransaction> Client::makePackTransactions()
{
    QList<SignTransaction> list;

    //default coin base transaction
    Transaction tr;
    tr.sender.addr = "CoinBase";
    tr.sender.count = 50;
    Receiver r;
    r.addr = address;
    r.count = 50;
    tr.reseivers.append(r);

    SignTransaction t;
    t.p_key = publicKey;
    t.reseivers.append(r);
    t.sender.addr = "CoinBase";
    t.sender.count = 50;
    t.sign = makeSign(tr);
    list.append(t);

    //add from buffer

    if(!TrasactionBuffer.isEmpty()){
        for(int i=0;i<TrasactionBuffer.length() && i<3;i++){
            list.append(TrasactionBuffer.takeFirst());
        }
    }

    return list;
}

void Client::checkBlock(Block block)
{
    emit logMessage(nodeNumber,"-----------------------------------------\n"
                               "Пришел блок на проверку!\n"
                               "Информация о блоке:\n"
                               "Создатель: " + block.transactions.first().reseivers.first().addr + "\n"
                               "Текущий хэш: " + block.currentBlockHash.toHex() + "\n"
                               "Хэш предыдущего блока: " + block.prevBlockHash.toHex() + "\n"
                               "Перебрано хэшей: " + QByteArray::number(block.nonce) + "\n"
                               "Метка времени: "+ block.timeStamp + "\n"
                               "Корень Меркла: "+ block.merkleRoot.toHex() + "\n"
                               "-----------------------------------------\n"
                               "Начинаю проверку!\n");



    bool levelFlag = true;

    QByteArray checkBytes = block.currentBlockHash.toHex();
    checkBytes.remove(8,checkBytes.length());


    for(int i = 0; i < hashLevel;i++){
        if(checkBytes.at(i)!= '0'){
            levelFlag = false;
        }
    }

    if(!levelFlag){
        emit logMessage(nodeNumber,"Проверка не пройдена, не достаточная сложность хэша!\n");
        return;
    }

    if(getHashLastBlock()!= block.prevBlockHash){
        emit logMessage(nodeNumber,"Проверка не пройдена, хэш предыдущего блока не верен!\n");
        return;
    }

    if(checkTrInBlock(block.transactions)){
        if(block.merkleRoot == makeMerkleRoot(block.transactions)){
            headBlockForHash s;
            s.prevBlockHash = block.prevBlockHash;
            s.merkleRoot = block.merkleRoot;
            s.nonce = block.nonce;
            s.timeStamp = block.timeStamp;

            if(block.currentBlockHash == creator->MakeHash(s)){
                qDebug()<<"WTF all right";
                QSqlQuery query = db.exec();

                if(!query.exec("INSERT INTO Blocks("
                               "PreviousHash,"
                               "CurrentHash,"
                               "Nonce,"
                               "MerkleRoot,"
                               "time"
                               ")"
                               "VALUES (" "\'" + getHashLastBlock().toHex() + "\'"
                               ", \'" + block.currentBlockHash.toHex() +"\'"
                               ", \'" + QByteArray::number(block.nonce) +"\'"
                               ", \'" + block.merkleRoot.toHex() +"\'"
                               ", \'" + block.timeStamp +"\'"
                               ")"))
                {
                    qDebug()<<"insert block faild";
                    qDebug()<<query.lastError().text();
                }else{
                    insertTransactions(block.transactions,block.currentBlockHash);

                    emit logMessage(nodeNumber,"Проверка блока прошла успешно!\n"
                                               "Блок добавлен в базу!\n"
                                               "Хэш блока:"+ block.currentBlockHash.toHex() + "\n");

//                    for(int i = 0; block.transactions.length();i++){
//                        for(int j=0; TrasactionBuffer.length();j++){
//                            if(block.transactions.at(i). == TrasactionBuffer.at(j)){

//                            }
//                        }
//                    }

                    balance = getBalance(address);
                }

                emit changeBalance(getBalance(address),nodeNumber);



            }
            else{

                emit logMessage(nodeNumber,"Проверка не пройдена, хэш предыдущего блока не верен!\n");
                qDebug()<<"Hash";
            }

        }
        else{
            emit logMessage(nodeNumber,"Проверка не пройдена, ошибка в корне Меркла!\n");
                qDebug()<<"Merkle";
                qDebug()<<block.merkleRoot.toBase64();
                qDebug()<<makeMerkleRoot(block.transactions).toBase64();
        }
    }else{
        emit logMessage(nodeNumber,"Проверка не пройдена, включена неверная транзакция!\n");
        qDebug()<<"transactions";
    }

    if(!stopMakeBlocks)
        startBlock();

}

void Client::checkTransaction(SignTransaction tr)
{

    emit logMessage(nodeNumber,"Пришла транзакция!\n"
                               "Начинаю проверку!\n");


    if(tr.sender.count < getBalance(tr.sender.addr)){

        EC_GROUP *check_group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        EC_KEY   *check_key = EC_KEY_new_by_curve_name(NID_secp256k1);
        EC_POINT *check_key_point = EC_POINT_new(check_group);

        const char* hexKey = tr.p_key.data();

        EC_POINT_hex2point(check_group,hexKey,check_key_point,NULL);
        EC_KEY_set_public_key(check_key,check_key_point);

        Transaction tmp;
        tmp.sender = tr.sender;
        tmp.reseivers = tr.reseivers;

        QByteArray tmpArr;
        QDataStream stream(&tmpArr,QIODevice::WriteOnly);
        stream<<tmp;
        QByteArray hashTr = QCryptographicHash::hash(tmpArr,QCryptographicHash::Algorithm::Sha256);

        //qDebug()<<"TrHash on check"<<nodeNumber<<" "<<hashTr;
        //qDebug()<<"Sign:"<<tr.sign.toBase64();
        //qDebug()<<"PubKey4check"<<tr.p_key;

        const unsigned char* hash4check = (unsigned char*)hashTr.data();
        const unsigned char* sign4check = (unsigned char*)tr.sign.data();

        if(ECDSA_verify(0,hash4check,hashTr.length(),sign4check,tr.sign.length(),check_key) <= 0){
            qDebug()<<"Error! Wrong sign";
            emit logMessage(nodeNumber,"Проверка не пройдена! Неверная сигнатура.\n");
        }else{
            qDebug()<<"Ok";
            TrasactionBuffer.append(tr);
            emit logMessage(nodeNumber,"Проверка пройдена! Транзакция добавлена в очередь.\n");
        }
        qDebug()<<"code:"<<ECDSA_verify(0,hash4check,hashTr.length(),sign4check,tr.sign.length(),check_key);

        char *err_buff2 = new char[1024];
        ERR_error_string(ERR_get_error(),err_buff2);
        qDebug()<<err_buff2;
    }else{
        emit logMessage(nodeNumber,"Проверка не пройдена! Недостаточно токенов.\n");
        qDebug()<<"balance wrong";
    }
}

void Client::startBlock()
{

    emit logMessage(nodeNumber,"Начинаю непрерывно считать блоки\n");
    stopMakeBlocks = false;
    makeBlock();

//    Transaction tr;
//    tr.sender.addr = "CoinBase";
//    tr.sender.count = 50;
//    Receiver r;
//    r.addr = address;
//    r.count = 50;
//    tr.reseivers.append(r);

//    QList<SignTransaction> list;

//    SignTransaction t;
//    t.p_key = publicKey;
//    t.reseivers.append(r);
//    t.sender.addr = "CoinBase";
//    t.sender.count = 50;
//    t.sign = makeSign(tr);
//    list.append(t);

//    block4send.transactions = list;


////    QByteArray tmpArr;
////    QDataStream stream(&tmpArr,QIODevice::WriteOnly);
////    stream<<tr;

////    QByteArray hashTr = QCryptographicHash::hash(tmpArr,QCryptographicHash::Algorithm::Sha256);

////    QByteArray merkleArr = QCryptographicHash::hash((hashTr+hashTr),QCryptographicHash::Algorithm::Sha256);

//    QByteArray merkleArr = makeMerkleRoot(list);

//    //qDebug()<<"test: "<<merkleArr.toBase64();
//    qDebug()<<"function: "<<makeMerkleRoot(list).toBase64();


//    headBlockForHash str;
//    str.prevBlockHash = getHashLastBlock();
//    str.merkleRoot = merkleArr;
//    str.timeStamp = QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8();;
//    str.nonce = -1;


//    creator->init(str);
//    emit startCreateHashBlock();


}

void Client::stopCreated()
{
    creator->stop();
    //hashThread->quit();
}

void Client::stopMake()
{
    emit logMessage(nodeNumber,"Непрерывное высчитывание блоков остановлено!\n");
    stopMakeBlocks = true;
    creator->stop();
    qDebug()<<"making blocks stoped";
}

void Client::makeOneBlock(int num)
{
    qDebug()<<"NumNode: "<<num;
    //if(num == nodeNumber){
        emit logMessage(nodeNumber,"Начинаю высчитывать блок\n");
        makeBlock();
    //}
}

void Client::makeTransaction(int num, double count, QByteArray addrReceiver)
{
    if(count <= balance && num == nodeNumber){
        Transaction tr;
        tr.sender.addr = address;
        tr.sender.count = count;

        Receiver r;
        r.addr = addrReceiver;
        r.count = count;
        tr.reseivers.append(r);

        SignTransaction t;
        t.p_key = publicKey;
        t.reseivers.append(r);
        t.sender.addr = address;
        t.sender.count = count;
        t.sign = makeSign(tr);

        balance = balance - count;

        emit transaction(t);
        //emit changeBalance(balance,nodeNumber);
    }else{
        emit logMessage(nodeNumber,"Транзакция не отправлена, недостаточный баланс.\n");
    }
}

void Client::setHashLevel(int num)
{
    hashLevel = num;
    creator->setLevel(num);
}

void Client::blockHashCreated(QByteArray Inhash)
{
    QByteArray hash = Inhash;
    qDebug()<<"I am first"<<nodeNumber;
    qDebug()<<hash;
    qDebug()<<hash.toHex(':');
    qDebug()<<"Nonce"<<creator->getLastNonce();
    emit blockCreated();
    headBlockForHash s = creator->getHeaderBlock();
    block4send.currentBlockHash = hash;
    block4send.prevBlockHash = s.prevBlockHash;
    block4send.timeStamp = s.timeStamp;
    block4send.merkleRoot = s.merkleRoot;
    block4send.nonce = s.nonce;
    emit logMessage(nodeNumber,"-----------------------------------------\n"
                               "Посчитан блок!\n"
                               "Информация о блоке:\n"
                               "Текущий хэш: " + block4send.currentBlockHash.toHex() + "\n"
                               "Хэш предыдущего блока: " + block4send.prevBlockHash.toHex() + "\n"
                               "Перебрано хэшей: " + QByteArray::number(block4send.nonce) + "\n"
                               "Метка времени: "+ block4send.timeStamp + "\n"
                               "Корень Меркла: "+ block4send.merkleRoot.toHex() + "\n"
                               "-----------------------------------------\n" );

    emit block4check(block4send);
}
