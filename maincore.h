#ifndef MAINCORE_H
#define MAINCORE_H

#include <QObject>
#include <QSqlDatabase>
#include <QThread>

#include "client.h"
#include "Structs.h"

class MainCore : public QObject
{
    Q_OBJECT
public:
    explicit MainCore(QObject *parent = nullptr);
    ~MainCore();

public slots:
    void createNewBC();
    void addNode(int num);

signals:
    void wallet(int,WalletStr);
    void start();
    void stop();
    void stopCreate();
    void transaction4check(SignTransaction);
    void block4check(Block);
    void changeBalance(double, int nodeNumber);

    void sendTokens(int num, double count, QByteArray addr);

    void makeblock(int num);

    void setHashLevel(int num);
    void logMessage(int num, QByteArray message);

private:
    QByteArray BCname;
    QSqlDatabase db;

    QList<Client*> clients;
    QList<QThread*> clientsThreads;

};

#endif // MAINCORE_H
