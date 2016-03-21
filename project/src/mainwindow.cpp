#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QDateTime>
#include <QFormLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QSettings>
#include <QDir>
#include <QMessageBox>

const int MAXFILEROWS = 300*1000;
const QString SETTINGS_FILE_NAME = QDir::currentPath()+QDir::separator()+"protanalyzer.ini";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    QSettings settings(SETTINGS_FILE_NAME, QSettings::IniFormat );

    QString basePath = settings.value("ExportFilePath",QDir::currentPath()+QDir::separator()+"export").toString();
    settings.setValue("ExportFilePath",QDir::currentPath()+QDir::separator()+"export");

    QString dateStr =  QDateTime::currentDateTime().toString("MM_dd__HH_mm_ss_");

    columnCount = 1;
    refreshTabs();
    fileIndex=0;
    fileRows=0;
    if (!basePath.endsWith(QDir::separator())){
        basePath+= QDir::separator();
    }
    fileName = basePath+dateStr;
    beginNewDumpFile();

}

MainWindow::~MainWindow()
{



    for(int i=0;i<ui->tabWidget->count();i++){
        QWidget* widget = ui->tabWidget->widget(i);

        QRadioButton*  radByLength = widget->findChild<QRadioButton*>("radByLength");
        QSpinBox* spinNLbyLenth = widget->findChild<QSpinBox*>("spinNLbyLenth");
        QRadioButton *radByEscape = widget->findChild<QRadioButton*>("radByEscape");
        QLineEdit* edtEscape = widget->findChild<QLineEdit*>("edtEscape");
        QRadioButton*  radByRegEx = widget->findChild<QRadioButton*>("radByRegEx");
        QLineEdit* edtRegEx = widget->findChild<QLineEdit*>("edtRegEx");

        QComboBox* comBaud = widget->findChild<QComboBox*>("comBaud");
        QComboBox* comDataBits = widget->findChild<QComboBox*>("comDataBits");
        QComboBox* comStopBits = widget->findChild<QComboBox*>("comStopBits");

        QComboBox* comFormat = widget->findChild<QComboBox*>("comFormat");

        QSettings settings(SETTINGS_FILE_NAME, QSettings::IniFormat );
        settings.beginGroup("COMName"+ui->tabWidget->tabText(i));

        settings.setValue("Display Format",comFormat->currentIndex());
        settings.setValue("COM Stop",comStopBits->currentIndex());
        settings.setValue("COM DataLength",comDataBits->currentIndex());
        settings.setValue("COM Baud",comBaud->currentIndex());
        settings.setValue("Display Escape Length",spinNLbyLenth->value());
        settings.setValue("Display Escape String",edtEscape->text());
        settings.setValue("Display Escape Regex",edtRegEx->text());
        int val=0;
        if (radByLength->isChecked()){
            val = 0;
        }else if (radByEscape->isChecked()){
            val = 1;
        }else if (radByRegEx->isChecked()){
            val = 2;
        }
        settings.setValue("Display Escape type",val);

        settings.endGroup();
        }
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
        QWidget* newtab = createTabPage(i,info.portName());
        ui->tabWidget->addTab(newtab,info.portName());
        SerialNode* serialNode = new SerialNode(this);
        serialPortList.append(serialNode);
        //serialNode->setRPCDescriptionFileName("C:/Users/ark/entwicklung/eclipse_workspace/funksonde2_probe/modules/rpcBluetooth/doc/server/RPC_BLUETOOTH_cmd_sg2probe.xml");
        serialNode->setRPCDescriptionFileName("C:/Users/ark/entwicklung/eclipse_workspace/funksonde2_probe/modules/rpcBluetooth/doc/client/RPC_BLUETOOTH_cmd_probe2sg.xml");
        i++;
    }
#endif

}

int MainWindow::insertColumn(QString name){
    if (portToColMap.contains(name)){
        return portToColMap.value(name);

    }else{
        int i = ui->tableWidget->columnCount() ;
        ui->tableWidget->insertColumn(i);
        portToColMap.insert(name,columnCount);
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(name));
        columnCount++;
        return columnCount-1;
    }
}

