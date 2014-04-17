#ifndef MAINWINDOWINTERFACE_H
#define MAINWINDOWINTERFACE_H
#include <QMainWindow>
#include "BoatInterface.h"
class MainWindowInterface: public QMainWindow
{ Q_OBJECT
    public:
        ~MainWindowInterface(){}
        virtual bool get_selPOI_instruction()=0;
        virtual BoatInterface * get_selectedBoatInterface()=0;
        virtual QList<POI *> * getPois()=0;
        virtual QColor getWindColorStatic(const double &v, const bool &smooth=true)=0;
        virtual QVariant getSettingApp(const int &key) const =0;
        virtual QString get_folder(QString str) const =0;
        virtual QPalette getOriginalPalette() const =0;
        virtual void setting_saveGeometry(QWidget * obj) =0;
        virtual bool getWPClipboard(QString *,double * lat,double * lon, double * wph) =0;
        virtual void setWPClipboard(double lat,double lon, double wph) =0;
        virtual QString pos2String(const int &type,const double &value) =0;
        virtual QString formatLongitude(const double &x) =0;
        virtual QString formatLatitude(const double &y) =0;
        virtual void setFontDialog(QObject * o) =0;
        virtual int get_nxtVac_cnt()=0;
       /* Note: the `boardPlugin` parameter may be any object that
        * defines the `void slot_selectWP_POI()` slot. The function
        * will still work if the given object does not define this
        * slot, but in that case it will not be possible to pick a POI
        * from the dialog. */
        virtual void manageWPDialog(BoatInterface * myBoat, QObject *boardPlugin)=0;

    public slots:
        virtual void slotVLM_Sync()=0;
        virtual void slot_clearPilototo()=0;
        virtual void slotPilototo(void)=0;
        virtual void slot_POIselected(POI* poi)=0;
        virtual void slotSelectWP_POI(void)=0;
};

#endif // MAINWINDOWINTERFACE_H
