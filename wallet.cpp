#include "wallet.h"
#include <QDebug>

Wallet::Wallet(QObject *parent) : QObject(parent)
{


}

int Wallet::init()
{
    key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if(EC_KEY_generate_key(key) < 1){
        return 0;
    }
    secret_key = BN_bn2hex(EC_KEY_get0_private_key(key));
    public_key = EC_POINT_point2hex(EC_KEY_get0_group(key),EC_KEY_get0_public_key(key),POINT_CONVERSION_COMPRESSED,NULL);

    address = QCryptographicHash::hash(public_key,QCryptographicHash::Algorithm::Sha256);

    return 1;
}

QByteArray Wallet::makeSign(QByteArray message)
{
    /*
    QByteArray result;
    unsigned char* c_message = (unsigned char*)message.data();

    int maxsize_sign = ECDSA_size(key);
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
    */

    return 0;
}
