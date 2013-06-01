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

#include "Barrier.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "Projection.h"
#include "BarrierSet.h"
#include "Util.h"
#include "MyView.h"

Barrier::Barrier(MainWindow *mainWindow,BarrierSet * barrierSet) {
    this->mainWindow=mainWindow;
    myCentralWidget * centralWidget = mainWindow->getMy_centralWidget();
    projection=centralWidget->getProj();

    this->barrierSet=barrierSet;

    width=1;
    editMode=false;

    setZValue(Z_VALUE_POI-1);
    setPos(0,0);
    centralWidget->getScene()->addItem(this);

    points.clear();
    name="";
    set_color(barrierSet->get_color());

    popUpMenu = new QMenu();

    ac_insert = new QAction(tr("Insert a point"),popUpMenu);
    popUpMenu->addAction(ac_insert);
    connect(ac_insert,SIGNAL(triggered()),this,SLOT(slot_insertPoint()));

    ac_edit = new QAction(tr("Edit"),popUpMenu);
    popUpMenu->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),barrierSet,SLOT(slot_editBarrierSet()));

    ac_delete = new QAction(tr("Delete barrier"),popUpMenu);
    popUpMenu->addAction(ac_delete);
    connect(ac_delete,SIGNAL(triggered()),this,SLOT(slot_deleteBarrier()));
}

Barrier::~Barrier(void) {
    clearBarrier();
}

void Barrier::clearBarrier(void) {
    while(!points.isEmpty())
        delete points.takeFirst();
}

void Barrier::initBarrier(QList<QPointF> pointList) {
    if(pointList.isEmpty()) return;

    clearBarrier();

    BarrierPoint * point;
    QPainterPath linesPath;
    QPointF pointPosition;


    for(int i=0;i<pointList.count();++i) {
        // MOD BARRIER
        //pointPosition=myView->mapEarthToScene(pointList.at(i));
        Util::computePosDouble(projection,pointList.at(i),&pointPosition);

        point = new BarrierPoint(mainWindow,this,color);
        point->set_position(pointList.at(i));
        points.append(point);

        if(i!=0)
           linesPath.lineTo(pointPosition);
        linesPath.moveTo(pointPosition);
    }

    QPainterPathStroker stroker;
    stroker.setWidth(8);
    shapePath=stroker.createStroke(linesPath);

    setPath(linesPath);

}

BarrierPoint * Barrier::appendPoint(QPointF point) {
    BarrierPoint * barrierPoint = new BarrierPoint(mainWindow,this,color);
    barrierPoint->set_position(point);
    points.append(barrierPoint);
    slot_pointPositionChanged();
    return barrierPoint;
}

void Barrier::removePoint(BarrierPoint * point) {
    points.removeAll(point);
    slot_pointPositionChanged();
}

BarrierPoint * Barrier::add_pointAfter(BarrierPoint * basePoint,QPointF position) {
    BarrierPoint * barrierPoint = new BarrierPoint(mainWindow,this,color);
    barrierPoint->set_position(position);
    if(points.first() == basePoint)
        points.prepend(barrierPoint);
    else
        points.append(barrierPoint);
    slot_pointPositionChanged();
    return barrierPoint;
}


#if 0
/* insert a new Point starting from given point */
/* point should only be the first or last point */

void Barrier::insertNewPoint(BarrierPoint * point) {
    double len = 20/myView->transform().m11();
    if(points.count() == 1) {
        /* only one point => add it at a given distance */
        BarrierPoint * newPoint = new BarrierPoint(mainWindow,this,color);
        points.append(newPoint);
        newPoint->set_position(myView->mapSceneToEarth(
                                   points.at(0)->get_scenePosition()
                                   +QPointF(len,len)));
        slot_pointPositionChanged();
    }
    else if(point == points.first()) {
        BarrierPoint * firstPoint = points.first();
        BarrierPoint * secPoint = points.at(1);
        /* compute position keeping prev segment direction, moving a bit in this direction */
        QPointF newPointPosition = QLineF(secPoint->get_scenePosition(),firstPoint->get_scenePosition()).pointAt(1.5);
        QLineF line= QLineF(firstPoint->get_scenePosition(),newPointPosition);
        line.setLength(len);
        newPointPosition = line.p2();
        /* create point */
        BarrierPoint * newPoint = new BarrierPoint(mainWindow,this,color);
        points.prepend(newPoint);
        newPoint->set_position(myView->mapSceneToEarth(newPointPosition));
        slot_pointPositionChanged();
    }
    else if(point == points.last()) {
        BarrierPoint * lastPoint = points.last();
        BarrierPoint * prevPoint = points.at(points.count()-2);
        /* compute position keeping prev segment direction, moving a bit in this direction */
        QPointF newPointPosition = QLineF(prevPoint->get_scenePosition(),lastPoint->get_scenePosition()).pointAt(1.5);
        QLineF line= QLineF(lastPoint->get_scenePosition(),newPointPosition);
        line.setLength(len);
        newPointPosition = line.p2();
        /* create point */
        BarrierPoint * newPoint = new BarrierPoint(mainWindow,this,color);
        points.append(newPoint);
        newPoint->set_position(myView->mapSceneToEarth(newPointPosition));
        slot_pointPositionChanged();
    }
}
#endif

