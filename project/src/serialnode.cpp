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

    pause = false;
}

SerialNode::~SerialNode()
{
    delete serialport;
}

void SerialNode::setColIndex(int i)
{
    colIndex = i;
}

void SerialNode::setTabIndex(int i)
{
    tabIndex = i;
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
    escapeStringDisplay = escaping;
}

void SerialNode::setEscapeChar(QString escaping)
{

    escapeStringRaw.clear();
    escapeStringDisplay.clear();

    int index = 0;
    while (index<escaping.count()){
        QString mid = escaping.mid(index);
        if (mid.startsWith("\\n")){
            escapeStringRaw.append('\n');
            escapeStringDisplay.append("\\0A");
            index+=2;
        }else if(mid.startsWith("\\r")){
            escapeStringRaw.append('\r');
            escapeStringDisplay.append("\\0D");
            index+=2;
        }else{
            escapeStringRaw.append(mid.at(0));
            escapeStringDisplay.append(mid.at(0));
            index+=1;
        }
    }
}

void SerialNode::setEscapeLength(int escapeLength)
{
    this->escapeLength = escapeLength;
}

void SerialNode::setRPCDescriptionFileName(QString fn)
{
    rpcinterpreter.openProtocolDescription(fn);
    rpcDecoder = RPCRuntimeDecoder(rpcinterpreter);
}

void SerialNode::setDecodeType(nodeDecoderType_t decType)
{
    (void)decType;
}

RPCRuntimeDecoder SerialNode::getPackageDecoder()
{

    return rpcDecoder;
}

bool SerialNode::isUsingChannelCodec()
{
    return true;
}

void SerialNode::setPause(bool pause)
{
    this->pause = pause;
}

void SerialNode::addWatchPoint(QString FieldID, QString humanReadableName, QPair<int, int> plotIndex, watchCallBack_t callback)
{
    rpcDecoder.addWatchPoint(FieldID,humanReadableName,plotIndex,callback);
}

void SerialNode::removeWatchPoint(QString FieldID)
{
    rpcDecoder.removeWatchPoint(FieldID);
}

void SerialNode::clearWatchPoint()
{
    rpcDecoder.clearWatchPoint();
}

bool SerialNode::isNewLine(const QByteArray lineRaw, const QString lineString){
    bool result = false;
    switch(nodeEscaping){
    case nodeEscaping_t::byLength:
        if (lineRaw.length() >= escapeLength){
            result = true;
        }else{

        }
        break;
    case nodeEscaping_t::byEscapeCharacter:
        {
            int comparePosStart;
            QByteArray escapeString_;
            QByteArray lineToTest;
            if (nodeAppearance == nodeAppearence_t::hex){
                escapeString_ = escapeStringRaw;
                lineToTest = lineRaw;
            }else{
                escapeString_ = escapeStringDisplay.toLocal8Bit();
                lineToTest = lineString.toLocal8Bit();
            }
            comparePosStart = lineToTest.count();
            comparePosStart -= escapeString_.count();

            if(comparePosStart < 0)
                break;

            bool found = true;
            for (int i = 0; i<escapeString_.count(); i++) {

                uint8_t l=escapeString_[i];
                uint8_t r=lineToTest[comparePosStart+i];
                if(l != r){
                    found = false;
                    break;
                }

            }
            result = found;
        }
        break;
    case nodeEscaping_t::byRegEx:
        break;
    }
    return result;

}

void SerialNode::addLine(){
    QString s = inComingTime.toString("MM.dd HH:mm:ss.zzz");
    MainWindow* mainwin = qobject_cast<MainWindow*>(parent());
    if (rpcDecoder.getWatchPointList().count()){
        rpcDecoder.setTimeStamp(inComingTime);
        if (isUsingChannelCodec()){
            rpcDecoder.RPCDecodeChannelCodedData(lineBufferRaw);
        }else{
            rpcDecoder.RPCDecodeRPCData(lineBufferRaw);
        }
    }
    mainwin->addNewEntry(s,lineBufferDisplay,lineBufferRaw, colIndex, tabIndex);

    lineBufferRaw.clear();
    lineBufferDisplay.clear();

}

void SerialNode::on_readyRead()
{
    static int oldByte=0;

    inComingTime = QDateTime::currentDateTime ();
    timeoutTimer->stop();
    QByteArray inbuffer = serialport->readAll();
    if (pause)
        return;
    if (inbuffer.count() == 512)
        qDebug() << "Rechner langsam";
    //qDebug() << "reading: " << inbuffer.count();
    for (int i = 0;i<inbuffer.count();i++){
        if (oldByte==inbuffer.at(i)){
            //qDebug() << "gleiche bytes" << i << "von" << inbuffer.count();
        }
        oldByte=inbuffer.at(i);
        lineBufferRaw.append(inbuffer.at(i));

        if (nodeAppearance == nodeAppearence_t::hex){
            lineBufferDisplay.append(QString("%1 ").arg((uint8_t)(inbuffer.at(i)), 2, 16, QChar('0')).toUpper());
        }else if(nodeAppearance == nodeAppearence_t::ascii){
            if ((0x20 <= inbuffer.at(i)) && (inbuffer.at(i) < 0x7F)){
                lineBufferDisplay += inbuffer.at(i);
            }else{
                lineBufferDisplay += '\\'+QString("%1").arg((uint8_t)(inbuffer.at(i)), 2, 16, QChar('0')).toUpper();
            }
        }

        if (isNewLine(lineBufferRaw,lineBufferDisplay)){
            addLine();
        }
    }
    if (lineBufferRaw.count()){
        timeoutTimer->start(300);
    }

}

void SerialNode::on_timeout()
{
    timeoutTimer->stop();
    addLine();

}

