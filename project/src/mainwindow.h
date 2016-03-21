#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>
#include <QFile>

#include "serialnode.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void addNewEntry(QString time, QString content, QByteArray binData, int colIndex);

public slots:
    void on_btnConnectClicked();
    void on_readyRead();

private slots:



    void on_cmb_currentIndexChanged(int arg1);
    //void on_cmb_editTextChanged(const QString &arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_radioButton_toggled(bool checked);

    void on_actionPause_triggered();



    void on_tableWidget_itemSelectionChanged();

private:
    Ui::MainWindow *ui;

    void refreshTabs(void);

    QWidget* createTabPage(int tabIndex, QString COMName);

    void clearSerialPorts();
    QList<SerialNode*> serialPortList;


    QSerialPort::StopBits stopBitDescriptionToStopBit(QString desc);
    void updateDisplayParamters(int tabIndex);

    QMap<QString, int> portToColMap;
    int columnCount;
    int insertColumn(QString name);
    bool isNewLine(QSerialPort *serPort, int count, QString inString);
    void beginNewDumpFile();
    QFile fileDisplay;
    QString fileName;
    int fileIndex;
    int fileRows;

    QList<QPair<int,QByteArray>> binaryDataList;

};

#endif // MAINWINDOW_H
