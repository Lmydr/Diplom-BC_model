#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>

#include <QSqlDatabase>
#include <QSqlQuery>

#include "Structs.h"

#include <openssl/ecdsa.h>
#include <openssl/crypto.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#include "hashblockcreator.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    int init(QByteArray sk, QByteArray pk, QByteArray folderName, int numNode);

    QByteArray makeSign(Transaction);

    bool checkTrInBlock(QList<SignTransaction>);
    bool checkTr(SignTransaction);
    QByteArray makeMerkleRoot(QList<SignTransaction>);

    QByteArray makeTransactionHash(Transaction);
    QByteArray makeTransactionHash(SignTransaction);

    QByteArray getHashLastBlock();
    qint64 getBlockID(QByteArray hash);

    double getSenderSum(QByteArray addr);
    double getReceiverSum(QByteArray addr);
    double getBalance(QByteArray addr);

    void insertTransactions(QList<SignTransaction>,QByteArray);

    void makeBlock();

    //void makeTransaction(double count, QByteArray addrReceiver);

    QList<SignTransaction> makePackTransactions();

signals:
    void transaction(SignTransaction);
    void block4check(Block);
    void nodeCreated(int,WalletStr);

    void startCreateHashBlock();
    void stopCreateHashBlock();
    void blockCreated();

    void changeBalance(double, int nodeNumber);

    void logMessage(int num, QByteArray message);




public slots:
    void checkBlock(Block);
    void checkTransaction(SignTransaction);
    void startBlock();
    void stopCreated();

    void stopMake();

    void makeOneBlock(int num);

    void makeTransaction(int num, double count, QByteArray addrReceiver);

    void setHashLevel(int num);

private slots:
    void blockHashCreated(QByteArray hash);

private:
    bool stopMakeBlocks = true;
    int hashLevel = 6;

    int nodeNumber;

    double balance = 0;

    QByteArray secretKey;
    QByteArray publicKey;
    QByteArray address;

    QList<SignTransaction> TrasactionBuffer;

    QThread *hashThread;
    HashBlockCreator* creator;

    Block block4send;

    QSqlDatabase db;
    QByteArray folder;

    EC_GROUP *group;
    EC_KEY *key;
};

#endif // CLIENT_H
