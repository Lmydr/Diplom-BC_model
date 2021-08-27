#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowTitle("Blockchein");


    ui->setupUi(this);
    QVBoxLayout *lay = new QVBoxLayout;
    tab = new QTabWidget;
    tab->setLayout(lay);
    ui->mainLay->addWidget(tab);

    ui->AddButton->setVisible(false);
    ui->widget_confNoFound->setVisible(false);
    ui->widget_confFounded->setVisible(false);
    ui->startBtn->setVisible(false);

    tab->setVisible(false);

    ui->stop_pushButton->setVisible(false);

    ui->label->setVisible(false);

    ui->stop_pushButton->setEnabled(false);


    ui->spinBox->setVisible(false);
    ui->pushButton->setVisible(false);//accept button


    QFile f("BC.conf");
    if(!f.open(QIODevice::ReadOnly)){
        ui->widget_confNoFound->setVisible(true);
    }else{
        ui->widget_confFounded->setVisible(true);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillForm(int num, WalletStr str)
{
    if(walletsForms.at(num)){
        WalletForm *tmp = walletsForms.at(num);
        tmp->setValues(num,str);
    }
}

void MainWindow::changeBalance(double balance, int num)
{
    if(walletsForms.at(num)){
        WalletForm *tmp = walletsForms.at(num);
        tmp->setBalance(balance);
    }
}

void MainWindow::setLogMessage(int num, QByteArray log)
{
    if(walletsForms.at(num)){
        WalletForm *tmp = walletsForms.at(num);
        tmp->setLog(log);
    }
}


void MainWindow::on_AddButton_clicked()
{
    ui->startBtn->setEnabled(true);
    WalletForm *wallet = new WalletForm();
    connect(wallet,&WalletForm::sendTokens,this,&MainWindow::sendTokens);
    connect(wallet,&WalletForm::makeBlock,this,&MainWindow::makeblock);
    connect(this,&MainWindow::setEnMakeBlock,wallet,&WalletForm::setMakeBlockEn);
    walletsForms.append(wallet);
    tab->addTab(wallet,QString::number(walletsForms.size()-1));

    emit generateWallet(walletsForms.size()-1);
}


void MainWindow::on_pushButton_noFoundOK_clicked()
{
    ui->AddButton->setVisible(true);
    ui->startBtn->setVisible(true);
    ui->startBtn->setEnabled(false);
    tab->setVisible(true);

    ui->stop_pushButton->setVisible(true);

    ui->label->setVisible(true);

    ui->spinBox->setVisible(true);
    ui->pushButton->setVisible(true);//accept button

    ui->widget_confNoFound->setVisible(false);
    ui->widget_confFounded->setVisible(false);
    emit newBC();
}

void MainWindow::on_pushButton_createNewBC_clicked()
{
    ui->AddButton->setVisible(true);
    ui->startBtn->setVisible(true);
    ui->startBtn->setEnabled(false);
    tab->setVisible(true);
    ui->widget_confNoFound->setVisible(false);
    ui->widget_confFounded->setVisible(false);
    emit newBC();
}

void MainWindow::on_pushButton_LoadBC_clicked()
{
    emit loadBC();
}

void MainWindow::on_startBtn_clicked()
{
    ui->startBtn->setEnabled(false);
    ui->stop_pushButton->setEnabled(true);
    //ui->horizontalSlider->setEnabled(false);
    emit setEnMakeBlock(false);
    emit start();
}

void MainWindow::on_stop_pushButton_clicked()
{
    ui->stop_pushButton->setEnabled(false);
    ui->startBtn->setEnabled(true);
    //ui->horizontalSlider->setEnabled(true);
    emit setEnMakeBlock(true);
    emit stop();
}

void MainWindow::on_pushButton_clicked()
{
    emit setHashLevel(ui->spinBox->value());
}
