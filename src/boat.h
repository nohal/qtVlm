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
        boat(QString name, bool activated,
            Projection * proj,MainWindow * main,myCentralWidget * parent);
        ~boat();

        virtual void setStatus(bool activated);
        void setParam(QString name);
        void setParam(QString name, bool activated);
        void setLockStatus(bool status);
        void setZoom(float zoom)   { this->zoom=zoom; }
        void setForceEstime(bool force_estime) { this->forceEstime=force_estime;}
        void unSelectBoat(bool needUpdate);
        virtual int getVacLen(void) {return 1; }
        virtual int getId(void) {return -1; }

        QString getBoatName(void)       {    return name; }
        bool getStatus(void)            {    return activated; }
        double getLat(void)             {    return lat; }
        double getLon(void)             {    return lon; }
        float getSpeed(void)            {    return speed; }
        float getHeading(void)          {    return heading; }
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

        float getBvmgUp(float ws);
        float getBvmgDown(float ws);

        int getType(void) { return boat_type; }

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;

    public slots:
        void slot_projectionUpdated();
        void slot_paramChanged();
        void slot_selectBoat();
        void slot_toggleEstime();
        void slot_updateGraphicsParameters();
        void slot_shLab(){this->labelHidden=!this->labelHidden;update();}        
        void slot_shSall(){ my_shSall();}
        void slot_shHall(){ my_shHall();}
        void slotTwaLine(){parent->twaDraw(lon,lat);}
        void slotCompassLine(void);

    signals:
        void boatSelected(boat*);
        void boatUpdated(boat*,bool,bool);
        void boatLockStatusChanged(boat*,bool);
        void getPolar(QString fname,Polar ** ptr);
        void releasePolar(QString fname);
        void clearSelection(void);
        void compassLine(int,int);

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

        /* DATA */
        int boat_type;

        QString polarName;
        Polar * polarData;

        QString name;
        bool activated;
        bool changeLocked;
        bool selected;
        double lat,lon;
        double WPLat,WPLon;
        float speed,heading;
        float zoom;

        Projection * proj;

        QLabel    *label;
        QColor    bgcolor,fgcolor;
        QColor    myColor;
        QColor    selColor;
        QCursor   enterCursor;
        int       width,height;
        QString   my_str;

        /* trace */
        vlmLine * trace_drawing;
        QList<vlmPoint> trace;
        void updateTraceColor(void);

        /* estime param */
        int estime_type;
        int estime_param;
        orthoSegment * estimeLine;
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

        virtual void my_selectBoat(void)   { }
        virtual void my_unselectBoat(void) { }
        virtual void my_shSall(void)       { }
        virtual void my_shHall(void)       { }

        void updateBoatData(void);
        virtual void reloadPolar(void);
        virtual void updateBoatName(void)  { }
        virtual void updateHint(void)      { }

};

#endif // BOAT_H
