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

#include <QDebug>
#include "Orthodromie.h"
#include "Projection.h"
#include "vlmLine.h"
#include "settings.h"
#include "Util.h"
#include "GshhsReader.h"
#include "mycentralwidget.h"
#include <QGestureEvent>
#include "Terrain.h"


vlmLine::vlmLine(Projection * proj, myScene *myscene, double z_level) :
   QGraphicsWidget(),
   roundedEnd (false)
{
    this->mcp=myscene->getMcp();
    this->myZvalue=z_level;
    this->proj=proj;
    this->myscene=myscene;
    drawingInMagnifier=false;
    connect(mcp->get_terrain(),SIGNAL(terrainUpdated()),this,SLOT(slot_showMe()));
    connect(mcp,SIGNAL(compassLineToggle(bool)),this,SLOT(slot_compassLineToggle(bool)));
    myscene->addItem(this);
    this->setZValue(z_level);
    this->linePen=QPen(Qt::red);
    linePen.setBrush(Qt::red);
    linePen.setWidth(2);
    lineWidth=2;
    pt_color=QColor(Qt::red);
    mode=VLMLINE_LINE_MODE;
    nbVacPerHour=12;
    this->onePoint=false;
    this->hidden=false;
    this->boundingR.setRect(0,0,0,0);
    this->labelHidden=false;
    this->solid=false;
    this->hasInterpolated=false;
    this->interpolatedLon=0;
    this->interpolatedLat=0;
    this->iceGate=0;
    this->replayMode=false;
    this->replayStep=0;
    this->coastDetected=false;
    this->coastDetection=false;
    if(myZvalue==Z_VALUE_ROUTE || myZvalue==Z_VALUE_BOAT || myZvalue==Z_VALUE_OPP || myZvalue==Z_VALUE_LINE_POI)
        this->setAcceptHoverEvents(true);
    if(Settings::getSetting(enable_Gesture).toString()=="1")
    {
        this->grabGesture(Qt::TapGesture);
    }
    this->setFlag(QGraphicsWidget::ItemIsSelectable,false);
    show();
}
bool vlmLine::event(QEvent *event)
{
    if(this->zValue()==Z_VALUE_ROUTE)
        qWarning()<<"event in vlmLine (route)"<<event->type();
    return QGraphicsWidget::event(event);
}

bool vlmLine::sceneEvent(QEvent *event)
{
    if(this->zValue()==Z_VALUE_ROUTE)
        qWarning()<<"sceneEvent in vlmLine (route)"<<event->type();
    if (event->type() == QEvent::Gesture)
    {
        QGraphicsWidget::sceneEvent(event);
        event->accept();
        QGestureEvent * gestureEvent=static_cast<QGestureEvent*>(event);
        foreach (QGesture * gesture, gestureEvent->gestures())
        {
            gestureEvent->accept(gesture);
            if(gesture->gestureType()==Qt::TapGesture)
                qWarning()<<"tap gesture in vlmLine"<<gesture->state();
            else if (gesture->gestureType()==Qt::TapAndHoldGesture)
            {
                qWarning()<<"tapAndHold gesture in vlmLine"<<gesture->state();
                QTapAndHoldGesture *p=static_cast<QTapAndHoldGesture*>(gesture);
                if(p->state()==Qt::GestureFinished && this->scene()->mouseGrabberItem()==this)
                {
                    QPointF tapCenter=gesture->hotSpot();
                    QPoint screenPos=tapCenter.toPoint();
                    QPointF scenePos=mcp->mapFromGlobal(screenPos);
                    mcp->clearOtherSelected(NULL);
                    int X=qRound(scenePos.x());
                    int Y=qRound(scenePos.y());
                    hover(true);
                    mcp->getMainWindow()->showContextualMenu(X,Y,screenPos);
                    hover(false);
                }
            }
            else
                qWarning()<<"unexpected gesture in vlmLine:"<<gesture->gestureType()<<gesture->state();
        }
        return true;
    }
    return QGraphicsWidget::sceneEvent(event);
}
QVariant vlmLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(this->zValue()==Z_VALUE_ROUTE)
        qWarning()<<"itemChange in vlmLine (route)"<<change;
    if(change==ItemToolTipHasChanged)
    {
        if(isSelected())
            mcp->showToolTip(toolTip(),false);
    }
    if(change==ItemSelectedHasChanged)
    {
        if(!isSelected())
        {
            mcp->showToolTip("");
            hover(false);
        }
        else
        {
            mcp->clearOtherSelected(this);
            mcp->showToolTip(toolTip());
            hover(true);
        }
    }
    return QGraphicsWidget::itemChange(change,value);
}

