#ifndef SENDTOKENSFORM_H
#define SENDTOKENSFORM_H

#include <QWidget>

namespace Ui {
class SendTokensForm;
}

class SendTokensForm : public QWidget
{
    Q_OBJECT

public:
    explicit SendTokensForm(QWidget *parent = nullptr);
    ~SendTokensForm();

    void setBalanceAndAddr(qint64, QByteArray);
    void setBalance(qint64);

signals:
    void sendTokens(double count, QByteArray addr);

private slots:
    void on_send_pushButton_clicked();

    void on_cancel_pushButton_2_clicked();

private:
    Ui::SendTokensForm *ui;
};

#endif // SENDTOKENSFORM_H
