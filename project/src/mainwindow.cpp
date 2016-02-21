#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
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
    int i = 0;
    ui->tabWidget->clear();
    clearSerialPorts();
#if 0
    for(int i=0;i<3; i++){
        QWidget* newtab = createTabPage(i);
        ui->tabWidget->addTab(newtab,QString("hallo"));
        QSerialPort* serialport = new QSerialPort();
        serialPortList.append(serialport);
    }
#else
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QWidget* newtab = createTabPage(i);
        ui->tabWidget->addTab(newtab,info.portName());
        QSerialPort* serialport = new QSerialPort();
        serialPortList.append(serialport);
        i++;
    }
#endif

}

QWidget *MainWindow::createTabPage(int tabIndex)
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

    btnConnect->setProperty("tabIndex",tabIndex);
    btnConnect->setProperty("connected",false);

    comBaud->setObjectName("comBaud");
    comDataBits->setObjectName("comDataBits");
    comStopBits->setObjectName("comStopBits");

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
    stopl << "1" << "1.5" << "2";
    comStopBits->addItems(stopl);
    comStopBits->setCurrentIndex(0);

    hbox->addWidget(btnConnect);
    hbox->addWidget(colParent);

    hbox->addStretch();

    connect(btnConnect,SIGNAL(clicked()),this,SLOT(on_btnConnectClicked()));
    return result;
}

void MainWindow::clearSerialPorts(){
    foreach (QSerialPort* P, serialPortList) {
        delete P;
    }
}

QSerialPort::StopBits MainWindow::stopBitDescriptionToStopBit(QString desc){
    if(desc == "1")
        return QSerialPort::OneStop;
    if(desc == "1.5")
        return QSerialPort::OneAndHalfStop;
    if(desc == "2")
        return QSerialPort::TwoStop;
    return QSerialPort::UnknownStopBits;
}

void MainWindow::on_btnConnectClicked(){

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    //qDebug() << "test";
    bool shallBeOpened = true;
    if( button != NULL )
    {
       if (button->property("connected").toBool() == true){
           button->setText("Connect");
           button->setProperty("connected",false);
           shallBeOpened = false;
       }else{
           button->setText("Disconnect");
           button->setProperty("connected",true);
           shallBeOpened = true;
       }
       int tabIndex = button->property("tabIndex").toInt();
       QSerialPort* serialport = serialPortList[tabIndex];

       if (shallBeOpened){
           serialport->setPortName(ui->tabWidget->tabText(tabIndex));

           QComboBox* comBaud = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comBaud");
           int baudrate = comBaud->currentText().toInt();
           serialport->setBaudRate(baudrate);

           QComboBox* comData = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comDataBits");
           int dataBits = comData->currentText().toInt();
           serialport->setDataBits((QSerialPort::DataBits)dataBits);

           QComboBox* comStop = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comStopBits");
           QSerialPort::StopBits stopBits = stopBitDescriptionToStopBit(comStop->currentText());
           serialport->setStopBits(stopBits);

           qDebug() << "open" << serialport->portName() << baudrate << dataBits << stopBits;
           serialport->open(QIODevice::ReadOnly);
           if (serialport->isOpen()){
               qDebug() << "opened";
           }else{
               qDebug() << "still closed";
           }
       }else{
           qDebug() << "close" << serialport->portName();
           serialport->close();
           if (serialport->isOpen()){
               qDebug() << "still opened";
           }else{
               qDebug() << "closed";
           }
       }
    }
}

