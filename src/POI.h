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


#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>

class POI;

#include "Projection.h"

//===================================================================
class POI : public QWidget
{ Q_OBJECT
    public:
        /* les enum */
        enum POI_TYPE { TYPE_POI=0, TYPE_WP, TYPE_BALISE};

        /* constructeurs, destructeurs */
        POI(QString name, POI_TYPE type, float lat, float lon,
                    Projection *proj, QWidget *ownerMeteotable,
                    QWidget *parentWindow, float wph,
                    int tstamp,bool useTstamp);

        ~POI();

        /* accés aux données */
        QString  getName(void)         {return name;}
        float    getLongitude(void)    {return lon;}
        float    getLatitude(void)     {return lat;}
        float    getWph(void)          {return wph;}
        int      getTimeStamp(void)    {return timeStamp;}
        bool     getUseTimeStamp(void) {if(timeStamp==-1) return false; else return useTstamp;}
        POI_TYPE getType(void)         {return type; }
        int      getTypeMask(void)     {return typeMask; }

        static QString  getTypeStr(int index);
        QString  getTypeStr(void)      {return getTypeStr(type); }

        /* modification des données */
        void setName         (QString name);
        void setLongitude    (float lon) {this->lon=lon;}
        void setLatitude     (float lat) {this->lat=lat;}
        void setWph          (float wph) {this->wph=wph;}
        void setTimeStamp    (int tstamp){this->timeStamp=tstamp;}
        void setUseTimeStamp (bool state){this->useTstamp=state;}
        void setType         (POI_TYPE type) {this->type=type;this->typeMask=(1<<type);}


        void doChgWP(float lat,float lon, float wph);



    public slots:
        void updateProjection();
        void timerClickEvent();
        void slot_editPOI();
        void slot_setWP();
        void slot_setGribDate();
        void slotDelPoi();
        void slot_copy();
        void paramChanged();
        void WPChanged(float,float);

    signals:
        void chgWP(float,float,float);
        void addPOI_list(POI*);
        void delPOI_list(POI*);
        void editPOI(POI*);
        void selectPOI(POI*);
        void setGribDate(int);
        void clearSelection(void);

    private:
        QString      name;
        float        lon, lat,wph;
        float        WPlon,WPlat;
        Projection   *proj;
        int       pi, pj;
        QCursor   enterCursor;
        QLabel    *label;
        QWidget   *parent;
        QWidget   *owner;
        QColor    bgcolor,fgcolor;
        QColor    poiColor,mwpColor,wpColor,baliseColor;
        int timeStamp;
        bool useTstamp;
        bool isWp;

        POI_TYPE type;
        int typeMask;

        void createWidget();

        void  paintEvent(QPaintEvent *event);
        void  enterEvent (QEvent * e);
        void  leaveEvent (QEvent * e);

        int   countClick;
        void  mousePressEvent(QMouseEvent * e);
        //void  mouseDoubleClickEvent(QMouseEvent * e);
        void  mouseReleaseEvent(QMouseEvent * e);
        void  contextMenuEvent(QContextMenuEvent * event);

        QMenu *popup;
        QAction * ac_edit;
        QAction * ac_setWp;
        QAction * ac_setGribDate;
        QAction * ac_delPoi;
        QAction * ac_meteo;
        QAction * ac_copy;
        void createPopUpMenu(void);

        void chkIsWP(void);
        void rmSignal(void);
};

#endif
