#ifndef BOATINTERFACE_H
#define BOATINTERFACE_H
#include <QGraphicsWidget>
#include "PolarInterface.h"
#include "vlmPoint.h"
class BoatInterface: public QGraphicsWidget
{ Q_OBJECT
    public:
        ~BoatInterface(){}
    virtual bool get_useSkin() const =0;
    virtual QString get_boardSkin() const =0;
    virtual PolarInterface * getPolarDataInterface(void)=0;
    virtual double getWindDir(void)=0;
    virtual double getWindSpeed(void)=0;
    virtual double getHeading(void)=0;
    virtual double getWPdir(void)=0;
    virtual vlmPoint getClosest() {return vlmPoint(0,0);}
    virtual double getSpeed(void)=0;
    virtual void drawEstime(double myHeading, double mySpeed)=0;
    virtual bool getLockStatus(void)=0;
    virtual void set_pilotHeading(double heading){Q_UNUSED(heading);}
    virtual void set_pilotAngle(double angle){Q_UNUSED(angle);}
    virtual void set_pilotOrtho(void){}
    virtual void set_pilotVmg(void){}
    virtual void set_pilotVbvmg(void){}
    virtual int get_boatType() const =0;
    virtual QPointF getPosition(void)=0;
    virtual double getDnm(void)=0;
    virtual double getOrtho(void)=0;
    virtual double getVmg(void)=0;
    virtual double getWPangle(void)=0;
    virtual double getLoch(void)=0;
    virtual double getAvg(void)=0;
    virtual QString getBoatName(void){return QString();}
    virtual QString getScore(void)=0;
    virtual int getRank(void)=0;
    virtual QString getRaceName(void){return QString();}
    virtual QList<vlmLine*> getGates()=0;
    virtual int getNWP(void)=0;
    virtual int getPilotType(void){return 0;}
    virtual QString getPilotString(void){return QString();}
    virtual double getTWA(void)=0;
    virtual QStringList getPilototo(void){return QStringList();}
    virtual double getWPLat(void)=0;
    virtual double getWPLon(void)=0;
    virtual double getWPHd(void)=0;
    virtual void setWP(QPointF point,double w)=0;
    virtual time_t getPrevVac()=0;
};

#endif // BOATINTERFACE_H
