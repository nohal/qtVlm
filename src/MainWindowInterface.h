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
    public slots:
        virtual void slotVLM_Sync()=0;
        virtual void slot_clearPilototo()=0;
        virtual void slot_POIselected(POI* poi)=0;
        virtual void slotSelectWP_POI(void)=0;
};

#endif // MAINWINDOWINTERFACE_H
