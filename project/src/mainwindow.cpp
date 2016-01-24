#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>

#include <QFormLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    refreshTabs();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTabs(void){
    ui->tabWidget->clear();
#if 0
    for(int i=0;i<3; i++){
        QWidget* newtab = createTabPage();
        ui->tabWidget->addTab(newtab,QString("hallo"));
    }
#else
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->tabWidget->addTab(ui->tab,info.portName());


        // Example use QSerialPort

    }
#endif
}

QWidget *MainWindow::createTabPage()
{
    QWidget* result = new QWidget(ui->tabWidget);
    QWidget* colParent = new QWidget(result);
    QHBoxLayout *hbox = new QHBoxLayout(result);
    QFormLayout *flayout = new QFormLayout(colParent);
    QPushButton* btnConnect = new QPushButton(result);

    QComboBox* comBaud = new QComboBox(colParent);
    QComboBox* comDataBits = new QComboBox(colParent);
    QComboBox* comStopBits = new QComboBox(colParent);

    flayout->addRow("Baud",comBaud);
    flayout->addRow("Data bits",comDataBits);
    flayout->addRow("Stop bits",comStopBits);

    btnConnect->setText("Connect");
    btnConnect->setAccessibleName("btnConnect");

    comBaud->setAccessibleName("comBaud");
    comDataBits->setAccessibleName("comDataBits");
    comStopBits->setAccessibleName("comStopBits");

    QStringList bdl;
    bdl << "75" << "300" << "1200" << "2400" << "4800" << "9600" << "14400" << "19200" << "28800" << "38400" << "57600" << "115200";
    comBaud->addItems(bdl);
    comBaud->setEditable(true);
    comBaud->setCurrentIndex(comBaud->count()-1);

    QStringList dtal;
    dtal << "5" << "6" << "7" << "8";
    comDataBits->addItems(dtal);
    comDataBits->setCurrentIndex(comDataBits->count()-1);

    QStringList stopl;
    stopl << "1" << "2";
    comStopBits->addItems(stopl);
    comStopBits->setCurrentIndex(0);

    hbox->addWidget(btnConnect);
    hbox->addWidget(colParent);

    hbox->addStretch();

    return result;
}