void Barrier::set_color(QColor color) {
    this->color=color;
    for(int i=0;i<points.count();++i) {
        points.at(i)->setBrush(color);
        points.at(i)->setPen(color);
    }
    /* changing path color */
    setPen(color);

    /* redraw path */
    slot_pointPositionChanged();
}

void Barrier::set_editMode(bool mode) {
    editMode=mode;
    QPen pen = this->pen();

    if(mode)
        width=2;
    else
        width=1;

    //MOD BARRIER
    //pen.setWidthF(width/myView->transform().m11());
    pen.setWidthF(width);
    setPen(pen);

    for(int i=0;i<points.count();++i) {
        points.at(i)->set_editMode(mode);
    }
}

void Barrier::set_barrierIsEdited(bool state) {
    for(int i=0;i<points.count();++i) {
        points.at(i)->setFlag(QGraphicsItem::ItemIsMovable,state);
    }
}

void Barrier::slot_pointPositionChanged(void) {
    QPointF pointPosition;
    QPainterPath linesPath;

    for(int i=0;i<points.count();++i) {        
        // MOD BARRIER
        //pointPosition=myView->mapEarthToScene(points.at(i)->get_position());
        Util::computePosDouble(projection,points.at(i)->get_position(),&pointPosition);
        if(i!=0)
           linesPath.lineTo(pointPosition);
        linesPath.moveTo(pointPosition);
    }

    QPainterPathStroker stroker;
    stroker.setWidth(8);

    prepareGeometryChange();
    shapePath=stroker.createStroke(linesPath);

    setPath(linesPath);
}

QPainterPath Barrier::shape() const {
    return shapePath;
}

void Barrier::slot_adjustWidthF(void)
{
    QPen pen;
    pen.setColor(color);
    //MOD BARRIER
    //pen.setWidthF(width/myView->transform().m11());
    pen.setWidthF(width);
    this->setPen(pen);
}

/* insert a point in a segment */
/* called by barrier contextual menu  */

void Barrier::slot_insertPoint(void) {
    //QPointF newPos;
    //Util::computePosDouble(projection,cursorPosition,&newPos);

    /* find which line we belong to */
    QPointF P1,P2;

    double dist_min=10e6;
    int index_min=-1;
    for(int i=0;i<points.count()-1;++i) {
        P1=points.at(i)->get_scenePosition();
        P2=points.at(i+1)->get_scenePosition();

        double dist=Util::distToSegment(cursorPosition,QLineF(P1,P2));
        if(dist<dist_min) {
            dist_min=dist;
            index_min=i;
        }

        qWarning() << i << ": " << dist << " - min=" << dist_min << ", index=" << index_min;
    }

    if(index_min !=-1) {
            BarrierPoint * newPoint = new BarrierPoint(mainWindow,this,color);
            points.insert(index_min+1,newPoint);
            QPointF p;
            projection->screen2mapDouble(cursorPosition,&p);
            newPoint->set_position(p);
            slot_pointPositionChanged();
    }
}

int Barrier::is_firstLast(QPointF screenPosition) {
    if(points.isEmpty()) return BARRIER_NO_POINT;

    if(points.first()->is_same(screenPosition))
        return BARRIER_FIRST_POINT;

    if(points.last()->is_same(screenPosition))
        return BARRIER_LAST_POINT;
    return BARRIER_NO_POINT;
}

void Barrier::slot_deleteBarrier(void) {
    barrierSet->get_barriers()->removeAll(this);
    deleteLater();
}

void Barrier::contextMenuEvent (QGraphicsSceneContextMenuEvent * e) {
    if(popUpMenu && mainWindow->get_barrierIsEditing()) {
        cursorPosition = e->scenePos();
        popUpMenu->exec(QCursor::pos());
    }
}

