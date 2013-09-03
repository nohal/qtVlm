/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef BOAT_H
#define BOAT_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QMenuBar>
#include <QLabel>

#include "mycentralwidget.h"

#include "class_list.h"

class boat: public QGraphicsWidget
{ Q_OBJECT
    public:
        boat(QString pseudo, bool activated,
            Projection * proj,MainWindow * main,myCentralWidget * parent);
        ~boat();

        virtual void setStatus(bool activated);
        virtual void showNextGates(){return;}
        void setParam(QString pseudo);
        void setParam(QString pseudo, bool activated);
        void setLockStatus(bool status);
        void setZoom(double zoom)   { this->zoom=zoom; }
        void setForceEstime(bool force_estime) { this->forceEstime=force_estime;}
        virtual void unSelectBoat(bool needUpdate);
        virtual int getId(void) {return -1; }
        QString getplayerName(void)     {    return playerName; }
        virtual void stopRead(){return;}
        virtual time_t getPrevVac(){return QDateTime::currentDateTimeUtc().toTime_t();}

        virtual void reloadPolar(bool forced=false);
        virtual QList<vlmLine*> getGates();

        void playerDeActivated(void);
        void playerActivated(void) { setStatus(activated); }

        bool getStatus(void)            {    return activated; }
        int getVacLen(void)             {    return vacLen; }
        double getLat(void)             {    return lat; }
        double getLon(void)             {    return lon; }
        QPointF getPosition(void)       {    return QPointF(lon,lat); }
        double getSpeed(void)           {    return speed; }
        double getHeading(void)         {    return heading; }
        double getAvg(void)             {    return avg; }
        double getDnm(void)             {    return dnm; }
        double getLoch(void)            {    return loch; }
        double getOrtho(void)           {    return ortho; }
        double getLoxo(void)            {    return loxo; }
        double getVmg(void)             {    return vmg; }
        double getWindDir(void)         {    return windDir; }
        double getWindSpeed(void)       {    return windSpeed; }
        double getTWA(void)             {    return TWA; }
        double getWPHd(void)            {    return WPHd; }
        double getWPdir(void);
        int getNWP(void)                {    return nWP; }
        QString getPolarName(void)      {    return polarName; }
        Polar * getPolarData(void)      {    return polarData; }
        bool getLockStatus(void)        {    return changeLocked;}
        bool getForceEstime(void)       {    return forceEstime; }
        int getEstimeType(void)         {    return estime_type; }
        bool getIsSelected(void)        {    return selected; }
        double getZoom(void)            {    return zoom; }
        bool isUpdating()               {    return false; }
        double getWPLat(void)           {    return WP.y(); }
        double getWPLon(void)           {    return WP.x(); }
        QPointF getWP(void)             {    return WP; }
        int getRank(void)               {    return rank; }
        QString getScore(void)          {    return score;}
        QString getBoatPseudo(void)     {    return pseudo; }
        QString getOwn(void)            {    return own; }

        virtual void setWP(QPointF point,double w);

        double getWPangle(void) { return 0; }

        double getBvmgUp(double ws);
        double getBvmgDown(double ws);
        int getX(){return x();}
        int getY(){return y();}

        FCT_SETGET_CST(int,boatType)

        void drawEstime(double myHeading, double mySpeed);
        double getDeclinaison(){return this->declinaison;}
        void setDeclinaison(double d){this->declinaison=d;}

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;
        vlmLine * getTraceDrawing(){return this->trace_drawing;}
        double getMinSpeedForEngine(){return this->minSpeedForEngine;}
        void setMinSpeedForEngine(double d){this->minSpeedForEngine=d;}
        double getSpeedWithEngine(){return this->speedWithEngine;}
        void setSpeedWithEngine(double d){this->speedWithEngine=d;}

        void drawOnMagnifier(Projection *mProj, QPainter *pnt);

