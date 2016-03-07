#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QDateTime>
#include <QFormLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    columnCount = 1;
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
        SerialNode* serialNode = new SerialNode(this);
        serialPortList.append(serialNode);
        i++;
    }
#endif

}

int MainWindow::insertColumn(QString name){
    if (portToColMap.contains(name)){
        return portToColMap.value(name);

    }else{

        QTreeWidgetItem* header = ui->treeWidget->headerItem();
        portToColMap.insert(name,columnCount);
        header->setText(columnCount,name);
        columnCount++;
        ui->treeWidget->setColumnCount(columnCount);
        ui->treeWidget->resizeColumnToContents(0);
        for (int i= 0;i< ui->treeWidget->columnCount();i++){
            ui->treeWidget->setColumnWidth(i,150);
        }
        return columnCount-1;
    }
}

QWidget *MainWindow::createTabPage(int tabIndex)
{
    QWidget* result = new QWidget(ui->tabWidget);
    QWidget* colCOM = new QWidget(result);
    QWidget* colFormat = new QWidget(result);
    QHBoxLayout *hbox = new QHBoxLayout(result);

    QFormLayout *flayoutCOM = new QFormLayout(colCOM);
    QFormLayout *flayoutFormat = new QFormLayout(colFormat);
    QPushButton* btnConnect = new QPushButton(result);

    QHBoxLayout *hboxLength = new QHBoxLayout(colFormat);
    QRadioButton*  radByLength = new QRadioButton(colFormat);
    QSpinBox* spinNLbyLenth = new QSpinBox(colFormat);

    QHBoxLayout *hboxEscape = new QHBoxLayout(colFormat);
    QRadioButton *radByEscape = new QRadioButton(colFormat);
    QLineEdit* edtEscape = new QLineEdit(colFormat);

    QHBoxLayout *hboxRegEx = new QHBoxLayout(colFormat);
    QRadioButton*  radByRegEx = new QRadioButton(colFormat);
    QLineEdit* edtRegEx = new QLineEdit(colFormat);

    QComboBox* comBaud = new QComboBox(colCOM);
    QComboBox* comDataBits = new QComboBox(colCOM);
    QComboBox* comStopBits = new QComboBox(colCOM);

    QComboBox* comFormat = new QComboBox(colFormat);

    flayoutCOM->addRow("Baud",comBaud);
    flayoutCOM->addRow("Data bits",comDataBits);
    flayoutCOM->addRow("Stop bits",comStopBits);

    hboxLength->addWidget(radByLength);
    hboxLength->addWidget(spinNLbyLenth);

    hboxEscape->addWidget(radByEscape);
    hboxEscape->addWidget(edtEscape);

    hboxRegEx->addWidget(radByRegEx);
    hboxRegEx->addWidget(edtRegEx);

    flayoutFormat->addRow("View",comFormat);
    flayoutFormat->addRow("Newline by length",hboxLength);
    flayoutFormat->addRow("Newline by Escape",hboxEscape);

    flayoutFormat->addRow("Newline by Regex",hboxRegEx);

    btnConnect->setText("Connect");
    btnConnect->setObjectName("btnConnect");

    btnConnect->setProperty("tabIndex",tabIndex);
    btnConnect->setProperty("connected",false);

    comBaud->setObjectName("comBaud");
    comDataBits->setObjectName("comDataBits");
    comStopBits->setObjectName("comStopBits");

    comFormat->setObjectName("comFormat");

    spinNLbyLenth->setValue(8);
    spinNLbyLenth->setObjectName("spinNLbyLenth");

    radByLength->setChecked(true);
    radByLength->setObjectName("radByLength");

    edtEscape->setText("\\r\\n");
    edtEscape->setObjectName("edtEscape");
    radByEscape->setObjectName("radByEscape");

    edtRegEx->setObjectName("edtRegEx");
    radByRegEx->setObjectName("radByRegEx");

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

    QStringList comFormatl;
    comFormatl << "HEX" << "ASCII";
    comFormat->addItems(comFormatl);
    comFormat->setCurrentIndex(0);

    hbox->addWidget(btnConnect);
    hbox->addWidget(colCOM);
    hbox->addWidget(colFormat);

    hbox->addStretch();

    connect(btnConnect,SIGNAL(clicked()),this,SLOT(on_btnConnectClicked()));
    return result;
}

void MainWindow::clearSerialPorts(){
    foreach (SerialNode* P, serialPortList) {
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
       QSerialPort* serialport = serialPortList[tabIndex]->serialport;

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

           QComboBox* comFormat = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comFormat");
           bool usehex = comFormat->currentIndex()==0;
           int colIndex = insertColumn(serialport->portName());
           serialPortList[tabIndex]->setAppearance(usehex);
           serialPortList[tabIndex]->setColIndex(colIndex);




           QRadioButton* radByLength = ui->tabWidget->widget(tabIndex)->findChild<QRadioButton*>("radByLength");
           QRadioButton* radByEscape = ui->tabWidget->widget(tabIndex)->findChild<QRadioButton*>("radByEscape");
           QRadioButton* radByRegEx = ui->tabWidget->widget(tabIndex)->findChild<QRadioButton*>("radByRegEx");

           if (radByLength->isChecked()){
                QSpinBox* spinNLbyLenth = ui->tabWidget->widget(tabIndex)->findChild<QSpinBox*>("spinNLbyLenth");
                serialPortList[tabIndex]->setEscaping(nodeEscaping_t::byLength);
                serialPortList[tabIndex]->setEscapeLength(spinNLbyLenth->value());
           }
           if (radByEscape->isChecked()){
                QLineEdit* edtEscape = ui->tabWidget->widget(tabIndex)->findChild<QLineEdit*>("edtEscape");
                serialPortList[tabIndex]->setEscaping(nodeEscaping_t::byEscapeCharacter);
                serialPortList[tabIndex]->setEscapeChar(edtEscape->text());
           }
           if (radByRegEx->isChecked()){
               QLineEdit* edtRegEx = ui->tabWidget->widget(tabIndex)->findChild<QLineEdit*>("edtRegEx");
               serialPortList[tabIndex]->setEscaping(nodeEscaping_t::byRegEx);
               serialPortList[tabIndex]->setEscapeRegEx(edtRegEx->text());

           }
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



void MainWindow::addNewEntry(QString time, QString content, int colIndex)
{
    QTreeWidgetItem *entry = new QTreeWidgetItem(ui->treeWidget);
    entry->setText(0,time);
    entry->setText(colIndex,content);
    // qDebug() << line;
    ui->treeWidget->addTopLevelItem(entry);
    ui->treeWidget->scrollToBottom();
}


void MainWindow::on_readyRead()
{

}

