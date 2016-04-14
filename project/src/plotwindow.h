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

    void addPlotPoint(QDateTime timeStamp, int64_t value);
    void resetCurve();

    QString FieldID;
    QString humanReadableName;
    QwtPlotCurve* plotCurve;
    QVector<double> values;
    QVector<double> timeaxis;
};



class PlotEntry{
public:
    PlotEntry(QString filePath);
    ~PlotEntry();
    PlotCurveEntrySearchResult getPlotCurveByFieldID(QString FieldID);
    void addPlotCurve(QString FieldID, QString humanReadableName);
    void removeCurve(QString FieldID);
    void dumpPlotPointToFile(QDateTime timeStamp, int64_t value, int colIndex);
    void resetCurves();
    void openDumpFile();
    void resetPlotZoom();
    void setFilePath(QString path);

    QPair<int,int>plotIndex;
    QwtPlot* plot;
    QList<PlotCurveEntry*> plotCurveEntries;
    QDateTime dumpFileStartTime;

private:
    QList<QColor> colorList;
    QString filePath;
    QFile dumpFile;
    int fileIndex;
    int fileLines;
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
    void setDumpFilePath(QString path);

private slots:
    void on_action_clear_triggered();
    void on_action_zoom_reset_triggered();

private:
    PlotCurveEntrySearchResult getPlotCurveByFieldID(QString FieldID);
    void setupWheelZooming(QwtPlot *plot);

    QString dumpFilePath;
    Ui::PlotWindow *ui;
    QList<PlotEntry*> plots;
};

#endif // PLOTWINDOW_H
