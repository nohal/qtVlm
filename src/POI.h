/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#ifndef POI_H
#define POI_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QMenu>

#include "class_list.h"

#define POI_TYPE_POI    0
#define POI_TYPE_WP     1
#define POI_TYPE_BALISE 2

//===================================================================
class POI : public QGraphicsWidget
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        POI(QString name, int type, float lat, float lon,
                    Projection *proj, QWidget *ownerMeteotable, myCentralWidget *parentWindow,
                    float wph, int tstamp,bool useTstamp, boatAccount *boat);

        ~POI();

        /* accés aux données */
        QString  getName(void)         {return name;}
        ROUTE    *getRoute(void)        {return route;}
        float    getLongitude(void)    {return lon;}
        float    getLatitude(void)     {return lat;}
        float    getWph(void)          {return wph;}
        int      getTimeStamp(void)    {if(useRouteTstamp && !useTstamp) return routeTimeStamp; else return timeStamp;}
        bool     getUseTimeStamp(void) {if(timeStamp==-1) return false; else return useTstamp;}
        int      getType(void)         {return type; }
        int      getTypeMask(void)     {return typeMask; }
        bool     getIsWp(void)         {return isWp;}
        QString  getRouteName(void);

        static QString  getTypeStr(int index);
        QString  getTypeStr(void)      {return getTypeStr(type); }

        /* modification des données */
        void setName           (QString name);
        void setLongitude      (float lon);
        void setLatitude       (float lat);
        void setWph            (float wph) {this->wph=wph;}
        void setTimeStamp      (int tstamp);
        void setRouteTimeStamp (int date);
        void setUseTimeStamp   (bool state){this->useTstamp=state;}
        void setType           (int type) {this->type=type;this->typeMask=(1<<type);}
        void setTip            (QString tip);
        void setRoute          (ROUTE *route);
        void setRouteName      (QString routeName){this->routeName=routeName;}


        /* comparateur de classe pour le tri */
        static bool myLessThan(POI * POI_1,POI* POI_2) {return POI_1->name < POI_2->name;}

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;

        /* event propagé par la scene */
        bool tryMoving(int x, int y);

    public slots:
        void slot_updateProjection();
        void slot_editPOI();
        void slot_setWP();
        void slot_setGribDate();
        void slot_delPoi();
        void slot_copy();
        void slot_paramChanged();
        void slot_WPChanged(float,float);
        void slot_updateTip(boatAccount *);
        void slot_shShow(){this->labelHidden=false;show();}
        void slot_shHidden(){hide();}
        void slot_shPoi(){this->isVisible()?hide():show();}
        void slot_shLab(){this->labelHidden=!this->labelHidden;update();}
        void slot_routeMenu(QAction* ptr_action);

    signals:
        void chgWP(float,float,float);
        void addPOI_list(POI*);
        void delPOI_list(POI*);
        void editPOI(POI*);
        void selectPOI(POI*);
        void setGribDate(int);
        void clearSelection(void);
        void updateTip(boatAccount*);
        void poiMoving();

    protected:
        void  mousePressEvent(QGraphicsSceneMouseEvent * e);
        void  mouseReleaseEvent(QGraphicsSceneMouseEvent * e);
        void  contextMenuEvent(QGraphicsSceneContextMenuEvent * event);

        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        /* parent, main */
        myCentralWidget   *parent;
        QWidget   *owner;
        Projection   *proj;

        /* widget component */
        QColor    bgcolor,fgcolor;
        QColor    poiColor,mwpColor,wpColor,baliseColor;
        int       width,height;
        QString   my_str;

        /* data */
        QString  name;
        double   lon, lat;
        float    wph;
        double   WPlon,WPlat;
        int      pi, pj;
        int      timeStamp;
        int      routeTimeStamp;
        bool     useTstamp;
        bool     useRouteTstamp;
        bool     isWp;
        int      type;
        int      typeMask;
        bool     isMoving;
        int      mouse_x,mouse_y;
        boatAccount *boat;

        void update_myStr();

        QMenu *popup;
        QAction * ac_edit;
        QAction * ac_setWp;
        QAction * ac_setGribDate;
        QAction * ac_delPoi;
        QAction * ac_copy;
        QAction * ac_compassLine;
        QMenu * ac_routeList;
        void createPopUpMenu(void);

        void chkIsWP(void);
        void rmSignal(void);
        ROUTE *route;
        QString routeName;
        bool labelHidden;
        bool VLMBoardIsBusy;
};

#endif
