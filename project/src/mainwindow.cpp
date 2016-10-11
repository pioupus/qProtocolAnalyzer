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
#include <QFileInfo>
#include <QDialogButtonBox>
#include <functional>
#include <QFileDialog>
#include"../libs/qRPCRuntimeParser/project/src/channel_codec_wrapper.h"
#include"../libs/qRPCRuntimeParser/project/src/rpcruntime_decoded_function_call.h"
#include"../libs/qRPCRuntimeParser/project/src/rpc_ui.h"

//#include "channel_codec_wrapper.h"


const int MAXFILEROWS = 200*1000;
const QString SETTINGS_FILE_NAME = QDir::currentPath()+QDir::separator()+"protanalyzer.ini";

Q_DECLARE_METATYPE(QStringList)

static bool fileExists(QString fn){
    QFileInfo checkFile(fn);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile()) {
        return true;
    } else {
        return false;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    testTimer = new QTimer(this);
    plotwindow = new PlotWindow(this);
    QSettings settings(SETTINGS_FILE_NAME, QSettings::IniFormat );

    QString basePath = settings.value("ExportFilePath",QDir::currentPath()+QDir::separator()+"export").toString();
    settings.setValue("ExportFilePath",QDir::currentPath()+QDir::separator()+"export");

    QString dateStr =  QDateTime::currentDateTime().toString("MM_dd__HH_mm_ss_");

    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));

    columnCount = 1;
    refreshTabs();
    fileIndex=0;
    fileRows=0;
    if (!basePath.endsWith(QDir::separator())){
        basePath+= QDir::separator();
    }
    plotwindow->setDumpFilePath(basePath);
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


        QComboBox* comDecoder = widget->findChild<QComboBox*>("comDecoder");
        QComboBox* comRPCFile = widget->findChild<QComboBox*>("comRPCFile");

        QSettings settings(SETTINGS_FILE_NAME, QSettings::IniFormat );
        settings.beginGroup("COMName"+ui->tabWidget->tabText(i));
		
        settings.setValue("Decoder",comDecoder->currentIndex());
        settings.setValue("RPCFile",comRPCFile->currentText());
        QStringList rpcFiles;
        for(int i=0;i<comRPCFile->count();i++){
            rpcFiles.append(comRPCFile->itemText(i));
        }
        settings.setValue("RPCFiles",rpcFiles);
		
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
	    QWidget* colDecoder = new QWidget(result);
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

    QFormLayout *flayoutDecoder = new QFormLayout(colDecoder);
    QComboBox* comDecoder = new QComboBox(colDecoder);
    QHBoxLayout *hboxRPCFile = new QHBoxLayout(colDecoder);
    QComboBox* comRPCFile = new QComboBox(colDecoder);
    QPushButton* btnBrowseRPCFile = new QPushButton(colDecoder);

    hboxRPCFile->addWidget(comRPCFile);
    hboxRPCFile->addWidget(btnBrowseRPCFile);

    flayoutDecoder->addRow("Decoder",comDecoder);
    flayoutDecoder->addRow("Description File",hboxRPCFile);
    btnBrowseRPCFile->setText("...");
    comRPCFile->setEditable(true);
    comDecoder->setObjectName("comDecoder");
    comRPCFile->setObjectName("comRPCFile");
    comRPCFile->setMinimumWidth(200);
	
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

	    QStringList comDecoderl;
    comDecoderl << "none" << "RPC with channel codec" << "RPC" << "0xAA framed uint" << "space seperated hex";
    comDecoder->addItems(comDecoderl);


	
    hbox->addWidget(btnConnect);
    hbox->addWidget(colCOM);
    hbox->addWidget(colFormat);
    hbox->addWidget(colDecoder);
   
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
    comRPCFile->setEditText(settings.value("RPCFile","").toString());
    comDecoder->setCurrentIndex(settings.value("Decoder",0).toInt());
	
    QStringList rpcFiles = settings.value("RPCFiles").toStringList();
    comRPCFile->addItems(rpcFiles);
    if (comRPCFile->findText(comRPCFile->currentText())==-1){
        comRPCFile->addItem(comRPCFile->currentText());
    }

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
    connect(comDecoder,SIGNAL(currentIndexChanged(int)),this,SLOT(on_cmb_codecChanged(int)));

    connect(comRPCFile,SIGNAL(editTextChanged(QString)),this,SLOT(on_decodeFile_changed(const QString)));
    connect(btnBrowseRPCFile,SIGNAL(clicked()),this,SLOT(on_btnRPCFile_Browse_clicked()));

    return result;
}


