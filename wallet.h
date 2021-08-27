#ifndef WALLET_H
#define WALLET_H

#include <QObject>
#include <QCryptographicHash>

#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/crypto.h>
#include <openssl/ec.h>


class Wallet : public QObject
{
    Q_OBJECT
public:
    explicit Wallet(QObject *parent = nullptr);

    // init function return 1 if all right 0 else
    int init();

    //
    QByteArray makeSign(QByteArray message);

    //
    int verefySign(QByteArray sign, QByteArray hash);

    //
    int verefyBlock();
signals:

private:
    QByteArray secret_key;
    QByteArray public_key;
    QByteArray address;

    double ballance;

    EC_GROUP *group;
    EC_KEY *key;


};

#endif // WALLET_H
