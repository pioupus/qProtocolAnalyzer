#ifndef SERIALNODE_H
#define SERIALNODE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPort>

#include"../libs/qRPCRuntimeParser/project/src/rpcruntime_protocol_description.h"

enum class nodeAppearence_t { hex, ascii};
enum class nodeEscaping_t { byLength, byEscapeCharacter, byRegEx};



class SerialNode : public QObject
{
    Q_OBJECT
public:
    explicit SerialNode(QObject *parent = 0);
    ~SerialNode();
    void setColIndex(int i);
    void setAppearance(bool useHex);
    void setEscaping(nodeEscaping_t escaping);
    void setEscapeRegEx(QString escaping);
    void setEscapeChar(QString escaping);
    void setEscapeLength(int escapeLength);

    RPCRunTimeProtocolDescription rpcinterpreter;
    void setPause(bool pause);
   QSerialPort* serialport;

signals:

public slots:
    void on_readyRead();
    void on_timeout();

private:
    bool isNewLine(const QByteArray lineRaw, const QString lineString);

    int colIndex;
    nodeAppearence_t nodeAppearance;
    QString escapeStringDisplay;
    QByteArray escapeStringRaw;
    nodeEscaping_t nodeEscaping;
    QByteArray lineBufferRaw;
    QString lineBufferDisplay;
    QTimer* timeoutTimer;
    int escapeLength;

    QDateTime inComingTime;
    void addLine();
    bool pause;

};

#endif // SERIALNODE_H
