#ifndef HASHBLOCKCREATOR_H
#define HASHBLOCKCREATOR_H

#include <QObject>
#include <Structs.h>
#include <QDataStream>


class HashBlockCreator : public QObject
{
    Q_OBJECT
public:
    explicit HashBlockCreator(QObject *parent = nullptr);
    void init(headBlockForHash);

    qlonglong getLastNonce();
    headBlockForHash getHeaderBlock();

    QByteArray MakeHash(headBlockForHash str);

    void setLevel(int num);

public slots:
    void start();
    void stop();




signals:
    void createdCorrectHash(QByteArray);
    void stopped();

private:
    headBlockForHash str;
    bool stopCreate = false;

    int hashLevel = 6;
    bool levelFlag = true;


    QByteArray hash;

};

#endif // HASHBLOCKCREATOR_H
