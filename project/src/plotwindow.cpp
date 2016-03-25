#include "plotwindow.h"
#include "ui_plotwindow.h"
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

PlotWindow::PlotWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlotWindow)
{
    ui->setupUi(this);
}

PlotWindow::~PlotWindow()
{
    delete ui;
}

PlotCurveEntry *PlotWindow::getPlotCurveByFieldID(QString FieldID)
{
    PlotCurveEntry* result=NULL;
    PlotEntry* pe = getPlotByFieldID(FieldID);
    if (pe){
        for (auto pce: pe->plotCurveEntries){
            if (pce-> FieldID == FieldID){
                return pce;
            }
        }
    }
    return result;
}

PlotEntry *PlotWindow::getPlotByFieldID(QString FieldID)
{
    for (auto pc : plots){
        PlotCurveEntry* pce = pc->getPlotCurveByFieldID(FieldID);
        if (pce){
            return pc;
        }
    }
    return NULL;
}

void PlotWindow::addPlotPoint(QString FieldID, QString humanReadableName, QPair<int, int> plotIndex, QDateTime timeStamp, int64_t value)
{
    PlotCurveEntry* plotCurveEntry = getPlotCurveByFieldID(FieldID);
    if (plotCurveEntry){
        plotCurveEntry->addPlotPoint(timeStamp,value);
    }else{
        PlotEntry* plotEntry = getPlotByPlotIndex(plotIndex);
        if (!plotEntry){
            plotEntry = addPlot(plotIndex);
        }
        plotEntry->addPlotCurve(FieldID,humanReadableName);
    }
}

PlotEntry *PlotWindow::addPlot(QPair<int, int> plotIndex)
{
    QwtPlot* plot = new QwtPlot();
    ui->plotLayout->addWidget(plot,plotIndex.second,plotIndex.first);

    PlotEntry* result = new PlotEntry();

    result->plot = plot;
    plots.append(result);
    result->plotIndex = plotIndex;
    return result;
}


PlotCurveEntry::PlotCurveEntry()
{

}

PlotCurveEntry::~PlotCurveEntry()
{

}

void PlotCurveEntry::addPlotPoint(QDateTime timeStamp, int64_t value)
{
    (void)timeStamp;
    (void)value;

}


PlotEntry::PlotEntry()
{

}

PlotEntry::~PlotEntry()
{

}


void PlotEntry::addPlotCurve(QString FieldID, QString humanReadableName )
{
    QwtPlotCurve* pc = new QwtPlotCurve();
    pc->attach(plot);

    PlotCurveEntry *pce = new PlotCurveEntry();
    pce->plotCurve = pc;
    pce->FieldID = FieldID;
    pce->humanReadableName = humanReadableName;

    plotCurveEntries.append(pce);
}