QWidget *MainWindow::createTabPage(int tabIndex, QString COMName)
{
    (void)COMName;
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
    spinNLbyLenth->setObjectName("spinNLbyLenth");
    radByLength->setObjectName("radByLength");
    edtEscape->setObjectName("edtEscape");
    radByEscape->setObjectName("radByEscape");
    edtRegEx->setObjectName("edtRegEx");
    radByRegEx->setObjectName("radByRegEx");

    QStringList bdl;
    bdl << "75" << "300" << "1200" << "2400" << "4800" << "9600" << "14400" << "19200" << "28800" << "38400" << "57600" << "115200";
    comBaud->addItems(bdl);
    comBaud->setEditable(true);


    QStringList dtal;
    dtal << "5" << "6" << "7" << "8";
    comDataBits->addItems(dtal);


    QStringList stopl;
    stopl << "1" << "1.5" << "2";
    comStopBits->addItems(stopl);

    QStringList comFormatl;
    comFormatl << "HEX" << "ASCII";
    comFormat->addItems(comFormatl);

    hbox->addWidget(btnConnect);
    hbox->addWidget(colCOM);
    hbox->addWidget(colFormat);

    hbox->addStretch();

    QSettings settings(SETTINGS_FILE_NAME, QSettings::IniFormat );
    settings.beginGroup("COMName"+COMName);
    comFormat->setCurrentIndex(settings.value("Display Format",0).toInt());
    comStopBits->setCurrentIndex(settings.value("COM Stop",0).toInt());
    comDataBits->setCurrentIndex(settings.value("COM DataLength",comDataBits->count()-1).toInt());
    comBaud->setCurrentIndex(settings.value("COM Baud",comBaud->count()-1).toInt());
    spinNLbyLenth->setValue(settings.value("Display Escape Length",8).toInt());
    edtEscape->setText(settings.value("Display Escape String","\\r\\n").toString());
    edtRegEx->setText(settings.value("Display Escape Regex","").toString());
    int val = settings.value("Display Escape type",0).toInt();
    if (val == 0){
        radByLength->setChecked(true);
    }else if (val == 1){
        radByEscape->setChecked(true);
    }else{
        radByRegEx->setChecked(true);
    }
    settings.endGroup();
    connect(btnConnect,SIGNAL(clicked()),this,SLOT(on_btnConnectClicked()));
    connect(comFormat,SIGNAL(currentIndexChanged(int)),this,SLOT(on_cmb_currentIndexChanged(int)));
    connect(edtRegEx,SIGNAL(textChanged(const QString)),this,SLOT(on_lineEdit_textChanged(const QString)));
    connect(edtEscape,SIGNAL(textChanged(const QString)),this,SLOT(on_lineEdit_textChanged(const QString)));
    connect(spinNLbyLenth,SIGNAL(valueChanged(int)),this,SLOT(on_spinBox_valueChanged(int)));
    connect(radByEscape,SIGNAL(toggled(bool)),this,SLOT(on_radioButton_toggled(bool)));
    connect(radByLength,SIGNAL(toggled(bool)),this,SLOT(on_radioButton_toggled(bool)));
    connect(radByRegEx,SIGNAL(toggled(bool)),this,SLOT(on_radioButton_toggled(bool)));


    return result;
}

