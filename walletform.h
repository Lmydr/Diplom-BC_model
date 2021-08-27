#ifndef WALLETFORM_H
#define WALLETFORM_H

#include <QWidget>
#include "Structs.h"
#include <sendtokensform.h>

namespace Ui {
class WalletForm;
}

class WalletForm : public QWidget
{
    Q_OBJECT

public:
    explicit WalletForm(QWidget *parent = nullptr);
    ~WalletForm();

    void init(QByteArrayList data);
    void setValues(int num, WalletStr str);
    void setBalance(qint64);
    void setLog(QByteArray log);


public slots:
    void sendT(double count, QByteArray addr);
    void setMakeBlockEn(bool);

signals:
    void sendTokens(int num, double count, QByteArray addr);
    void makeBlock(int num);

private slots:
    void on_pushButton_clicked();

    void on_makeBlock_pushButton_2_clicked();

private:
    int myNum;
    Ui::WalletForm *ui;
    SendTokensForm *form;
};

#endif // WALLETFORM_H
