#include "vlmpointgraphic.h"
#include "Util.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "routage.h"

vlmPointGraphic::vlmPointGraphic(ROUTAGE * routage,int isoNb, int pointIsoNb,float lon, float lat, Projection * proj, QGraphicsScene * myScene,int z_level) : QGraphicsWidget()
{
    this->proj=proj;
    this->myScene=myScene;
    this->lon=lon;
    this->lat=lat;
    connect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_showMe()));
    myScene->addItem(this);
    this->setZValue(z_level);
    this->isoNb=isoNb;
    this->pointIsoNb=pointIsoNb;
    this->routage=routage;
    debug="";
    //this->setAcceptHoverEvents(true);
    setData(0,7);
    show();
}
vlmPointGraphic::~vlmPointGraphic()
{
//    if(myScene!=NULL)
//        myScene->removeItem(this);
}
void vlmPointGraphic::slot_showMe()
{
    update();
}
QRectF vlmPointGraphic::boundingRect() const
{
    return QRectF(0,0,10,10);
}
void vlmPointGraphic::setEta(time_t eta)
{
    this->eta=eta;
    QDateTime tm;
    tm.setTimeSpec(Qt::UTC);
    tm.setTime_t(eta);
    if(this->debug.isEmpty())
        this->setToolTip("eta: "+tm.toString("dd MMM-hh:mm"));
    else
        this->setToolTip("eta: "+tm.toString("dd MMM-hh:mm")
                     +"<br>IsoNb->"+QString().setNum(isoNb)+" pt nb->"+QString().setNum(this->pointIsoNb)
                     +"<br>debug->"+this->debug);
}

//void  vlmPointGraphic::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
void vlmPointGraphic::drawWay()
{
    if(!routage->getShowIso())
        return;
    routage->setPivotPoint(this->isoNb,this->pointIsoNb);
    routage->slot_drawWay();
}

//void  vlmPointGraphic::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
//{
//    routage->eraseWay();
//}

void vlmPointGraphic::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{
    routage->showContextMenu(this->isoNb,this->pointIsoNb);
}
void vlmPointGraphic::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    int pi,pj;
    Util::computePos(proj,lat, lon, &pi, &pj);
    setPos(pi,pj);
}