void vlmLine::slot_compassLineToggle(bool b)
{
    if(!b)
        this->setToolTip(myToolTip);
    else
        this->setToolTip("");
}

void vlmLine::set_zValue(const double &z)
{
    this->myZvalue=z;
    this->setZValue(z);
}

vlmLine::~vlmLine()
{
    setSelected(false);
    qDeleteAll(polyList);
    polyList.clear();
}
void vlmLine::slot_replay(int i)
{
    this->replayStep=i;
    calculatePoly();
}

QRectF vlmLine::boundingRect() const
{
    return boundingR;
}

void vlmLine::addPoint(const double &lat, const double &lon)
{
    vlmPoint point(lon,lat);
    line.append(point);
}
void vlmLine::addVlmPoint(const vlmPoint &point)
{
    line.append(point);
}
void vlmLine::removeVlmPoint(const int &index)
{
    line.removeAt(index);
}

void vlmLine::setPoly(const QList<vlmPoint> & points)
{
    line=points;
    calculatePoly();
}

void vlmLine::slot_showMe()
{
    calculatePoly();
}

void vlmLine::setLineMode()
{
    mode = VLMLINE_LINE_MODE;
    calculatePoly();
}

void vlmLine::setPointMode(const QColor &pt_color)
{
    mode = VLMLINE_POINT_MODE;
    this->pt_color = pt_color;
    calculatePoly();
}
void vlmLine::setGateMode(const QString &desc)
{
    mode = VLMLINE_GATE_MODE;
    this->desc=desc;
    calculatePoly();
}
void vlmLine::setTip(QString tip)
{
    tip=tip.replace(" ","&nbsp;");
    tip="<qt>"+tip+"</qt>";
    setToolTip(tip);
    myToolTip=tip;
}
void vlmLine::setHidden(const bool &hidden)
{
    this->hidden=hidden;
    this->setVisible(!hidden);
    calculatePoly();
}
void vlmLine::calculatePoly(void)
{
    if(this->zValue()==Z_VALUE_ROUTE)
        qWarning()<<"calculatePoly in vlmLine (route)";
    collision.clear();
    qDeleteAll(polyList);
    polyList.clear();
    coastDetected=false;
    if(hidden || mcp->getIsStartingUp())
    {
        prepareGeometryChange();
        boundingR=QRectF();
        myPath=QPainterPath();
        return;
    }
    int n=0;
    double X,Y,previousX=0,previousY=0;
    QPainterPath myPath2;
    QRectF tempBound;
    tempBound.setRect(0,0,0,0);
    QPolygon * poly=new QPolygon();
    polyList.append(poly);
    vlmPoint previousWorldPoint(0,0);
    bool coasted=false;
    int cc=-1;
    GshhsReader * map=NULL;
    if(mcp)
        map=mcp->get_gshhsReader();
    if(line.count()>1 && this->isVisible())
    {
        QList<vlmPoint>::const_iterator i;
        for (i = line.constBegin(); i != line.constEnd(); ++i)
        {
            ++cc;
            const vlmPoint worldPoint=*i;
            if(replayMode)
            {
                if(worldPoint.timeStamp>replayStep) break;
            }
            //if(worldPoint.isDead) continue;
            if(worldPoint.isBroken && n==0) continue;
            if(n==0)
                proj->map2screenDouble(worldPoint.lon,worldPoint.lat,&X,&Y);
            else
                proj->map2screenByReference(previousWorldPoint.lon,previousX,worldPoint.lon,worldPoint.lat,&X,&Y);
            bool reverseWorld=false;
            if(n!=0)
                reverseWorld=(!proj->isInBounderies(X,Y) && proj->isPointVisible(worldPoint.lon,worldPoint.lat)) ||
                             (!proj->isInBounderies(previousX,previousY) && proj->isPointVisible(previousWorldPoint.lon,previousWorldPoint.lat));
            poly->putPoints(n,1,X-x(),Y-y());
            if(this->coastDetection && n!=0 && !coasted && map && map->crossing(QLineF(previousX,previousY,X,Y),
               QLineF(previousWorldPoint.lon,previousWorldPoint.lat,worldPoint.lon,worldPoint.lat)))
            {
                coasted=true;
                coastDetected=true;
            }
            if(reverseWorld)
            {
                //qWarning()<<"reverseWorld detected";
                collision.append(coasted);
                tempBound=tempBound.united(poly->boundingRect());
                poly=new QPolygon();
                polyList.append(poly);
                proj->map2screenDouble(worldPoint.lon,worldPoint.lat,&X,&Y);
                double myX=X;
                double myY=Y;
                previousX=X;
                poly->putPoints(0,1,X-x(),Y-y());
                proj->map2screenByReference(worldPoint.lon,previousX,previousWorldPoint.lon,previousWorldPoint.lat,&X,&Y);
                poly->putPoints(1,1,X-x(),Y-y());
                collision.append(coasted);
                tempBound=tempBound.united(poly->boundingRect());
                poly=new QPolygon();
                polyList.append(poly);
                poly->putPoints(0,1,myX-x(),myY-y());
                coasted=false;
                n=1;
                previousWorldPoint=worldPoint;
                previousX=myX;
                previousY=myY;
                continue;
            }
            previousWorldPoint=worldPoint;
            previousX=X;
            previousY=Y;
            if(worldPoint.isBroken)
            {
                collision.append(coasted);
                tempBound=tempBound.united(poly->boundingRect());
                poly=new QPolygon();
                n=0;
                polyList.append(poly);
                coasted=false;
                continue;
            }




            if(worldPoint.isPOI && cc!=0 && cc!=line.count()-1)
            {
                collision.append(coasted);
                tempBound=tempBound.united(poly->boundingRect());
                poly=new QPolygon();
                n=0;
                polyList.append(poly);
                poly->putPoints(n,1,X,Y);
                coasted=false;
            }
            ++n;
        }
        tempBound=tempBound.united(poly->boundingRect());
    }
    else
    {
        qDeleteAll(polyList);
        polyList.clear();
    }
    collision.append(coasted);
    if(!polyList.isEmpty())
    {
        poly=polyList.first();
        if(poly->size()!=0)
        {
            int width=20;
            int height=10;
            setFont(QApplication::font());
            QFontMetrics fm(font());
            width = fm.width(desc) + 10 +2;
            height = qMax(fm.height()+2,10);
            int x=poly->point(0).x()+5;
            int y=poly->point(0).y()-5;
            if(iceGate==1)
                y-=10;
            else if(iceGate==2)
                y+=10;
            QRectF r;
            r.setRect(x,y, width-10,height-1);
            double x1,y1,x2,y2;
            tempBound=tempBound.united(r);
            tempBound=tempBound.normalized();
            tempBound.getCoords(&x1,&y1,&x2,&y2);
            tempBound.setCoords(x1-linePen.widthF()*2,y1-linePen.widthF()*2,x2+linePen.widthF()*2,y2+linePen.widthF()*2);
        }
        foreach(const QPolygon * pol,polyList)
            myPath2.addPolygon(*pol);
    }
    if(drawingInMagnifier) return;
    prepareGeometryChange();
    boundingR=tempBound;
    myPath=myPath2;
}
void vlmLine::drawInMagnifier(QPainter * pnt, Projection * tempProj)
{
    if(!this->isVisible() || this->hidden) return;
    drawingInMagnifier=true;
    Projection * myProj=proj;
    proj=tempProj;
    calculatePoly();
    paint(pnt,NULL,NULL);
    proj=myProj;
    drawingInMagnifier=false;
    calculatePoly();
}
void vlmLine::hover(const bool &isHovered)
{
    prepareGeometryChange();
    if(isHovered)
    {
        this->setZValue(myZvalue+0.5);
        this->linePen.setWidthF(this->lineWidth*2.0);
        emit hovered();
    }
    else
    {
        this->setZValue(this->myZvalue);
        this->linePen.setWidthF(this->lineWidth);
        emit unHovered();
    }
}

