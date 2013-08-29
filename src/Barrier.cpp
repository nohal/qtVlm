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
#include "DialogChooseMultipleBoat.h"

Barrier::Barrier(MainWindow *mainWindow,BarrierSet * barrierSet) {
    this->mainWindow=mainWindow;
    centralWidget = mainWindow->getMy_centralWidget();
    projection=centralWidget->getProj();

    this->barrierSet=barrierSet;

    width=1;
    editMode=false;
    isClosed=false;

    setZValue(Z_VALUE_POI-1);
    setPos(0,0);
    centralWidget->getScene()->addItem(this);

    points.clear();
    name="";
    set_color(barrierSet->get_color());

    popUpMenu = new QMenu();
    popUpMenu2 = new QMenu();

    ac_insert = new QAction(tr("Insert a point"),popUpMenu);
    popUpMenu->addAction(ac_insert);
    connect(ac_insert,SIGNAL(triggered()),this,SLOT(slot_insertPoint()));

    chk_closeBarrier = new QCheckBox(tr("Closed barrier"),popUpMenu);
    ac_closeBarrier = new QWidgetAction(popUpMenu);
    ac_closeBarrier->setDefaultWidget(chk_closeBarrier);
    popUpMenu->addAction(ac_closeBarrier);
    connect(chk_closeBarrier,SIGNAL(toggled(bool)),this,SLOT(slot_closeBarrierChg(bool)));

    ac_edit = new QAction(tr("Edit"),popUpMenu);
    popUpMenu->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),barrierSet,SLOT(slot_editBarrierSet()));

    ac_delete = new QAction(tr("Delete barrier"),popUpMenu);
    popUpMenu->addAction(ac_delete);
    connect(ac_delete,SIGNAL(triggered()),this,SLOT(slot_deleteBarrier()));

    ac_assBoats = new QAction(tr("Associate boats"),popUpMenu2);
    popUpMenu->addAction(ac_assBoats);
    popUpMenu2->addAction(ac_assBoats);
    connect(ac_assBoats,SIGNAL(triggered()),this,SLOT(slot_associateBoats()));
}

Barrier::~Barrier(void) {
    clearBarrier();
}

void Barrier::set_isClosed(bool val) {
    if(val!=isClosed) {
        isClosed=val;
        slot_pointPositionChanged();
    }
}

void Barrier::clearBarrier(void) {
    while(!points.isEmpty())
        delete points.takeFirst();
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
    if(points.count()<3) isClosed=false;
    slot_pointPositionChanged();
    barrierSet->cleanEmptyBarrier(this,true);
}

BarrierPoint * Barrier::add_pointAfter(BarrierPoint * basePoint,QPointF position) {
    QPointF screenPos;
    Util::computePosDouble(projection,position,&screenPos);
    int firstLast=is_firstLast(screenPos);
    qWarning() << "firstLast=" << firstLast;
    if(firstLast!=BARRIER_NO_POINT) {
        qWarning() << "basePoint: " << (basePoint==points.first()?"oui":"non") << "," << (basePoint==points.last()?"oui":"non");
        if((basePoint==points.first() && firstLast==BARRIER_LAST_POINT)
                || (basePoint==points.last() && firstLast==BARRIER_FIRST_POINT)) {
            set_isClosed(true);
            return NULL;
        }
        else
            return basePoint;

    }
    BarrierPoint * barrierPoint = new BarrierPoint(mainWindow,this,color);
    barrierPoint->set_position(position);
    if(points.first() == basePoint)
        points.prepend(barrierPoint);
    else
        points.append(barrierPoint);
    slot_pointPositionChanged();
    return barrierPoint;
}

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

    pen.setWidthF(width);
    setPen(pen);

    for(int i=0;i<points.count();++i) {
        points.at(i)->set_editMode(mode);
    }
}

void Barrier::set_sh(bool state) { // state = true => hide
    this->setVisible(!state);
    for(int i=0;i<points.count();++i) {
        points.at(i)->setVisible(!state);
    }
}

void Barrier::set_barrierIsEdited(bool state) {
    for(int i=0;i<points.count();++i) {
        points.at(i)->set_isMovable(state);
    }
}