void MainWindow::on_btnRPCFile_Browse_clicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button){
        int tabIndex = button->property("tabIndex").toInt();
        QComboBox* cmb = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comRPCFile");
        QString fn = cmb->currentText();
        if (cmb){
            QFileDialog *fdiag = new QFileDialog(this, "open RPC description file", fn, "XML files (*.xml);;All files (*.*)");
            //QStringList filter;

            //filter  << "XML files (*.xml)" << "All files (*.*)";
            //fdiag.setFilters(filter);
            //fdiag->selectFile();
            if (fdiag->exec()){
                if(fdiag->selectedFiles().count()){
                    fn = fdiag->selectedFiles()[0];
                    int index = cmb->findText(fn);
                    if (index == -1){
                        cmb->addItem(fn);
                        index = cmb->count()-1;
                    }
                    cmb->setCurrentIndex(index);
                }
            }
        }
    }

}

void MainWindow::on_cmb_codecChanged(int arg1)
{
    (void)arg1;//codec
    QComboBox* combobox = qobject_cast<QComboBox*>(sender());
    if (combobox){
        int tabIndex = ui->tabWidget->currentIndex();
        SerialNode* serialNode = serialPortList[tabIndex];
        nodeDecoderType_t decType=nodeDecoderType_t::none;
        switch(combobox->currentIndex()){
            case 0:
            decType=nodeDecoderType_t::none;
            break;
        case 1:
        decType=nodeDecoderType_t::rpcWithCodec;
        break;
        case 2:
        decType=nodeDecoderType_t::rpc;
        break;
        case 3:
        decType=nodeDecoderType_t::AAFramedUint;
        break;
        case 4:
        decType=nodeDecoderType_t::spaceSeperatedHex;
        break;
        }
        serialNode->setDecodeType(decType);

    }
}

void MainWindow::on_decodeFile_changed(const QString &arg1)
{
    (void)arg1;//rpc file
    QComboBox* combobox = qobject_cast<QComboBox*>(sender());
    if (combobox){
        int tabIndex = ui->tabWidget->currentIndex();
        if (fileExists(combobox->currentText())){
            int i = combobox->findText(combobox->currentText());
            if(i==-1){
                combobox->addItem(combobox->currentText());
            }
            SerialNode* serialNode = serialPortList[tabIndex];
            serialNode->setRPCDescriptionFileName(combobox->currentText());
        }
    }
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
       SerialNode* serialNode = serialPortList[tabIndex];

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

           QComboBox* comRPCFile = ui->tabWidget->widget(tabIndex)->findChild<QComboBox*>("comRPCFile");


           serialNode->setRPCDescriptionFileName(comRPCFile->currentText());

           int colIndex = insertColumn(serialport->portName());
           serialPortList[tabIndex]->setColIndex(colIndex);
           serialPortList[tabIndex]->setTabIndex(tabIndex);

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

void MainWindow::addNewEntry(QString time, QString content, QByteArray binData,  int colIndex, int tabIndex)
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
    QPair<int,QByteArray> binEntry = QPair<int,QByteArray>(tabIndex,binData);
    binaryDataList.append(binEntry);
    ui->tableWidget->insertRow(rowindex);
    ui->tableWidget->setItem(rowindex,0,item_t);
    ui->tableWidget->setItem(rowindex,colIndex,item_c);

    ui->tableWidget->scrollToBottom();
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
    int col = ui->tableWidget->currentColumn();
    bool complete = false;
    QPair<int,QByteArray> binEntry = binaryDataList[row];
    SerialNode* serialNode = serialPortList[binEntry.first];
    RPCRuntimeDecoder decoder = serialNode->getPackageDecoder();

    std::unique_ptr<QTreeWidgetItem> items;
    std::vector<unsigned char> inData(binEntry.second.data(),binEntry.second.data()+binEntry.second.count());

    if (serialNode->isUsingChannelCodec()){
        Channel_codec_wrapper cc(decoder);


        cc.add_data(inData);

       if (cc.transfer_complete()){
           complete = true;
           RPCRuntimeDecodedFunctionCall function_call = cc.pop();
           items = getTreeWidgetReport(function_call);
       }
        //decoder.RPCDecodeChannelCodedData(binEntry.second);
    }else{
        RPCRuntimeTransfer transfer = decoder.decode(inData);
        if (transfer.is_complete()){
            complete = true;
            RPCRuntimeDecodedFunctionCall function_call = transfer.decode();
            items = getTreeWidgetReport(function_call);
        }

        // QCOMPARE(transfer.is_complete(), true);
        //QCOMPARE(transfer.get_min_number_of_bytes(), static_cast<int>(sizeof inBinData_array));

        // function_call = transfer.decode();

        //decoder.RPCDecodeRPCData(binEntry.second);
    }

    if (complete){


        ui->treeWidget->clear();
        ui->treeWidget->addTopLevelItem(items.release());
        ui->treeWidget->expandAll();
        ui->treeWidget->resizeColumnToContents(0);
        ui->treeWidget->resizeColumnToContents(1);
    }
    ui->edtTextToCopy->setText(ui->tableWidget->item(row,col)->text());
    QList<QTableWidgetItem *> iList = ui->tableWidget->selectedItems();

    if (iList.count()>1){

        int row_first = iList.first()->row();
        int row_last = iList.last()->row();
        QString time_str_first = ui->tableWidget->item(row_first,0)->text();
        QString time_str_last = ui->tableWidget->item(row_last,0)->text();
        QDateTime time_first = QDateTime::fromString(time_str_first,"MM.dd HH:mm:ss.zzz");
        QDateTime time_last = QDateTime::fromString(time_str_last,"MM.dd HH:mm:ss.zzz");
        int msecs = time_first.msecsTo(time_last);
        int secs = msecs/1000;
        msecs = abs(msecs%1000);
        //QString msecsStr = ;
        ui->lblTimeDiff->setText("Time difference: "+QString::number(secs)+"."+QString("%1").arg(msecs, 3, 10, QChar('0'))+" s" );
    }else{
        ui->lblTimeDiff->setText("Time difference: 0.000 s" );
    }


}

