#include "sendtokensform.h"
#include "ui_sendtokensform.h"

SendTokensForm::SendTokensForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendTokensForm)
{
    ui->setupUi(this);
}

SendTokensForm::~SendTokensForm()
{
    delete ui;
}

void SendTokensForm::setBalanceAndAddr(qint64 balance, QByteArray addr)
{
    ui->addr_label->setText(addr);
    ui->balance_label->setText(QString::number(balance));
}

void SendTokensForm::setBalance(qint64 balance)
{
     ui->balance_label->setText(QString::number(balance));
}

void SendTokensForm::on_send_pushButton_clicked()
{
    emit sendTokens(ui->count_lineEdit_2->text().toDouble(),ui->taAddr_lineEdit->text().toUtf8());
    close();
}

void SendTokensForm::on_cancel_pushButton_2_clicked()
{
    close();
}