void Barrier::slot_pointPositionChanged(void) {
    QPointF pointPosition;
    QPainterPath linesPath;

    for(int i=0;i<points.count();++i) {
        Util::computePosDouble(projection,points.at(i)->get_position(),&pointPosition);
        if(i!=0)
           linesPath.lineTo(pointPosition);
        linesPath.moveTo(pointPosition);
    }

    if(isClosed && points.count()>=3) {
        Util::computePosDouble(projection,points.at(0)->get_position(),&pointPosition);
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
    if(popUpMenu && mainWindow->get_barrierIsEditing() && centralWidget->get_barrierEditMode()==BARRIER_EDIT_NO_EDIT) {
        chk_closeBarrier->blockSignals(true);
        if(points.count()<3) {
            chk_closeBarrier->setEnabled(false);
            chk_closeBarrier->setChecked(false);
        }
        else {
            chk_closeBarrier->setEnabled(true);
            chk_closeBarrier->setChecked(isClosed);
        }
        chk_closeBarrier->blockSignals(false);

        cursorPosition = e->scenePos();
        popUpMenu->exec(QCursor::pos());
    }
    else if(popUpMenu2 && !mainWindow->get_barrierIsEditing()) {
        popUpMenu2->exec(QCursor::pos());
    }
}

void Barrier::slot_closeBarrierChg(bool status) {
    QObject * senderObj = QObject::sender();
    if(senderObj) {
        ((QMenu *)(senderObj->parent()))->close();
    }

    if(points.count()<3)
        set_isClosed(false);
    else
        set_isClosed(status);
}

void Barrier::slot_associateBoats(void) {
    DialogChooseMultipleBoat::chooseBoat(mainWindow,barrierSet,centralWidget->get_boatList());
}

bool Barrier::cross(QLineF line) {
    for(int i=0;i<(points.count()-1);++i) {
        QLineF l1(points.at(i)->get_scenePosition(),points.at(i+1)->get_scenePosition());
        if(l1.intersect(line,NULL)==QLineF::BoundedIntersection)
            return true;
    }
    if(isClosed) {
        QLineF l1(points.at(0)->get_scenePosition(),points.at(points.count()-1)->get_scenePosition());
        if(l1.intersect(line,NULL)==QLineF::BoundedIntersection)
            return true;
    }
    return false;
}

void Barrier::printBarrier(void) {
    for(int i=0;i<(points.count()-1);++i) {
        qWarning() << "P" << i << ": " << points.at(i)->get_position();
    }
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
    setData(0,BARRIERPOINT_WTYPE);

    /*** move management **/
    isMoving=false;
    isMovable=mainWindow->get_barrierIsEditing();

    setBrush(color);
    setPen(color);

    set_editMode(barrier->get_editMode());

    connect(projection,SIGNAL(projectionUpdated()),this,SLOT(slot_projectionUpdated()));

    popUpMenu = new QMenu();
    popUpMenu2 = new QMenu();

    ac_remove = new QAction(tr("Remove point"),popUpMenu);
    popUpMenu->addAction(ac_remove);
    connect(ac_remove,SIGNAL(triggered()),this,SLOT(slot_removePoint()));

    ac_insertAfter = new QAction(tr("Insert a point"),popUpMenu);
    popUpMenu->addAction(ac_insertAfter);
    connect(ac_insertAfter,SIGNAL(triggered()),this,SLOT(slot_insertAfter()));

    ac_edit = new QAction(tr("Edit"),popUpMenu);
    popUpMenu->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),barrier->get_barrierSet(),SLOT(slot_editBarrierSet()));

    chk_closeBarrier = new QCheckBox(tr("Closed barrier"),popUpMenu);
    ac_closeBarrier = new QWidgetAction(popUpMenu);
    ac_closeBarrier->setDefaultWidget(chk_closeBarrier);
    popUpMenu->addAction(ac_closeBarrier);
    connect(chk_closeBarrier,SIGNAL(toggled(bool)),barrier,SLOT(slot_closeBarrierChg(bool)));

    ac_deleteBarrier = new QAction(tr("Delete barrier"),popUpMenu);
    popUpMenu->addAction(ac_deleteBarrier);
    connect(ac_deleteBarrier,SIGNAL(triggered()),barrier,SLOT(slot_deleteBarrier()));

    ac_assBoats = new QAction(tr("Associate boats"),popUpMenu2);
    popUpMenu->addAction(ac_assBoats);
    popUpMenu2->addAction(ac_assBoats);
    connect(ac_assBoats,SIGNAL(triggered()),barrier,SLOT(slot_associateBoats()));
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
    QPointF pointPosition;
    Util::computePosDouble(projection,position,&pointPosition);
    this->position=position;
    setPos(pointPosition);
}

void BarrierPoint::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && e->modifiers()==Qt::ShiftModifier)
    {
        previousPos=position;
        isMoving=true;
        mousePos=e->scenePos();
        setPos(mousePos);
        setCursor(Qt::BlankCursor);
        // is this needed ?
        //update();
    }
    else
        e->ignore();

}

bool BarrierPoint::tryMoving(QPoint pos)
{
    if(isMoving)
    {
        mousePos=pos;
        QPointF newPos;
        // need to move point a bit
        projection->screen2mapDouble(mousePos,&newPos);
        set_position(newPos);
        emit positionChanged();
        return true;
    }
    return false;
}

void BarrierPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(isMoving)
    {
        QPointF newPos;
        if(e->modifiers()==Qt::ShiftModifier)
        {
            projection->screen2mapDouble(mousePos,&newPos);
        }
        else
        {
            newPos=previousPos;
        }
        set_position(newPos);
        isMoving=false;
        setCursor(Qt::PointingHandCursor);
        emit positionChanged();
        return;
    }

    if(e->pos().x() < 0 || e->pos().x()>pointSize || e->pos().y() < 0 || e->pos().y() > pointSize)
    {
        e->ignore();
        return;
    }
}

void BarrierPoint:: slot_projectionUpdated(void) {
    set_position(position);
    emit positionChanged();
}

void BarrierPoint::slot_removePoint(void) {
    barrier->removePoint(this);
    deleteLater();
}

void BarrierPoint::slot_insertAfter(void) {    
    centralWidget->insert_barrierPointAfterPoint(this);
}

void BarrierPoint::contextMenuEvent (QGraphicsSceneContextMenuEvent *) {
    if(popUpMenu && mainWindow->get_barrierIsEditing() && centralWidget->get_barrierEditMode()==BARRIER_EDIT_NO_EDIT) {
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

        chk_closeBarrier->blockSignals(true);
        if(barrier->get_points()->count()<3) {
            chk_closeBarrier->setEnabled(false);
            chk_closeBarrier->setChecked(false);
        }
        else {
            chk_closeBarrier->setEnabled(true);
            chk_closeBarrier->setChecked(barrier->get_isClosed());
        }
        chk_closeBarrier->blockSignals(false);

        popUpMenu->exec(QCursor::pos());
    }
    else if(popUpMenu2 && !mainWindow->get_barrierIsEditing()) {
        popUpMenu2->exec(QCursor::pos());
    }
}

QPointF BarrierPoint::get_scenePosition(void) {
    /*QPointF p;
    Util::computePosDouble(projection,position,&p);
    return p;*/
    return pos();
}
