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

***********************************************************************/

#ifndef VLMLINE_H
#define VLMLINE_H

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QPainter>
#include <QObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

#include "class_list.h"
#include "route.h"
#include "vlmPoint.h"




#define VLMLINE_LINE_MODE   0
#define VLMLINE_POINT_MODE  1
#define VLMLINE_GATE_MODE   2

class vlmLine : public QGraphicsWidget
{ Q_OBJECT
    public:
        vlmLine(Projection * proj, myScene * myscene,double z_level);

        void addPoint(const double &lat,const double &lon);
        void addVlmPoint(const vlmPoint &point);
        void removeVlmPoint(const int &index);
        void deleteAll(void);
        void setLinePen ( const QPen & pen ) {linePen = pen;lineWidth=linePen.widthF();calculatePoly(); }
        const QPen getLinePen () const {return this->linePen;}
        void setPoly(const QList<vlmPoint> & points);

        void setLineMode();
        void setPointMode(const QColor &pt_color);
        void setGateMode(const QString &desc);
        QString getDesc() const {return desc;}
        void setIceGate(const int &i){iceGate=i;}
        bool isIceGate() const {return iceGate!=0;}
        int  getIceGate() const {return iceGate;}
        void setTip(QString tip);
        void setNbVacPerHour(const int &nbVacPerHour) {this->nbVacPerHour=nbVacPerHour;}
        void setPorteOnePoint(void){this->onePoint=true;}
        void setHidden(const bool &hidden);
        bool getHidden(void) const {return this->hidden;}
        QList<vlmPoint> * getPoints(){return & this->line;}
        void setSolid(const bool &solid){this->solid=solid;}

        int count(void) const { return line.count(); }
        void setPointDead(const int &n){this->line[n].isDead=true;}
        void setPointIsBroken2(const int &n){this->line[n].isBroken2=true;}
        void setPointStartCap(const int &n,const double &c){this->line[n].startCap=c;}
        void setPointWind(const int &n, const double &twd, const double &tws){this->line[n].wind_angle=twd;this->line[n].wind_speed=tws;}
        void setPointCurrent(const int &n, const double &cd, const double &cs){this->line[n].current_angle=cd;this->line[n].current_speed=cs;}
        void setPointDistIso(const int &n, const double &d){this->line[n].distIso=d;}
        void setPointCapVmg(const int &n, const double &d){this->line[n].capVmg=d;}
        void setPointcapOrigin(const int &n,const double &d){this->line[n].capOrigin=d;}
        void setPointIsoIndex(const int &n, const int &i){this->line[n].isoIndex=i;}
        void setPointCoordProj(const int &n,const double &x,const double &y){this->line[n].lonProj=x;this->line[n].latProj=y;}
        void setNotSimplificable(const int &n){this->line[n].notSimplificable=true;}
        void setLastPointIsPoi(){this->line[line.count()-1].isPOI=true;}
        vlmPoint * getOrigin(const int &n) {return this->line[n].origin;}
        vlmPoint * getPoint(const int &n) {return & line[n];}
        void setInterpolated(const double &lon,const double &lat){this->interpolatedLon=lon;this->interpolatedLat=lat;calculatePoly();}
        void setHasInterpolated(const bool &b){this->hasInterpolated=b;calculatePoly();}
        vlmPoint * getLastPoint() {return & line.last();}
        void setRoundedEnd(const bool &b){this->roundedEnd=b;}
        void setCoastDetection(const bool &b){this->coastDetection=b;}
        bool getCoastDetected(){return this->coastDetected;}
        ~vlmLine();
        void set_zValue(const double &z);
        void drawInMagnifier(QPainter *pnt, Projection *tempProj);
        void setRoute(ROUTE * r){this->route=r;}
        FCT_SETGET_CST(bool, crossOnce)
protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
        QRectF boundingRect() const;
        QPainterPath shape() const;
        void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        static void deletePoly(QPolygon * p){delete p;}
public slots:
        void slot_compassLineToggle(bool b);
        void slot_showMe(void);
        void slot_shLab(bool state){this->labelHidden=state;calculatePoly();}
        void slot_startReplay(bool b){this->replayMode=b;slot_showMe();}
        void slot_replay(int i);
    signals:
        void hovered();
        void unHovered();
    private:
        QList<vlmPoint> line;
        QPen linePen;
        QColor pt_color;
        QList<QPolygon*> polyList;
        bool onePoint;
        int mode;
        int iceGate;
        void calculatePoly(void);
        int nbVacPerHour;
        Projection * proj;
        myScene * myscene;
        QString desc;
        QRectF boundingR;
        bool hidden;
        bool labelHidden;
        bool solid;
        double interpolatedLon;
        double interpolatedLat;
        bool hasInterpolated;
        QRectF r;
        bool replayMode;
        int replayStep;
        bool roundedEnd;
        bool coastDetection;
        bool coastDetected;
        QList<bool>collision;
        GshhsReader *map;
        double lineWidth;
        QPainterPath myPath;
        double myZvalue;
        myCentralWidget * mcp;
        bool drawingInMagnifier;
        QString myToolTip;
        void hover(const bool &isHovered);
        ROUTE * route;
        bool crossOnce;
};
Q_DECLARE_TYPEINFO(vlmLine,Q_MOVABLE_TYPE);


#endif // VLMLINE_H
