#include "plotwindow.h"
#include "ui_plotwindow.h"
#include <qwt/qwt_plot.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_draw.h>
#include <qwt/qwt_plot_magnifier.h>
#include <qwt/qwt_plot_zoomer.h>
#include <qwt/qwt_plot_panner.h>

class TimeScaleDraw:public QwtScaleDraw
{
public:
    TimeScaleDraw(const QTime & base)
        :baseTime(base)
    {
    }
    virtual QwtText label(double v)const
    {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch((int64_t)v);
        return dt.toString("MM.dd HH:mm:ss.zzz");
    }
private:
    QTime baseTime;
};

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

#if 1
PlotCurveEntrySearchResult PlotWindow::getPlotCurveByFieldID(QString FieldID)
{
    PlotCurveEntrySearchResult result;
    for (PlotEntry * p : plots){
        PlotCurveEntrySearchResult pce = p->getPlotCurveByFieldID(FieldID);
        if (pce.pce){
            result = pce;
            result.pe = p;
            break;
        }
    }
    return result;
}
#endif
PlotEntry *PlotWindow::getPlotByFieldID(QString FieldID)
{
    for (auto pc : plots){
        PlotCurveEntrySearchResult pcesr = pc->getPlotCurveByFieldID(FieldID);
        if (pcesr.pce){

            return pc;
        }
    }
    return NULL;
}

PlotEntry *PlotWindow::getPlotByPlotIndex(QPair<int, int> plotIndex)
{
    for (auto pe : plots){
        if (pe->plotIndex == plotIndex){
            return pe;
        }
    }
    return NULL;
}



void PlotWindow::addPlotPoint(QString FieldID, QString humanReadableName, QPair<int, int> plotIndex, QDateTime timeStamp, int64_t value)
{

    PlotCurveEntrySearchResult pcesr = getPlotCurveByFieldID(FieldID);

    if (pcesr.pce){
        pcesr.pce->addPlotPoint(timeStamp,value);
        pcesr.pe->dumpPlotPointToFile(timeStamp,value,pcesr.pcIndex);
    }else{
        PlotEntry* plotEntry = getPlotByPlotIndex(plotIndex);
        if (!plotEntry){
            plotEntry = addPlot(plotIndex);
        }
        plotEntry->addPlotCurve(FieldID,humanReadableName);
    }

}

void PlotWindow::setupWheelZooming(QwtPlot * plot)
{
    QwtPlotPanner* pPanner = new QwtPlotPanner(plot->canvas());
    (void)pPanner;
#if 1
     QwtPlotMagnifier *zoom_x = new QwtPlotMagnifier( plot->canvas() );
    (void)zoom_x;
    QwtPlotMagnifier *zoom_y = new QwtPlotMagnifier( plot->canvas() );
    (void)zoom_y;
#endif

    //zoom_x->setWheelModifiers(Qt::ShiftModifier);
   // zoom_x->setAxisEnabled(Qt::XAxis, true);
   // zoom_x->setAxisEnabled(Qt::YAxis,false);
    //zoom_y->setWheelModifiers(Qt::ControlModifier);
   // zoom_y->setAxisEnabled(Qt::XAxis,false);
   // zoom_y->setAxisEnabled(Qt::YAxis,true);

    //QwtPlotZoomer* pZoomer = new QwtPlotZoomer(plot->canvas());

   // pZoomer->setKeyPattern( QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier );
   // pZoomer->setKeyPattern( QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier );
  //  pZoomer->setKeyPattern( QwtEventPattern::KeyHome, Qt::Key_Home );
    //(void)pZoomer;
}

PlotEntry *PlotWindow::addPlot(QPair<int, int> plotIndex)
{
    QwtLegend *legend = new QwtLegend;


    QwtPlot* plot = new QwtPlot();

    plot->setCanvasBackground(QColor(Qt::white));
    ui->plotLayout->addWidget(plot,plotIndex.second,plotIndex.first);

    PlotEntry* result = new PlotEntry();

    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    plot->insertLegend(legend, QwtPlot::BottomLegend);

    result->plot = plot;
    plots.append(result);
    result->plotIndex = plotIndex;
    setupWheelZooming(plot);
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
    values.push_back(value);
    timeaxis.push_back(timeStamp.currentMSecsSinceEpoch());
    plotCurve->setSamples(timeaxis,values);
    plotCurve->plot()->replot();
}


PlotEntry::PlotEntry()
{
    colorList.append(Qt::red);
    colorList.append(Qt::blue);
    colorList.append(Qt::black);
    colorList.append(Qt::green);
    colorList.append(Qt::magenta);
    colorList.append(Qt::darkYellow);
    colorList.append(Qt::darkCyan);

    int fileIndex=0;
    dumpFile.close();
    QString myfileName = "test"+QString("_%1").arg(fileIndex, 4, 10, QChar('0'))+".csv";
    dumpFile.setFileName(myfileName);
    if (!dumpFile.open(QIODevice::WriteOnly | QIODevice::Text)){
      #if 0
        QMessageBox::warning(this, "ProtocolAnalyzer",
                                      "Cant open file \""+myfileName+"\" for dumping data.",
                                      QMessageBox::Ok ,
                                      QMessageBox::Ok);
#endif
    }

}

PlotEntry::~PlotEntry()
{

}

PlotCurveEntrySearchResult PlotEntry::getPlotCurveByFieldID(QString FieldID)
{
    PlotCurveEntrySearchResult result;
    int i=0;
    for (auto pce : plotCurveEntries){
        if (pce->FieldID == FieldID){
            result.pce = pce;
            result.pcIndex = i;
            break;
        }
        i++;
    }
    return result;
}




void PlotEntry::addPlotCurve(QString FieldID, QString humanReadableName )
{

    QwtPlotCurve* pc = new QwtPlotCurve();

    pc->setPen(QPen(colorList[plotCurveEntries.count()]));
    pc->setTitle(humanReadableName);
    pc->attach(plot);
    plot->setAxisScaleDraw( QwtPlot::xBottom,new TimeScaleDraw(QTime::currentTime()));
    plot->setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    plot->setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );

    PlotCurveEntry *pce = new PlotCurveEntry();
    pce->plotCurve = pc;
    pce->FieldID = FieldID;
    pce->humanReadableName = humanReadableName;

    plotCurveEntries.append(pce);
}

void PlotEntry::dumpPlotPointToFile(QDateTime timeStamp, int64_t value, int colIndex)
{
    if (dumpFile.isWritable()){
        QString row = timeStamp.toString("MM.dd HH:mm:ss.zzz");
        for(int i=1; i<plotCurveEntries.count();i++){
            if (i == colIndex){
                row += ";\""+QString::number(value)+"\"";
            }else{
                row += ";\"\"";
            }
        }
        row+="\n";

        dumpFile.write(row.toLocal8Bit(),row.length());
#if 0
        fileRows++;
        if (fileRows > MAXFILEROWS){
            beginNewDumpFile();
        }
#endif
    }



}

