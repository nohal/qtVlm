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
        void setZoom(float zoom)   { this->zoom=zoom; }
        void setForceEstime(bool force_estime) { this->forceEstime=force_estime;}
        virtual void unSelectBoat(bool needUpdate);
        virtual int getId(void) {return -1; }
        QString getplayerName(void)     {    return playerName; }
        virtual void stopRead(){return;}

        virtual void reloadPolar(void);

        void playerDeActivated(void);
        void playerActivated(void) { setStatus(activated); }

        bool getStatus(void)            {    return activated; }
        int getVacLen(void)             {    return vacLen; }
        double getLat(void)             {    return lat; }
        double getLon(void)             {    return lon; }
        float getSpeed(void)            {    return speed; }
        float getHeading(void)          {    return heading; }
        float getAvg(void)              {    return avg; }
        float getDnm(void)              {    return dnm; }
        float getLoch(void)             {    return loch; }
        float getOrtho(void)            {    return ortho; }
        float getLoxo(void)             {    return loxo; }
        float getVmg(void)              {    return vmg; }
        float getWindDir(void)          {    return windDir; }
        float getWindSpeed(void)        {    return windSpeed; }
        float getTWA(void)              {    return TWA; }
        float getWPHd(void)             {    return WPHd; }
        QString getPolarName(void)      {    return polarName; }
        Polar * getPolarData(void)      {    return polarData; }
        bool getLockStatus(void)        {    return changeLocked;}
        bool getForceEstime(void)       {    return forceEstime; }
        int getEstimeType(void)         {    return estime_type; }
        bool getIsSelected(void)        {    return selected; }
        float getZoom(void)             {    return zoom; }
        bool isUpdating()               {    return false; }
        double getWPLat(void)           {    return WPLat; }
        double getWPLon(void)           {    return WPLon; }
        int getRank(void)               {    return rank; }
        QString getScore(void)          {    return score;}
        QString getBoatPseudo(void)     {    return pseudo; }
        QString getOwn(void)            {    return own; }

        float getBvmgUp(float ws);
        float getBvmgDown(float ws);
        int getX(){return x();}
        int getY(){return y();}

        int getType(void) { return boat_type; }

        void drawEstime(float myHeading, float mySpeed);

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;

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
    signals:
        void boatSelected(boat*);
        void boatUpdated(boat*,bool,bool);
        void boatLockStatusChanged(boat*,bool);
        void getPolar(QString fname,Polar ** ptr);
        void releasePolar(QString fname);
        void clearSelection(void);
        void compassLine(int,int);
        void getTrace(QByteArray,QList<vlmPoint> *);

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

        /* DATA */
        int boat_type;

        QString polarName;
        Polar * polarData;

        QString pseudo;
        bool activated;
        bool changeLocked;
        bool selected;
        double lat,lon;
        double WPLat,WPLon;
        float speed,heading;


        float avg;
        float dnm,loch,ortho,loxo,vmg;
        float windDir,windSpeed;
        float TWA;

        float WPHd;
        QString ETA;
        QString score;
        time_t prevVac;
        time_t nextVac;
        int nWP;
        int   vacLen;
        int rank;



        float zoom;

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
        QList<vlmPoint> trace;
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
};

#endif // BOAT_H