        /*** Barrier ***/
        QList<BarrierSet *>* get_barrierSets(void) { return &barrierSets; }
        QList<QString> * get_barrierKeys(void) {return &barrierKeys; }
        void add_barrierSet(BarrierSet* set);
        void rm_barrierSet(BarrierSet* set);
        void update_barrierKey(BarrierSet* set);
        void clear_barrierSet(void);
        void clear_barrierKeys(void) { barrierKeys.clear(); }
        void updateBarrierKeys(void);
        void setSetKeys(QList<QString> keys) { barrierKeys = keys; }
        void cleanBarrierList(void);
        bool cross(QLineF line);

public slots:
        void slot_projectionUpdated();
        void slot_paramChanged();
        virtual void slot_selectBoat();
        void slot_toggleEstime();
        void slot_updateGraphicsParameters();
        void slot_shLab(bool state){this->labelHidden=state;update();}
        virtual void slot_shSall() { }
        virtual void slot_shHall() { }
        void slotTwaLine(){parent->twaDraw(lon,lat);}
        void slotCompassLine(void);
        void slot_estimeFlashing(void);
        void polarLoaded(QString,Polar *);

        /*** Barrier ***/
        void slot_chooseBarrierSet(void);

        void slot_centerOnBoat();

    signals:
        void boatSelected(boat*);
        void boatUpdated(boat*,bool,bool);
        void boatLockStatusChanged(boat*,bool);
        void getPolar(QString);
        void releasePolar(QString fname);
        void clearSelection(void);
        void compassLine(double,double);
        void getTrace(QByteArray,QList<vlmPoint> *);
        void showMessage(QString,int);

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

        /* DATA */
        int boatType;

        QString polarName;
        Polar * polarData;

        QString pseudo;
        bool activated;
        bool changeLocked;
        bool selected;
        double lat,lon;
        QPointF WP;
        double speed,heading;


        double avg;
        double dnm,loch,ortho,loxo,vmg;
        double windDir,windSpeed;
        double TWA;

        double WPHd;
        QString ETA;
        QString score;
        QString stopAndGo;
        time_t prevVac;
        time_t nextVac;
        int nWP;
        int   vacLen;
        int rank;

        /*** Barrier ***/
        QList<BarrierSet *> barrierSets;
        QList<QString> barrierKeys;

        double zoom;

        Projection * proj;

        QLabel    *label;
        QColor    bgcolor,fgcolor;
        QColor    myColor;
        QColor    selColor;
        QCursor   enterCursor;
        int       width,height;
        QString   my_str;
        QString   playerName;

        /* trace */
        vlmLine * trace_drawing;
        void updateTraceColor(void);

        /* estime param */
        int estime_type;
        int estime_param;
        vlmLine * estimeLine;
        orthoSegment * WPLine;

        myCentralWidget * parent;
        MainWindow * mainWindow;

        void updatePosition(void);

        bool forceEstime;

        void drawEstime(void);
        bool labelHidden;

        /* MENU */
        QMenu *popup;
        QAction * ac_select;
        QAction * ac_estime;
        QAction * ac_compassLine;
        QAction * ac_twaLine;
        QAction * ac_chooseBarrierSet;
        QAction * ac_centerOnboat;
        void createPopUpMenu();        

        void updateBoatData(void);        
        virtual void updateBoatString(void)  { }
        virtual void updateHint(void)      { }
        virtual void myCreatePopUpMenu(void)   {  }
        QString country;
        QImage flag;
        bool drawFlag;
        QString own;
        QTimer * estimeTimer;

        polarList * polar_list;
        bool my_intersects(QLineF line1,QLineF line2) const;
        double windEstimeDir;
        double windEstimeSpeed;
        double declinaison;
        double minSpeedForEngine;
        double speedWithEngine;
};
Q_DECLARE_TYPEINFO(boat,Q_MOVABLE_TYPE);
#endif // BOAT_H