bool Barrier::cross(QLineF line) {
    for(int i=0;i<(points.count()-1);++i) {
        QLineF l1(points.at(i)->get_scenePosition(),points.at(i+1)->get_scenePosition());
        if(l1.intersect(line,NULL)==QLineF::BoundedIntersection)
            return true;
    }
    return false;
}


/*****************************************************************************************/
/*                              BarrierPoint                                             */
/*****************************************************************************************/

#define POINT_WIDTH 5

BarrierPoint::BarrierPoint(MainWindow * mainWindow, Barrier *barrier, QColor color):QGraphicsRectItem() {
    this->mainWindow=mainWindow;
    centralWidget = mainWindow->getMy_centralWidget();
    projection=centralWidget->getProj();
    this->barrier=barrier;

    pointSize=POINT_WIDTH;

    connect(this,SIGNAL(positionChanged()),barrier,SLOT(slot_pointPositionChanged()));

    centralWidget->getScene()->addItem(this);

    setZValue(Z_VALUE_POI);

    // MOD BARRIER
    //resetTransform();

    setBrush(color);
    setPen(color);


    setFlag(QGraphicsItem::ItemIsMovable,mainWindow->get_barrierIsEditing());
    // MOD BARRIER
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    //setFlag(QGraphicsItem::ItemIgnoresTransformations);

    set_editMode(barrier->get_editMode());

    popUpMenu = new QMenu();

    ac_remove = new QAction(tr("Remove point"),popUpMenu);
    popUpMenu->addAction(ac_remove);
    connect(ac_remove,SIGNAL(triggered()),this,SLOT(slot_removePoint()));

    ac_insertAfter = new QAction(tr("Insert a point"),popUpMenu);
    popUpMenu->addAction(ac_insertAfter);
    connect(ac_insertAfter,SIGNAL(triggered()),this,SLOT(slot_insertAfter()));

    ac_edit = new QAction(tr("Edit"),popUpMenu);
    popUpMenu->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),barrier->get_barrierSet(),SLOT(slot_editBarrierSet()));

    ac_deleteBarrier = new QAction(tr("Delete barrier"),popUpMenu);
    popUpMenu->addAction(ac_deleteBarrier);
    connect(ac_deleteBarrier,SIGNAL(triggered()),barrier,SLOT(slot_deleteBarrier()));
}

void BarrierPoint::set_editMode(bool mode) {
    int pointSize=mode?POINT_WIDTH+2:POINT_WIDTH;
    setRect(-pointSize/2,-pointSize/2,pointSize,pointSize);
}

bool BarrierPoint::is_same(QPointF screenPosition) {
    QRectF zone;
    zone.moveCenter(pos());
    zone.setSize(QSizeF(pointSize,pointSize));
    return zone.contains(screenPosition);
}

void BarrierPoint::set_position(QPointF position) {
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,false);
    QPointF pointPosition;
    Util::computePosDouble(projection,position,&pointPosition);
    this->position=position;
    setPos(pointPosition);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

QVariant BarrierPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = centralWidget->getView()->mapFromGlobal(QCursor::pos())-QPoint(pointSize/2,pointSize/2);
        QRectF rect = QRect(0,0,projection->getW(),projection->getH());
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
        }
        projection->screen2mapDouble(newPos,&position);
        emit positionChanged();
        return newPos;
    }
    else
        return QGraphicsItem::itemChange(change, value);

}

void BarrierPoint::slot_removePoint(void) {
    barrier->removePoint(this);
    deleteLater();
}

void BarrierPoint::slot_insertAfter(void) {    
    centralWidget->insert_barrierPointAfterPoint(this);
}

void BarrierPoint::contextMenuEvent (QGraphicsSceneContextMenuEvent *) {
    if(popUpMenu && mainWindow->get_barrierIsEditing()) {
        /* should we display the 'insert point' entry ? */
        ac_insertAfter->setEnabled(true);
        QList<BarrierPoint * > * points = barrier->get_points();
        if(points) {
            if(points->count()>1) {
                int index = points->indexOf(this);
                if(index != -1) {
                    if(index!=0 && index != (points->count()-1))
                        ac_insertAfter->setEnabled(false);
                }
            }
        }
        popUpMenu->exec(QCursor::pos());
    }
}

QPointF BarrierPoint::get_scenePosition(void) {
    QPointF p;
    Util::computePosDouble(projection,position,&p);
    return p;
}
