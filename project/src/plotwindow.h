#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <qwt/qwt_plot_curve.h>

namespace Ui {
class PlotWindow;
}

class PlotCurveEntry{
public:
    PlotCurveEntry();

    ~PlotCurveEntry();

    QString FieldID;
    QString humanReadableName;

    QwtPlotCurve* plotCurve;

    void addPlotPoint(QDateTime timeStamp, int64_t value);

};

class PlotEntry{
public:
    PlotEntry();
    ~PlotEntry();

    QPair<int,int>plotIndex;

    QwtPlot* plot;

    PlotCurveEntry* getPlotCurveByFieldID(QString FieldID);

    QList<PlotCurveEntry*> plotCurveEntries;

    void addPlotCurve(QString FieldID, QString humanReadableName);


};

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = 0);
    ~PlotWindow();

    void addPlotCurve(QPair<int,int> plotindex, QString FieldIndex ,QString humanReadableName);
    PlotCurveEntry* getPlotCurveByFieldID(QString FieldID);



    PlotEntry* getPlotByFieldID(QString FieldID);

    PlotEntry* getPlotByPlotIndex(QPair<int,int> plotIndex);

    void addPlotPoint(QString FieldID, QString humanReadableName, QPair<int,int> plotIndex, QDateTime timeStamp, int64_t value);

    PlotEntry* addPlot(QPair<int,int> plotIndex);

private:
    Ui::PlotWindow *ui;

    QList<PlotEntry*> plots;

};

#endif // PLOTWINDOW_H
