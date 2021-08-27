#ifndef STRUCTS_H
#define STRUCTS_H

#include <QObject>
#include <QByteArray>
#include <QList>

struct WalletStr{
    QByteArray s_key;
    QByteArray p_key;
    QByteArray addr;
    double balance;
};

struct Sender{
    QByteArray addr;
    double count;
};
struct Receiver{
    QByteArray addr;
    double count;
};

struct Transaction{
    Sender sender;
    QList<Receiver> reseivers;

};

struct SignTransaction{
    QByteArray p_key;
    QByteArray sign;
    Sender sender;
    QList<Receiver> reseivers;
}; Q_DECLARE_METATYPE(SignTransaction);

struct headBlockForHash{
    QByteArray prevBlockHash;
    QByteArray merkleRoot;
    QByteArray timeStamp;
    qlonglong nonce;
};

struct Block{
    QByteArray prevBlockHash;
    QByteArray currentBlockHash;
    qlonglong nonce;
    QByteArray merkleRoot;
    QByteArray timeStamp;
    QList<SignTransaction> transactions;
};Q_DECLARE_METATYPE(Block);



#endif // STRUCTS_H
