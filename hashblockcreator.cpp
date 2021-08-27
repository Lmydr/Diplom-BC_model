#include "hashblockcreator.h"

#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include <QBigEndianStorageType>
#include <QLittleEndianStorageType>

QDataStream & operator << (QDataStream & in, headBlockForHash & s){
    in<<s.prevBlockHash;
    in<<s.merkleRoot;
    in<<s.timeStamp;
    in<<s.nonce;
    return in;
}

HashBlockCreator::HashBlockCreator(QObject *parent) : QObject(parent)
{

}

void HashBlockCreator::init(headBlockForHash head)
{
    str = head;
}

qlonglong HashBlockCreator::getLastNonce()
{
    return str.nonce;
}

headBlockForHash HashBlockCreator::getHeaderBlock()
{
    return str;
}

QByteArray HashBlockCreator::MakeHash(headBlockForHash str)
{
    QByteArray headArr;
    QDataStream s(&headArr,QIODevice::WriteOnly);
    s<<str;
    return QCryptographicHash::hash(headArr,QCryptographicHash::Algorithm::Sha256);

}

void HashBlockCreator::setLevel(int num)
{
    hashLevel = num;
}

void HashBlockCreator::start()
{
    stopCreate = false;
    //QByteArray forCheck;

    do{
        QByteArray headArr;
        QDataStream s(&headArr,QIODevice::WriteOnly);
        str.nonce++;
        str.timeStamp = QByteArray::number(QDateTime::currentSecsSinceEpoch());
        s<<str;
        hash = QCryptographicHash::hash(headArr,QCryptographicHash::Algorithm::Sha256);
        //forCheck = hash.toHex();



        QByteArray checkBytes = hash.toHex();
        checkBytes.remove(8,checkBytes.length());


        levelFlag = true;

        for(int i = 0; i < hashLevel;i++){
            if(checkBytes.at(i)!= '0'){
                levelFlag = false;
            }
        }

        //QByteArray  test = 0x0fffffff;

//        for(int i = 0; i < hashLevel;i++){
//            if(hash.at(i)!=0x00){
//                levelFlag = false;
//            }
//        }

    }while((!levelFlag) && (!stopCreate) );
//    }while((hash.at(0)!= 0x00 || hash.at(1)!= 0x00 || hash.at(2)!= 0x00 ) && (!stopCreate) );

    //}while((forCheck[0]!= '0' || forCheck[1] != '0' || forCheck[2] != '0'|| forCheck[3] != '0' || forCheck[4] != '0'|| forCheck[5] != '0') && (!stopCreate) );

    if(!stopCreate){
        emit createdCorrectHash(hash);
    }

}

void HashBlockCreator::stop()
{
    stopCreate = true;
    qDebug()<<"stoped";
}
