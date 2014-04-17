#ifndef DIALOGZYGRIB_H
#define DIALOGZYGRIB_H
#include "class_list.h"
#include <QDialog>
#include <QTime>
#include "dataDef.h"
#include "LoadGribFile.h"
#include "ui_DialogZygrib.h"
class DialogZygrib : public QDialog, public Ui::DialogZygrib_ui
{
    Q_OBJECT

public:
    explicit DialogZygrib(MainWindow *main);
    ~DialogZygrib();
    void setZone(double x0, double y0, double x1, double y1);
public slots:
    void slotBtOK();
    void slotBtCancel();
    void slotServerStatus();
    void slotGribDataReceived(QByteArray *content, QString file);
    void slotGribFileError(QString error);
    void slotGribMessage(QString msg);
    void slotGribStartLoadData();
    void slotParameterUpdated();
    void slotAltitudeAll();
    void slotUngrayButtons();
    void slotProgress(qint64,qint64);
    void slot_screenResize();
signals:
    void signalGribFileReceived(QString fileName);
    void clearSelection();

private:
    LoadGribFile    *loadgrib;
    bool     loadInProgress;
    QTime    timeLoad;
    MainWindow * main;

    double   xmin,ymin,xmax,ymax,resolution;
    int     interval,days;
    bool    rain, cloud, pressure, wind, temp, humid, isotherm0;
    bool	tempPot, tempMin, tempMax, snowCateg, frzRainCateg;
    bool 	CAPEsfc, CINsfc;

    void    updateParameters();
    QString fileName;
    void loadSettings();
};

#endif // DIALOGZYGRIB_H
