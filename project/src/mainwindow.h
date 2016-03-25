#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>
#include <QFile>
#include <QDialog>
#include <QSpinBox>
#include "serialnode.h"
#include "plotwindow.h"

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

private slots:



    void on_cmb_currentIndexChanged(int arg1);
    //void on_cmb_editTextChanged(const QString &arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_radioButton_toggled(bool checked);

    void on_actionPause_triggered();



    void on_tableWidget_itemSelectionChanged();

    void on_actionTestDecode_triggered();

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionAddToPlot_triggered();

    void on_actionRemoveFromPlot_triggered();

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
    PlotWindow* plotwindow;

    QList<QPair<int,QByteArray>> binaryDataList;

    void watchPointCallback(QString FieldID, QString humanReadableName, QPair<int, int> plotIndex, QDateTime timeStamp, int64_t value);
};

class AddToPlotDialog : public QDialog
{
        Q_OBJECT
public:

    AddToPlotDialog(QString FieldID, QString humanReadableName, QWidget * parent = 0, Qt::WindowFlags f = 0);

    ~AddToPlotDialog();
    QPair<int,int> getIndex();
    QString getFieldID();
    QString getHumanReadableName();
private:
   QPair<int,int> index;
   QSpinBox *sb_x;
   QSpinBox *sb_y;
   QString FieldID;
   QString humanReadableName;
private slots:
   void acceptAndSetIndex();
};

#endif // MAINWINDOW_H
