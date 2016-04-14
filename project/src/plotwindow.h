#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QFile>
#include <qwt/qwt_plot_curve.h>

namespace Ui {
class PlotWindow;
}

class PlotCurveEntrySearchResult;

class PlotCurveEntry{
public:
    PlotCurveEntry();

    ~PlotCurveEntry();

    QString FieldID;
    QString humanReadableName;

    QwtPlotCurve* plotCurve;

    void addPlotPoint(QDateTime timeStamp, int64_t value);
    QVector<double> values;
    QVector<double> timeaxis;

    void resetCurve();


};



class PlotEntry{
public:
    PlotEntry();
    ~PlotEntry();

    QPair<int,int>plotIndex;

    QwtPlot* plot;

    PlotCurveEntrySearchResult getPlotCurveByFieldID(QString FieldID);

    QList<PlotCurveEntry*> plotCurveEntries;

    void addPlotCurve(QString FieldID, QString humanReadableName);
    void removeCurve(QString FieldID);
    QList<QColor> colorList;

    void dumpPlotPointToFile(QDateTime timeStamp, int64_t value, int colIndex);

    QFile dumpFile;
    int fileIndex;
    int fileLines;


    void resetCurves();

    void openDumpFile();
    QDateTime dumpFileStartTime;


    void resetPlotZoom();
private:

};

class PlotCurveEntrySearchResult{
public:
    PlotCurveEntrySearchResult()
    {
        pce = NULL;
        pe = NULL;
        pcIndex = 0;

    }
    PlotCurveEntry* pce;
    PlotEntry* pe;
    int pcIndex;
};

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = 0);
    ~PlotWindow();



    PlotEntry* getPlotByFieldID(QString FieldID);

    PlotEntry* getPlotByPlotIndex(QPair<int,int> plotIndex);


    void addPlotPoint(QString FieldID, QString humanReadableName, QPair<int,int> plotIndex, QDateTime timeStamp, int64_t value);

    void removeCurve(QString FieldID);

    PlotEntry* addPlot(QPair<int,int> plotIndex);


    bool curveExists(QString FieldID);
private slots:
    void on_action_clear_triggered();

    void on_action_zoom_reset_triggered();
private:
    Ui::PlotWindow *ui;

    QList<PlotEntry*> plots;

    void setupWheelZooming(QwtPlot *plot);

    PlotCurveEntrySearchResult getPlotCurveByFieldID(QString FieldID);
};

#endif // PLOTWINDOW_H
