#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "walletform.h"
#include "Structs.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void fillForm(int num, WalletStr);

    void changeBalance(double balance, int num);

    void setLogMessage(int num,QByteArray log);

signals:
    void generateWallet(int num);
    void newBC();
    void loadBC();
    void start();
    void stop();

    void makeblock(int num);

    void sendTokens(int num, double count, QByteArray addr);

    void setHashLevel(int);

    void setEnMakeBlock(bool);


private slots:
    void on_AddButton_clicked();

    void on_pushButton_noFoundOK_clicked();

    void on_pushButton_createNewBC_clicked();

    void on_pushButton_LoadBC_clicked();

    void on_startBtn_clicked();

    void on_stop_pushButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QTabWidget *tab;
    QList<WalletForm*> walletsForms;

};
#endif // MAINWINDOW_H