void MainWindow::on_actionTestDecode_triggered()
{
#if 0
    const uint8_t inBinData_array[] = {0x18 ,0x2b ,0x00 ,0x48 ,0x61 ,0x6c ,0x6c ,0x6f ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x38 ,0x39 ,0x30,
                                       0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x34 ,0x38 ,0x39 ,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35,
                                       0x36 ,0x37 ,0x38 ,0x39 ,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x38 ,0x00 ,0x10 ,0x20,
                                       0x01 ,0x11 ,0x21};
    QByteArray inBinData = QByteArray((char*)inBinData_array, sizeof(inBinData_array));
    RPCRunTimeProtocolDescription rpcinterpreter;
    rpcinterpreter.openProtocolDescription("../../../project/libs/qRPCRuntimeParser/project/tests/scripts/decodeTest_struct_int.xml");
    RPCRuntimeDecoder decoder(rpcinterpreter);
    decoder.RPCDecodeRPCData(inBinData);

    ui->treeWidget->clear();
    //QTreeWidgetItem * rootWidget = new QTreeWidgetItem(ui->treeWidget,(QTreeWidgetItem*)NULL);
    ui->treeWidget->addTopLevelItems(decoder.getTreeWidgetReport(NULL));



    ui->treeWidget->expandAll();
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);
#endif
}

void MainWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    (void)pos;
    if (ui->treeWidget->selectedItems().count() > 0){
        int row = ui->tableWidget->currentRow();
        QTreeWidgetItem* selectedItem = ui->treeWidget->selectedItems()[0];
        QString FieldID = selectedItem->data(0,Qt::UserRole).toString();

        QPair<int,QByteArray> binEntry;
        SerialNode* serialNode = NULL;


        if ((row > 0) && (row < binaryDataList.count())){
            binEntry = binaryDataList[row];
            serialNode = serialPortList[binEntry.first];
            if(serialNode){
                #if WATCHPOINT==1
                RPCRuntimeParameterDescription::Type paramType = serialNode->getPackageDecoder().getParamDescriptionByFieldID(FieldID).rpcParamType;
                if (paramType == RPCParamType_t::param_int) {
                    QMenu contextMenu(tr("Context menu"), this);

                    QAction action_addToPlot("add to plot", &contextMenu);
                    connect(&action_addToPlot, SIGNAL(triggered()), this, SLOT(on_actionAddToPlot_triggered()));
                    contextMenu.addAction(&action_addToPlot);

                    QAction action_removeFromPlot("remove from plot", &contextMenu);
                    connect(&action_removeFromPlot, SIGNAL(triggered()), this, SLOT(on_actionRemoveFromPlot_triggered()));
                    contextMenu.addAction(&action_removeFromPlot);


                    action_removeFromPlot.setVisible(plotwindow->curveExists(FieldID));




                    contextMenu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
                }
                #endif
            }
        }
    }
}

