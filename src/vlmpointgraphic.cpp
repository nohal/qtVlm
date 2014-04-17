#include "vlmpointgraphic.h"
#include "Util.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "routage.h"

vlmPointGraphic::vlmPointGraphic(ROUTAGE * routage,int isoNb, int pointIsoNb,double lon, double lat, Projection * proj,  myCentralWidget * mcp,int z_level) : QGraphicsWidget()
{
    this->proj=proj;
    this->myScene=mcp->getScene();
    this->mcp=mcp;
    this->lon=lon;
    this->lat=lat;
    connect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_showMe()));
    myScene->addItem(this);
    this->setZValue(z_level);
    this->isoNb=isoNb;
    this->pointIsoNb=pointIsoNb;
    this->routage=routage;
    debug="";
    setData(0,7);
    show();
}
QVariant vlmPointGraphic::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change==ItemSelectedHasChanged)
    {
        //prepareGeometryChange();
        if(!isSelected())
            routage->eraseWay();
        else
        {
            mcp->clearOtherSelected(this);
            routage->setPivotPoint(this->isoNb,this->pointIsoNb);
            routage->slot_drawWay();
        }
    }
    return QGraphicsWidget::itemChange(change,value);
}
vlmPointGraphic::~vlmPointGraphic()
{
    setSelected(false);
//    if(myScene!=NULL)
//        myScene->removeItem(this);
}
void vlmPointGraphic::slot_showMe()
{
    int pi,pj;
    Util::computePos(proj,lat, lon, &pi, &pj);
    setPos(pi,pj);
}
QRectF vlmPointGraphic::boundingRect() const
{
    return QRectF(-10,-10,20,20);
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
void vlmPointGraphic::slot_updateTip(int i,int n, QString t)
{
    if(i!=isoNb) return;
    if(n!=pointIsoNb) return;
    setEta(eta);
    QString newtip=this->toolTip()+"<br>"+t;
    newtip=newtip.replace(" ","&nbsp;");
    setToolTip(newtip);
}

void  vlmPointGraphic::setAcceptHover()
{
    this->setAcceptHoverEvents(true);
    this->setFlag(ItemIsSelectable,true);
}

void  vlmPointGraphic::hoverLeaveEvent ( QGraphicsSceneHoverEvent * )
{
    routage->eraseWay();
}
void  vlmPointGraphic::hoverEnterEvent ( QGraphicsSceneHoverEvent * )
{
    if(!routage->getShowIso())
        return;
    routage->setPivotPoint(this->isoNb,this->pointIsoNb);
    routage->slot_drawWay();
}

void vlmPointGraphic::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    routage->showContextMenu(this->isoNb,this->pointIsoNb);
}
void vlmPointGraphic::paint(QPainter *, const QStyleOptionGraphicsItem * , QWidget * )
{
}
