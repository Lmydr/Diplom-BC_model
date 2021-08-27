#include "mainwindow.h"
#include <QApplication>
#include <QSslEllipticCurve>
#include <QSslKey>
#include <QDebug>
#include <openssl/ecdsa.h>
#include <openssl/crypto.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#include "wallet.h"
#include "maincore.h"

#include <QCryptographicHash>
#include <QObject>

QByteArray makeSign(unsigned char* hash, int hash_len, EC_KEY* key);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    MainCore *core = new MainCore();
    QObject::connect(&w,&MainWindow::newBC,core,&MainCore::createNewBC);
    QObject::connect(&w,&MainWindow::generateWallet,core,&MainCore::addNode);
    QObject::connect(core,&MainCore::wallet,&w,&MainWindow::fillForm);
    QObject::connect(&w,&MainWindow::start,core,&MainCore::start);
    QObject::connect(core,&MainCore::changeBalance,&w,&MainWindow::changeBalance);
    QObject::connect(&w,&MainWindow::sendTokens,core,&MainCore::sendTokens);
    QObject::connect(&w,&MainWindow::stop,core,&MainCore::stop);
    QObject::connect(&w,&MainWindow::makeblock,core,&MainCore::makeblock);
    QObject::connect(&w,&MainWindow::setHashLevel,core,&MainCore::setHashLevel);
    QObject::connect(core,&MainCore::logMessage,&w,&MainWindow::setLogMessage);






    EC_KEY *testKey = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_generate_key(testKey);

    QByteArray privat_key = BN_bn2hex(EC_KEY_get0_private_key(testKey));
    QByteArray public_key = EC_POINT_point2hex(EC_KEY_get0_group(testKey),EC_KEY_get0_public_key(testKey),POINT_CONVERSION_COMPRESSED,NULL);

    const char* pk = public_key.data();

    qDebug()<<"Privat key: "<<privat_key;
    qDebug()<<"Public key: "<<public_key;
    QByteArray hash = QCryptographicHash::hash("help someone please!",QCryptographicHash::Algorithm::Sha256);
    QByteArray s = makeSign((unsigned char*)hash.data(),hash.length(),testKey);
    const unsigned char* sig = (unsigned char*)s.data();

    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_KEY *p = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_POINT *p_key_point = EC_POINT_new(group);
    EC_POINT_hex2point(group,pk,p_key_point,NULL);
    EC_KEY_set_public_key(p,p_key_point);

    //EC_KEY_oct2key(p,pk,public_key.length(),NULL);
    //o2i_ECPublicKey(&p,&pk,(long)public_key.length());
    if(ECDSA_verify(0,(unsigned char*)hash.data(),hash.length(),sig,s.length(),p)  <= 0){
        qDebug()<<"Error";
    }else{
        qDebug()<<"KEK";
    }
    char *err_buff2 = new char[1024];
    ERR_error_string(ERR_get_error(),err_buff2);
    qDebug()<<err_buff2;

    return a.exec();
}

QByteArray makeSign(unsigned char* hash, int hash_len, EC_KEY* key){
    QByteArray result;
    int maxsize_sign = ECDSA_size(key);
    //unsigned char* sign = new unsigned char[maxsize_sign];
    unsigned char* sign = new unsigned char[maxsize_sign];
    unsigned int sign_size;
    if(ECDSA_sign(0,hash,hash_len,sign,&sign_size,key)==1){
        qDebug()<<"all right";
    }
    result = result.fromRawData((char*)sign,sign_size);
    qDebug()<<"Signature: "<< result.toHex();

    if(ECDSA_verify(0,hash,hash_len,(unsigned char*)result.data(),result.length(),key) == 1){
        qDebug()<<"true";
    }else{
        qDebug()<<"false";
    }

    return result;
}