void vlmLine::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    hover(true);
    QGraphicsItem::hoverEnterEvent(e);
}
void vlmLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    hover(false);
    QGraphicsItem::hoverLeaveEvent(e);
}
void vlmLine::deleteAll()
{
    line.clear();
    calculatePoly();
}

void vlmLine::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    if(this->hidden || polyList.isEmpty()) return;
//    if(this->zValue()==Z_VALUE_ROUTE)
//        qWarning()<<"route inside paint"<<QDateTime::currentMSecsSinceEpoch();
    pnt->setRenderHint(QPainter::Antialiasing);
    pnt->setPen(linePen);
    QPen coastedPen=linePen;
    if(linePen.color()==Qt::red)
        coastedPen.setColor(Qt::black);
    else
        coastedPen.setColor(Qt::red);
    coastedPen.setWidthF(linePen.widthF()*2);
    double penW=linePen.widthF();
    if(polyList.isEmpty()) return;
    QPolygon * poly;
    QListIterator<QPolygon*> nPoly (polyList);
    bool labelAlreadyMade=false;
    int nn=-1;
    while(nPoly.hasNext())
    {
        poly=nPoly.next();
        ++nn;
        if(this->coastDetection && collision.at(nn))
            pnt->setPen(coastedPen);
        else
            pnt->setPen(linePen);
        if(poly->size()==0) continue;
        //pnt->drawText(poly->at(0),QString().setNum(nn));
        switch(mode)
        {
        case VLMLINE_LINE_MODE:
            if(!hidden)
            {
                if (!solid)
                {
                    pnt->drawPolyline(*poly);
                }
                else
                {
                    pnt->setBrush(linePen.brush());
                    pnt->drawPolygon(*poly,Qt::WindingFill);
                }
            }
            break;
        case VLMLINE_POINT_MODE:
            if(!hidden)
            {
                int nbVac=nbVacPerHour*Settings::getSetting(traceLength).toInt()+1;
                int x0=poly->point(0).x();
                int y0=poly->point(0).y();
                for(int i=1;i<poly->count() && i<nbVac;++i)
                {
                    int x,y;
                    x=poly->point(i).x();
                    y=poly->point(i).y();
                    if(i%10)
                        linePen.setWidthF(penW+1);
                    else
                        linePen.setWidthF(penW+2);
                    pnt->setPen(linePen);
                    pnt->drawPoint(x,y);
                    linePen.setWidthF(penW);
                    pnt->setPen(linePen);
                    pnt->drawLine(x0,y0,x,y);
                    x0=x;
                    y0=y;
                }
            }
            break;
        case VLMLINE_GATE_MODE:
            if(onePoint)
            {
                linePen.setStyle(Qt::DashDotDotLine);
                pnt->setPen(linePen);
            }
            if(!hidden)
            {
                pnt->drawPolyline(*poly);
            }
            linePen.setWidthF(penW*2);
            if(onePoint)
            {
                linePen.setStyle(Qt::SolidLine);
            }
            pnt->setPen(linePen);
            if(onePoint)
                pnt->drawPoint(poly->point(0));
            else
                pnt->drawPoints(*poly);
            linePen.setWidthF(penW);
            pnt->setPen(linePen);
            if(labelAlreadyMade || labelHidden) break;
            QColor bgcolor = QColor(255,255,255,150);
            int width=20;
            int height=10;
            setFont(QApplication::font());
            QFontMetrics fm(font());
            width = fm.width(desc) + 10 +2;
            height = qMax(fm.height()+2,10);
            int x=poly->point(0).x()+5;
            int y=poly->point(0).y()-5;
            if(this->iceGate==1)
                y-=10;
            else if(this->iceGate==2)
                y+=10;
            pnt->setFont(font());
            QPen linePenBis(Qt::black,1);
            linePenBis.setWidthF(1);
            pnt->setPen(linePenBis);
            r.setRect(x,y, width-10,height-1);
            pnt->fillRect(r, QBrush(bgcolor));
            pnt->drawRect(r);
            pnt->drawText(x,y+fm.height()-2,desc);
            pnt->setPen(linePen);
            labelAlreadyMade=true;
            break;
        }
    }
    if(hasInterpolated && !hidden)
    {
        linePen.setWidthF(penW+10);
        pnt->setPen(linePen);
        int X,Y;
        Util::computePos(proj,interpolatedLat, interpolatedLon, &X, &Y);
        pnt->drawPoint(X,Y);
        linePen.setWidthF(penW);
        pnt->setPen(linePen);
    }
    if(roundedEnd && !hidden && !polyList.last()->isEmpty())
    {
        linePen.setWidthF(penW+1);
        pnt->setPen(linePen);
        pnt->drawPoint(polyList.last()->last());
        linePen.setWidthF(penW);
        pnt->setPen(linePen);
    }
    //pnt->drawPath(shape());
}

QPainterPath vlmLine::shape() const
{
    QPainterPath path;
    if(mode==VLMLINE_GATE_MODE)
    {
        if(this->labelHidden)
            path.addRect(QRect(0,0,0,0));
        else
            path.addRect(r);
    }
    else
    {
        QPainterPathStroker stroker;
#ifdef __ANDROID__
        stroker.setWidth(Util::getFingerSize()/2.0);
#else
        stroker.setWidth(4);
#endif
        stroker.setJoinStyle(Qt::BevelJoin);
        path=stroker.createStroke(myPath);
        path.setFillRule(Qt::WindingFill);
    }
    return path;
}