void MainWindow::watchPointCallback(QString FieldID, QString humanReadableName, QPair<int,int> plotIndex, QDateTime timeStamp, int64_t value){

    plotwindow->addPlotPoint( FieldID,  humanReadableName, plotIndex,  timeStamp, value);
}

void MainWindow::on_actionAddToPlot_triggered()
{
    #if WATCHPOINT==1
    if (ui->treeWidget->selectedItems().count() > 0){

        int row = ui->tableWidget->currentRow();


        QTreeWidgetItem* selectedItem = ui->treeWidget->selectedItems()[0];
        QString FieldID = selectedItem->data(0,Qt::UserRole).toString();

        QPair<int,QByteArray> binEntry;
        SerialNode* serialNode = NULL;

        QString humanReadableName = selectedItem->text(0);

        if ((row > 0) && (row < binaryDataList.count())){
                binEntry = binaryDataList[row];
                serialNode = serialPortList[binEntry.first];

                humanReadableName = serialNode->serialport->portName()+": "+selectedItem->text(0);
        }
        AddToPlotDialog dialog(FieldID, humanReadableName);
        if (dialog.exec() == QDialog::Accepted){
            QPair<int,int> plotIndex = dialog.getIndex();

            watchCallBack_t callback = std::bind(&MainWindow::watchPointCallback, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) ;

            if (serialNode){
                serialNode->addWatchPoint(dialog.getFieldID(),dialog.getHumanReadableName(),plotIndex,callback);
            }
            plotwindow->show();
            watchPointCallback(dialog.getFieldID(),dialog.getHumanReadableName(),plotIndex,QDateTime(),3);

        }
    }
#endif
}

void MainWindow::on_actionRemoveFromPlot_triggered()
{
    #if WATCHPOINT==1
    if (ui->treeWidget->selectedItems().count() > 0){

        int row = ui->tableWidget->currentRow();


        QTreeWidgetItem* selectedItem = ui->treeWidget->selectedItems()[0];
        QString FieldID = selectedItem->data(0,Qt::UserRole).toString();

        QPair<int,QByteArray> binEntry;
        SerialNode* serialNode = NULL;

        if ((row > 0) && (row < binaryDataList.count())){
                binEntry = binaryDataList[row];
                serialNode = serialPortList[binEntry.first];
        }
        if (serialNode){
            serialNode->removeWatchPoint(FieldID);
        }
        plotwindow->removeCurve(FieldID);
    }
    #endif
}


AddToPlotDialog::AddToPlotDialog(QString FieldID, QString humanReadableName, QWidget *parent, Qt::WindowFlags f): QDialog( parent, f )
{
    this->FieldID = FieldID;
    this->humanReadableName = humanReadableName;

    sb_x = new QSpinBox;
    sb_y = new QSpinBox;

    QGridLayout* gLayout = new QGridLayout;
    QVBoxLayout* vLayout = new QVBoxLayout;


    setWindowTitle("Add To Plot");
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                       | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptAndSetIndex()));

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));



    gLayout->addWidget(sb_x, 0,0);
    gLayout->addWidget(sb_y, 0,1);
    vLayout->addItem(gLayout);
    vLayout->addWidget(buttonBox);
    setLayout(vLayout);
}

AddToPlotDialog::~AddToPlotDialog(){

}

QPair<int, int> AddToPlotDialog::getIndex()
{
    return index;
}

QString AddToPlotDialog::getFieldID()
{
    return FieldID;
}

QString AddToPlotDialog::getHumanReadableName()
{
    return humanReadableName;
}

void AddToPlotDialog::acceptAndSetIndex()
{
    index.first = sb_x->value();
    index.second = sb_y->value();
    accept();
}

void MainWindow::on_actionTestPlot1_triggered()
{
    plotwindow->show();
    watchPointCallback("test1","COM1 TEst",QPair<int,int>(0,0),QDateTime(),3);
    connect(testTimer, SIGNAL(timeout()), this, SLOT(onTestTimerTriggered()));
    testTimer->start(10);
}

void MainWindow::onTestTimerTriggered()
{
    watchPointCallback("test1","COM1 TEst",QPair<int,int>(0,0),QDateTime::currentDateTime(),qrand());
}



