#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_btnConnectClicked();


private:
    Ui::MainWindow *ui;

    void refreshTabs(void);

    QWidget* createTabPage(int tabIndex);

    void clearSerialPorts();
   QList<QSerialPort*> serialPortList;
   QList<QSerialPort*> openPortList;

   QSerialPort::StopBits stopBitDescriptionToStopBit(QString desc);
};

#endif // MAINWINDOW_H
