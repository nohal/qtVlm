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
        POI(const QString &name, const int &type, const double &lat, const double &lon,
                    Projection *proj, MainWindow *ownerMeteotable, myCentralWidget *parentWindow,
                    const double &wph, const int &tstamp, const bool &useTstamp);

        ~POI();

        /* accés aux données */
        QString  getName(void)         {return name;}
        ROUTE    *getRoute(void)        {return route;}
        double    getLongitude(void)    {return lon;}
        double    getLatitude(void)     {return lat;}
        double    getWph(void)          {return wph;}
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
        static QString  getTypeStr(int index){    QString type_str[3] = { "POI", "Marque", "Balise" };
                                                  return type_str[index];}
        QString  getTypeStr(void)      {return getTypeStr(type); }
        int     getNavMode(){return this->navMode;}
        bool    getHas_eta(void)        {return useRouteTstamp;}
        double getLonConnected(){return lonConnected;}
        double getLatConnected(){return latConnected;}
        bool    getAutoRange(void)  { return autoRange; }

        /* modification des données */
        void setName           (QString name);
        void setLongitude      (double lon);
        void setLatitude       (double lat);
        void setWph            (double wph);
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
        void setMyLabelHidden  (bool b) {if(route==NULL) this->myLabelHidden=false; else this->myLabelHidden=b;prepareGeometryChange();update();}
        bool getMyLabelHidden  (void) {return this->myLabelHidden;}
        void setNotSimplificable(bool b) {this->notSimplificable=b;this->ac_simplifiable->setChecked(b);update();}
        bool getNotSimplificable(){return this->notSimplificable;}
        QColor getLineColor(){return lineColor;}
        void setLineColor(QColor c){lineColor=c;}
        double getLineWidth(){return lineWidth;}
        void setLineWidth(double f){lineWidth=f;}
        POI * getConnectedPoi(){return connectedPoi;}
        void setConnectedPoi(POI * p){connectedPoi=p;}
        void setPosConnected(double lon,double lat){lonConnected=lon;latConnected=lat;}
        void setLineBetweenPois(orthoSegment * line){this->lineBetweenPois=line;}
        bool getPiloteSelected(){return piloteSelected;}
        void setPiloteSelected(bool b){this->piloteSelected=b;this->ac_pilot->setChecked(b);}
        void setAutoRange (bool b) { autoRange = b; }
        /* comparateur de classe pour le tri */
        static bool byName(POI * POI_1,POI* POI_2) {return POI_1->name < POI_2->name;}
        static bool bySequence(POI * POI_1,POI* POI_2) {return POI_1->sequence < POI_2->sequence;}

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;

        /* event propagé par la scene */
        bool tryMoving(int x, int y);
        time_t getPiloteDate(){return piloteDate;}
        void setPiloteDate(const time_t &t){this->piloteDate=t;}
        double getPiloteWph() {return piloteWph;}
        void setPiloteWph(const double &d){this->piloteWph=d;}
        void setLabelTransp(bool b){this->labelTransp=b;}
        void chkIsWP(void);
        bool isPartOfTwa(){return partOfTwa;}
        void setPartOfTwa(bool b){this->partOfTwa=b;}
        void setSequence(int i){this->sequence=i;}
        int getSequence(){return this->sequence;}
        void manageLineBetweenPois();

        static void importZyGrib(myCentralWidget * centralWidget);
        static void importGeoData(myCentralWidget * centralWidget);
        static void read_POIData(myCentralWidget * centralWidget);
        static void write_POIData(QList<POI*> & poi_list,myCentralWidget * centralWidget);
        static void cleanFile(QString fname);
        FCT_GET_CST(bool,drawLineOrtho)
        FCT_SETGET_CST(int, myBoatId)
        void set_drawLineOrtho(const bool &b){this->drawLineOrtho=b;manageBoatCircle();}
        void setBoatCircle(const int &id);

public slots:
        void slot_updateProjection();
        void slot_editPOI();
        void slot_setWP();
        void slot_setWP_ask();
        void slot_setGribDate();
        void slot_delPoi();
        void slot_copy();
        void slot_paramChanged();
        void slot_WPChanged(double tlat,double tlon);
        void slot_updateTip();
        void slot_shPoi(bool isHidden){if(isHidden) hide(); else show();}
        void slot_shLab(bool state){this->labelHidden=state;update();}
        void slot_routeMenu(QAction* ptr_action);
        void slot_finePosit(bool silent=false);
        void slot_abort(){this->abortSearch=true;}
        void slot_setMode(QAction* ptr_action);
        void slot_setHorn();
        void slot_twaLine(){parent->twaDraw(lon,lat);}
        void slotCompassLine();
        void slot_editRoute();
        void slot_poiRoute();
        void slot_optimizeRoute();
        void slot_simplifyRouteMax();
        void slot_simplifyRouteMin();
        void slot_copyRoute();
        void slot_zoomRoute();
        void slot_relier();
        void slot_pilote();
        void slot_notSimplificable(bool b){this->notSimplificable=b;update();}
        void slot_routage(void) { ROUTAGE * routage=parent->addRoutage(); parent->slot_editRoutage(routage,true,this); }
        void slot_timerSimp();
        void slot_centerOnBoat(void);

        void slot_boatCircleMenu();
        void manageBoatCircle();
signals:
        void chgWP(double,double,double);
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
        double    wph;
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
        int      myBoatId;
        bool     autoRange;

        void update_myStr();

        QMenu *popup;
        QAction * ac_edit;
        QAction * ac_setWp;
        QAction * ac_setGribDate;
        QAction * ac_delPoi;
        QAction * ac_delRoute;
        QAction * ac_editRoute;
        QAction * ac_poiRoute;
        QMenu   * menuSimplify;
        QAction * ac_simplifyRouteMax;
        QAction * ac_simplifyRouteMin;
        QAction * ac_optimizeRoute;
        QAction * ac_copyRoute;
        QAction * ac_zoomRoute;
        QAction * ac_copy;
        QAction * ac_compassLine;
        QAction * ac_twaLine;
        QAction * ac_centerOnPOI;
        QAction * ac_setHorn;
        QMenu * ac_routeList;
        QAction * ac_finePosit;
        QMenu * ac_modeList;
        QAction * ac_modeList1;
        QAction * ac_modeList2;
        QAction * ac_modeList3;
        QAction * ac_connect;
        QAction * ac_boatCircle;
        QAction * ac_pilot;
        QAction * ac_routage;
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
        orthoSegment * lineBetweenPois;
        bool drawLineOrtho;
        QColor lineColor;
        double lineWidth;
        int colorPilototo;
        bool piloteSelected;
        time_t piloteDate;
        double piloteWph;
        bool labelTransp;
        double lonConnected,latConnected;
        int sequence;
        QTimer * timerSimp;
        vlmLine * boatCircle;
};
Q_DECLARE_TYPEINFO(POI,Q_MOVABLE_TYPE);

#endif
