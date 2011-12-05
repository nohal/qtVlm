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
#include "mycentralwidget.h"

#define POI_TYPE_POI    0
#define POI_TYPE_WP     1
#define POI_TYPE_BALISE 2

//===================================================================
class POI : public QGraphicsWidget
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        POI(QString name, int type, double lat, double lon,
                    Projection *proj, MainWindow *ownerMeteotable, myCentralWidget *parentWindow,
                    float wph, int tstamp,bool useTstamp, boat *boat);

        ~POI();

        /* accés aux données */
        QString  getName(void)         {return name;}
        ROUTE    *getRoute(void)        {return route;}
        double    getLongitude(void)    {return lon;}
        double    getLatitude(void)     {return lat;}
        float    getWph(void)          {return wph;}
        int      getTimeStamp(void)    {if(useRouteTstamp && !useTstamp) return routeTimeStamp; else return timeStamp;}
        bool     getUseTimeStamp(void) {if(timeStamp==-1) return false; else return useTstamp;}
        bool     getUseRouteTstamp(void) {if(routeTimeStamp==-1) return false; else return useRouteTstamp;}
        time_t   getRouteTimeStamp(void){return this->routeTimeStamp;}
        int      getType(void)         {return type; }
        int      getTypeMask(void)     {return typeMask; }
        bool     getIsWp(void)         {return isWp;}
        QString  getRouteName(void);
        double   getSearchRangeLon(void) {return this->searchRangeLon;}
        double   getSearchRangeLat(void) {return this->searchRangeLat;}
        double   getSearchStep(void) {return this->searchStep;}
        bool    getOptimizing(void) {return this->optimizing;}
        bool    getWasWP(void) {return this->wasWP;}
        void    setWasWP(bool b){this->wasWP=b;}
        static QString  getTypeStr(int index);
        QString  getTypeStr(void)      {return getTypeStr(type); }
        int     getNavMode(){return this->navMode;}

        /* modification des données */
        void setName           (QString name);
        void setLongitude      (double lon);
        void setLatitude       (double lat);
        void setWph            (float wph);
        void setTimeStamp      (time_t tstamp);
        void setRouteTimeStamp (time_t date);
        void setUseTimeStamp   (bool state){this->useTstamp=state;}
        void setType           (int type) {this->type=type;this->typeMask=(1<<type);}
        void setTip            (QString tip);
        void setRoute          (ROUTE *route);
        void setRouteName      (QString routeName){this->routeName=routeName;}
        void setSearchRangeLon (double value){this->searchRangeLon=value;}
        void setSearchRangeLat (double value){this->searchRangeLat=value;}
        void setSearchStep     (double value){this->searchStep=value;}
        void setNavMode        (int mode);
        void setOptimizing     (bool b) {this->optimizing=b;}
        void setMyLabelHidden  (bool b) {if(route==NULL) this->myLabelHidden=false; else this->myLabelHidden=b;}
        bool getMyLabelHidden  (void) {return this->myLabelHidden;}
        void setNotSimplificable(bool b) {this->notSimplificable=b;this->ac_simplifiable->setChecked(b);}
        bool getNotSimplificable(){return this->notSimplificable;}
        QColor getLineColor(){return lineColor;}
        void setLineColor(QColor c){lineColor=c;}
        float getLineWidth(){return lineWidth;}
        void setLineWidth(float f){lineWidth=f;}
        POI * getConnectedPoi(){return connectedPoi;}
        void setConnectedPoi(POI * p){connectedPoi=p;}
        void setLineBetweenPois(vlmLine * line){this->lineBetweenPois=line;}
        bool getPiloteSelected(){return piloteSelected;}
        void setPiloteSelected(bool b){this->piloteSelected=b;this->ac_pilot->setChecked(b);}
        /* comparateur de classe pour le tri */
        static bool myLessThan(POI * POI_1,POI* POI_2) {return POI_1->name < POI_2->name;}

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;

        /* event propagé par la scene */
        bool tryMoving(int x, int y);
        time_t getPiloteDate(){return piloteDate;}
        void setPiloteDate(time_t t){this->piloteDate=t;}
        void setLabelTransp(bool b){this->labelTransp=b;}
        void chkIsWP(void);
        bool isPartOfTwa(){return partOfTwa;}
        void setPartOfTwa(bool b){this->partOfTwa=b;}

    public slots:
        void slot_updateProjection();
        void slot_editPOI();
        void slot_setWP();
        void slot_setGribDate();
        void slot_delPoi();
        void slot_copy();
        void slot_paramChanged();
        void slot_WPChanged(double tlat,double tlon);
        void slot_updateTip(boat *);
        void slot_shShow(){show();}
        void slot_shHidden(){hide();}
        void slot_shPoi(){this->isVisible()?hide():show();}
        void slot_shLab(bool state){this->labelHidden=state;update();}
        void slot_routeMenu(QAction* ptr_action);
        void slot_finePosit(bool silent=false);
        void slot_abort(){this->abortSearch=true;}
        void slot_setMode(QAction* ptr_action);
        void slot_setHorn();
        void slot_twaLine(){parent->twaDraw(lon,lat);}
        void slotCompassLine();
        void slot_editRoute();
        void slot_relier();
        void slot_pilote();
        void slot_notSimplificable(bool b){this->notSimplificable=b;}

    signals:
        void chgWP(float,float,float);
        void addPOI_list(POI*);
        void delPOI_list(POI*);
        void editPOI(POI*);
        void selectPOI(POI*);
        void setGribDate(time_t);
        void clearSelection(void);
        void updateTip(boat*);
        void poiMoving();
        void compassLine(double,double);
        void wpChanged();

    protected:
        void  mousePressEvent(QGraphicsSceneMouseEvent * e);
        void  mouseReleaseEvent(QGraphicsSceneMouseEvent * e);
        void  contextMenuEvent(QGraphicsSceneContextMenuEvent * event);

        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        /* parent, main */
        myCentralWidget   *parent;
        MainWindow   *owner;
        Projection   *proj;

        /* widget component */
        QColor    bgcolor,fgcolor;
        QColor    poiColor,mwpColor,wpColor,baliseColor;
        int       width,height;
        QString   my_str;

        /* data */
        QString  name;
        double   lon, lat;
        double   previousLon,previousLat;
        float    wph;
        double   WPlon,WPlat;
        int      pi, pj;
        time_t      timeStamp;
        time_t      routeTimeStamp;
        bool     useTstamp;
        bool     useRouteTstamp;
        bool     isWp;
        int      type;
        int      typeMask;
        bool     isMoving;
        int      mouse_x,mouse_y;
        boat *   myBoat;

        void update_myStr();

        QMenu *popup;
        QAction * ac_edit;
        QAction * ac_setWp;
        QAction * ac_setGribDate;
        QAction * ac_delPoi;
        QAction * ac_delRoute;
        QAction * ac_editRoute;
        QAction * ac_copy;
        QAction * ac_compassLine;
        QAction * ac_twaLine;
        QAction * ac_setHorn;
        QMenu * ac_routeList;
        QAction * ac_finePosit;
        QMenu * ac_modeList;
        QAction * ac_modeList1;
        QAction * ac_modeList2;
        QAction * ac_modeList3;
        QAction * ac_connect;
        QAction * ac_pilot;
        QAction * ac_simplifiable;
        void createPopUpMenu(void);

        void rmSignal(void);
        ROUTE *route;
        QString routeName;
        bool labelHidden;
        bool VLMBoardIsBusy;
        double searchRangeLon;
        double searchRangeLat;
        double searchStep;
        bool abortSearch;
        int  navMode;
        bool wasWP;
        bool optimizing;
        bool myLabelHidden;
        bool partOfTwa;
        bool notSimplificable;
        POI * connectedPoi;
        vlmLine * lineBetweenPois;
        QColor lineColor;
        float lineWidth;
        int colorPilototo;
        bool piloteSelected;
        time_t piloteDate;
        bool labelTransp;
};

#endif
