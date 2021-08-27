#include "walletform.h"
#include "ui_walletform.h"
#include "QTime"

WalletForm::WalletForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WalletForm)
{
    ui->setupUi(this);
    ui->addressValue_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->balanceValue_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->publicKeyValue_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->secretKeyValue_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    form = new SendTokensForm();
    connect(form,&SendTokensForm::sendTokens,this,&WalletForm::sendT);


}

WalletForm::~WalletForm()
{
    delete ui;
}

void WalletForm::setValues(int num, WalletStr str)
{
    myNum = num;
    ui->secretKeyValue_label->setText(str.s_key);
    ui->publicKeyValue_label->setText(str.p_key);
    ui->addressValue_label->setText(str.addr);
    ui->balanceValue_label->setText(QString::number(str.balance));

    form->setBalanceAndAddr(str.balance,str.addr);
}

void WalletForm::setBalance(qint64 balance)
{
    ui->balanceValue_label->setText(QString::number(balance));
    form->setBalance(balance);
}

void WalletForm::setLog(QByteArray log)
{
    QByteArray arr = QTime::currentTime().toString().toUtf8() + "\n";
    ui->textBrowser->append(arr+log);
}

void WalletForm::sendT(double count, QByteArray addr)
{
    emit sendTokens(myNum,count,addr);
}

void WalletForm::setMakeBlockEn(bool f)
{
    ui->makeBlock_pushButton_2->setEnabled(f);
}

void WalletForm::on_pushButton_clicked()
{
    form->show();
}

void WalletForm::on_makeBlock_pushButton_2_clicked()
{
    emit makeBlock(myNum);
}