void MainWindow::on_cmb_currentIndexChanged(int arg1)
{
    (void)arg1;
    updateDisplayParamters(ui->tabWidget->currentIndex());
}
#if 0
void MainWindow::on_cmb_editTextChanged(const QString &arg1)
{
    (void)arg1;
    updateDisplayParamters(ui->tabWidget->currentIndex());
}
#endif
void MainWindow::on_spinBox_valueChanged(int arg1)
{
    (void)arg1;
    updateDisplayParamters(ui->tabWidget->currentIndex());
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{

    (void)arg1;
    updateDisplayParamters(ui->tabWidget->currentIndex());
}

void MainWindow::on_radioButton_toggled(bool checked)
{

    (void)checked;
    updateDisplayParamters(ui->tabWidget->currentIndex());
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

void MainWindow::updateDisplayParamters(int tabIndex){
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
    QComboBox* comFormat = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comFormat");
    bool usehex = comFormat->currentIndex()==0;
    serialPortList[tabIndex]->setAppearance(usehex);
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


           int colIndex = insertColumn(serialport->portName());
           serialPortList[tabIndex]->setColIndex(colIndex);

            updateDisplayParamters(tabIndex);



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


void MainWindow::beginNewDumpFile(){
    fileDisplay.close();
    QString myfileName = fileName+QString("_%1").arg(fileIndex, 4, 10, QChar('0'))+".csv";
    fileIndex++;
    fileRows=0;
    fileDisplay.setFileName(myfileName);
    if (!fileDisplay.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(this, "ProtocolAnalyzer",
                                      "Cant open file \""+myfileName+"\" for dumping data.",
                                      QMessageBox::Ok ,
                                      QMessageBox::Ok);
    }
}

void MainWindow::addNewEntry(QString time, QString content, QByteArray binData,  int colIndex)
{
    QTableWidgetItem *item_t=new QTableWidgetItem (time);
    QTableWidgetItem *item_c=new QTableWidgetItem (content);

    if (fileDisplay.isWritable()){
        QString row = time;
        for(int i=1; i<columnCount;i++){
            if (i == colIndex){
                row += ";\""+content+"\"";
            }else{
                row += ";\"\"";
            }
        }
        row+="\n";

        fileDisplay.write(row.toLocal8Bit(),row.length());
        fileRows++;
        if (fileRows > MAXFILEROWS){
            beginNewDumpFile();
        }
    }
    int rowindex = ui->tableWidget->rowCount() ;
    if (rowindex > MAXFILEROWS){
         ui->tableWidget->removeRow(0);
         binaryDataList.removeFirst();
         rowindex--;
    }
    QPair<int,QByteArray> binEntry = QPair<int,QByteArray>(colIndex,binData);
    binaryDataList.append(binEntry);
    ui->tableWidget->insertRow(rowindex);
    ui->tableWidget->setItem(rowindex,0,item_t);
    ui->tableWidget->setItem(rowindex,colIndex,item_c);

    ui->tableWidget->scrollToBottom();
}


void MainWindow::on_readyRead()
{

}




void MainWindow::on_actionPause_triggered()
{
    bool pause;

    if (ui->actionPause->property("resumt").toBool() == false){
        ui->actionPause->setText("Resume");
        ui->actionPause->setProperty("resumt",true);
        pause = true;
    }else{
        ui->actionPause->setText("Pause");
        ui->actionPause->setProperty("resumt",false);
        pause = false;
    }


    for(int i=0;i< serialPortList.count(); i++){
        serialPortList[i]->setPause(pause);
    }

}

void MainWindow::on_tableWidget_itemSelectionChanged()
{

    int row = ui->tableWidget->currentRow();
   // int col = ui->tableWidget->currentColumn();
    QPair<int,QByteArray> binEntry = binaryDataList[row];
    SerialNode* serialNode = serialPortList[binEntry.first];
    RPCRuntimeDecoder decoder = serialNode->getPackageDecoder();
    if (serialNode->isUsingChannelCodec()){
        decoder.RPCDecodeChannelCodedData(binEntry.second);
    }else{
        decoder.RPCDecodeRPCData(binEntry.second);
    }
    ui->treeWidget->clear();
    ui->treeWidget->insertTopLevelItems(0, decoder.getTreeWidgetReport(NULL));
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);
}

void MainWindow::on_actionTestDecode_triggered()
{
    const uint8_t inBinData_array[] = {0x18 ,0x2b ,0x00 ,0x48 ,0x61 ,0x6c ,0x6c ,0x6f ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x38 ,0x39 ,0x30,
                                       0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x34 ,0x38 ,0x39 ,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35,
                                       0x36 ,0x37 ,0x38 ,0x39 ,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x38 ,0x00 ,0x10 ,0x20,
                                       0x01 ,0x11 ,0x21};
    QByteArray inBinData = QByteArray((char*)inBinData_array, sizeof(inBinData_array));
    RPCRunTimeProtocolDescription rpcinterpreter;
    rpcinterpreter.openProtocolDescription("C:/Users/ark/entwicklung/qt/qProtocolInterpreter/project/tests/scripts/decodeTest_struct_int.xml");
    RPCRuntimeDecoder decoder(rpcinterpreter);
    decoder.RPCDecodeRPCData(inBinData);

    ui->treeWidget->clear();
    ui->treeWidget->insertTopLevelItems(0, decoder.getTreeWidgetReport(NULL));


     ui->treeWidget->expandAll();

     ui->treeWidget->resizeColumnToContents(0);
     ui->treeWidget->resizeColumnToContents(1);
}
