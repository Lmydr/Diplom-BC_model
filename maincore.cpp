#include "maincore.h"
#include <QFile>
#include <QDir>

#include <openssl/ecdsa.h>
#include <openssl/crypto.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#include <QCryptographicHash>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>
#include <QThread>

MainCore::MainCore(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE","mainThread");

    qRegisterMetaType<Block>();
    qRegisterMetaType<SignTransaction>();
}

MainCore::~MainCore()
{
    for(int i=0;i<clients.length();i++){
        delete clients.at(i);
    }

    for(int i=0;i<clientsThreads.length();i++){
        delete clientsThreads.at(i);
    }
}

void MainCore::createNewBC()
{
    QFile confFile("newBC.conf");
    confFile.open(QIODevice::WriteOnly);
    confFile.write("/newBC");
    confFile.close();
    QDir dir;
    dir.mkdir("newBC");
    BCname = "newBC";
}

void MainCore::addNode(int num)
{
    EC_KEY *testKey = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_generate_key(testKey);

    QByteArray privat_key = BN_bn2hex(EC_KEY_get0_private_key(testKey));
    QByteArray public_key = EC_POINT_point2hex(EC_KEY_get0_group(testKey),EC_KEY_get0_public_key(testKey),POINT_CONVERSION_COMPRESSED,NULL);
    QByteArray address = QCryptographicHash::hash(public_key,QCryptographicHash::Algorithm::Sha256).toBase64();


    db.setDatabaseName("./" + BCname + "/" + public_key + ".sqlite");

    if(!db.open()){
        qDebug()<<"Error! db was not opened";
    }else{
        QSqlQuery query = db.exec();

        if(!query.exec("PRAGMA foreign_keys=on")){
            qDebug()<<"Error ForeignKeys";
            qDebug()<<query.lastError().text();
        }

        if(!query.exec("CREATE TABLE IF NOT EXISTS  AccountTable("
                       "secretKey   BLOB NOT NULL,"
                       "publicKey   BLOB NOT NULL,"
                       "address     BLOB NOT NULL"
                       ")"))
        {
            qDebug()<<"Error where create a table";
            qDebug()<<query.lastError().text();
        }

        if(!query.exec("INSERT INTO AccountTable("
                   "secretKey,"
                   "publicKey,"
                   "address"
                   ")"
                   "VALUES (" "\'" + privat_key + "\'" + "," "\'"+ public_key +"\'" ",""\'" + address +"\'"")"))
        {
            qDebug()<<query.lastError().text();
        };

        if(!query.exec(
                    "CREATE TABLE IF NOT EXISTS Blocks ("
                    "Block_ID	INTEGER NOT NULL UNIQUE,"
                    "PreviousHash	BLOB NOT NULL UNIQUE,"
                    "CurrentHash	BLOB NOT NULL UNIQUE,"
                    "Nonce          INTEGER NOT NULL,"
                    "MerkleRoot     BLOB NOT NULL,"
                    "time           TEXT NOT NULL UNIQUE,"
                    "PRIMARY KEY(\"Block_ID\" AUTOINCREMENT)"
                    ")"))
        {
            qDebug()<<"Error where create a table, table Blocks was not created";
            qDebug()<<query.lastError().text();
        }

        if(!query.exec(
                    "CREATE TABLE IF NOT EXISTS Transactions ("
                    "Block_id           INTEGER NOT NULL,"
                    "Transaction_Hash	BLOB NOT NULL UNIQUE,"
                    "PublicKey          BLOB NOT NULL,"
                    "Signature          BLOB NOT NULL,"
                    "Summ               REAL NOT NULL,"
                    "FOREIGN KEY (Block_id) REFERENCES Blocks(Block_ID)"
                    ")"
                    ))

        {
            qDebug()<<"Error where create a table, table Transactions was not created";
            qDebug()<<query.lastError().text();
        }

        if(!query.exec(
                    "CREATE TABLE IF NOT EXISTS Senders ("
                    "Transaction_Hash    BLOB NOT NULL,"
                    "sender_address      BLOB NOT NULL,"
                    "tokens_count        REAL NOT NULL,"
                    "FOREIGN KEY (Transaction_Hash) REFERENCES Transactions(Transaction_Hash)"
                    ")"
                    ))

        {
            qDebug()<<"Error where create a table, table Senders was not created";
            qDebug()<<query.lastError().text();
        }

        if(!query.exec(
                    "CREATE TABLE IF NOT EXISTS Receivers ("
                    "Transaction_Hash    BLOB NOT NULL,"
                    "Receiver_address    BLOB NOT NULL,"
                    "tokens_count        REAL NOT NULL,"
                    "FOREIGN KEY (Transaction_Hash) REFERENCES Transactions(Transaction_Hash)"
                    ")"
                    ))

        {
            qDebug()<<"Error where create a table, table Receivers was not created";
            qDebug()<<query.lastError().text();
        }

        db.close();

        QThread *thread = new QThread();
        clientsThreads.append(thread);
        thread->start();

        Client *client = new Client();
        client->moveToThread(thread);

        connect(client,&Client::nodeCreated,this,&MainCore::wallet);
        connect(this,&MainCore::start,client,&Client::startBlock);
        connect(client,&Client::transaction,this,&MainCore::transaction4check);
        connect(this,&MainCore::transaction4check,client,&Client::checkTransaction);
        connect(client,&Client::blockCreated,this,&MainCore::stopCreate);
        connect(this,&MainCore::stopCreate,client,&Client::stopCreated);
        connect(client,&Client::block4check,this,&MainCore::block4check);
        connect(this,&MainCore::block4check,client,&Client::checkBlock);

        connect(client,&Client::changeBalance,this,&MainCore::changeBalance);
        connect(this,&MainCore::sendTokens,client,&Client::makeTransaction);
        connect(this,&MainCore::stop,client,&Client::stopMake);
        connect(this,&MainCore::makeblock,client,&Client::makeOneBlock);
        connect(this,&MainCore::setHashLevel,client,&Client::setHashLevel);
        connect(client,&Client::logMessage,this,&MainCore::logMessage);

        client->init(privat_key,public_key, BCname, num);

        clients.append(client);

    }

}


