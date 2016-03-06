#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>

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
    void addNewEntry(QString time, QString content, int colIndex);

public slots:
    void on_btnConnectClicked();
    void on_readyRead();

private:
    Ui::MainWindow *ui;

    void refreshTabs(void);

    QWidget* createTabPage(int tabIndex);

    void clearSerialPorts();
    QList<SerialNode*> serialPortList;


    QSerialPort::StopBits stopBitDescriptionToStopBit(QString desc);

    QMap<QString, int> portToColMap;
    int columnCount;
    int insertColumn(QString name);
    bool isNewLine(QSerialPort *serPort, int count, QString inString);

};

#endif // MAINWINDOW_H
