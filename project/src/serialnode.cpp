#include "serialnode.h"
#include "mainwindow.h"
#include <QDateTime>
#include <QDebug>
SerialNode::SerialNode(QObject *parent) :
    QObject(parent)
{
    serialport = new QSerialPort();
    timeoutTimer = new QTimer(this);
    connect(serialport,SIGNAL(readyRead()),this,SLOT(on_readyRead()));
    connect(timeoutTimer,SIGNAL(timeout()),this,SLOT(on_timeout()));
}

SerialNode::~SerialNode()
{
    delete serialport;
}

void SerialNode::setColIndex(int i)
{
    colIndex = i;
}

void SerialNode::setAppearance(bool useHex)
{
    if (useHex)
        nodeAppearance = nodeAppearence_t::hex;
    else
        nodeAppearance = nodeAppearence_t::ascii;
}

void SerialNode::setEscaping(nodeEscaping_t escaping)
{
    nodeEscaping = escaping;
}

void SerialNode::setEscapeRegEx(QString escaping)
{
    escapeString = escaping;
}

void SerialNode::setEscapeChar(QString escaping)
{
    int i = escaping.indexOf("\\n");
    while (i > -1){

        escaping.replace(i,2,"\n");
        i = escaping.indexOf("\\n");
    }
    i = escaping.indexOf("\\r");
    while (i > -1){

        escaping.replace(i,2,"\r");
        i = escaping.indexOf("\\r");
    }
    i = escaping.indexOf("\\t");
    while (i > -1){

        escaping.replace(i,2,"\t");
        i = escaping.indexOf("\\t");
    }
    escapeString = escaping;
}

void SerialNode::setEscapeLength(int escapeLength)
{
   this->escapeLength = escapeLength;
}

bool SerialNode::isNewLine(const QByteArray lineBin){
    bool result = false;
    switch(nodeEscaping){
    case nodeEscaping_t::byLength:
        if (lineBin.length() == escapeLength){
            result = true;
        }else{

        }
        break;
    case nodeEscaping_t::byEscapeCharacter:
        {
            if(escapeString.count() > lineBin.count())
                break;

            bool found = true;

            for (int i = 0; i<escapeString.count(); i++) {
                QChar l=escapeString[i];
                QChar r=lineBin[lineBin.count()-escapeString.count()+i];
                if(l != r){
                    found = false;
                    break;
                }
                result = found;
            }
        }
        break;
    case nodeEscaping_t::byRegEx:
        break;
    }
    return result;

}

void SerialNode::addLine(){
    QString line;
    if (nodeAppearance == nodeAppearence_t::hex){
        line = lineBufferHex;
    }else{
        line = "";
        for (int i = 0;i<lineBufferBin.count();i++){
            if ((0x20 <= lineBufferBin.at(i)) && (lineBufferBin.at(i) < 0x7F)){
                line += lineBufferBin.at(i);
            }else{
                line += '\\'+QString("%1 ").arg((uint8_t)(lineBufferBin.at(i)), 2, 16, QChar('0'));
            }

        }
    }


    QString s = inComingTime.toString("MM.dd HH:mm:ss.zzz");
    MainWindow* mainwin = qobject_cast<MainWindow*>(parent());
    mainwin->addNewEntry(s,line,colIndex);

    lineBufferBin.clear();
    lineBufferHex.clear();

}

void SerialNode::on_readyRead()
{
    static int oldByte=0;
    inComingTime = QDateTime::currentDateTime ();
    timeoutTimer->stop();
    QByteArray inbuffer = serialport->readAll();
    if (inbuffer.count() == 512)
        qDebug() << "Rechner langsam";
    qDebug() << "reading: " << inbuffer.count();
    for (int i = 0;i<inbuffer.count();i++){
        if (oldByte==inbuffer[i]){
            //qDebug() << "gleiche bytes" << i << "von" << inbuffer.count();
        }
        oldByte=inbuffer[i];
        lineBufferBin.append(inbuffer[i]);
        if (nodeAppearance == nodeAppearence_t::hex){
            lineBufferHex.append(QString("%1 ").arg((uint8_t)(inbuffer.at(i)), 2, 16, QChar('0')));
        }

        if (isNewLine(lineBufferBin)){
            addLine();
        }
    }
    if (lineBufferBin.count()){
        timeoutTimer->start(300);
    }

}

void SerialNode::on_timeout()
{
    timeoutTimer->stop();
    addLine();

}

