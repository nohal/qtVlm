
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>

#include "Magnifier.h"
#include "settings.h"
#include "ToolBar.h"
#include "boat.h"
#include "opponentBoat.h"
#include "boatVLM.h"
#include "route.h"
#include "Projection.h"
#include "GshhsReader.h"
#include "mycentralwidget.h"


Magnifier::Magnifier(myCentralWidget *parent)
{
    this->parent=parent;
    Projection * proj=parent->getProj();
    drawMask();
    this->setFlags(QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIgnoresTransformations |
                   QGraphicsItem::ItemSendsScenePositionChanges);
    QGraphicsDropShadowEffect * effect=new QGraphicsDropShadowEffect();
    QColor color=effect->color();
    color.setAlpha(100);
    effect->setColor(color);
    this->setGraphicsEffect(effect);
    this->popup=new QMenu();
    changeSize = new QAction(tr("Change magnifier size"),popup);
    popup->addAction(changeSize);
    connect(changeSize,SIGNAL(triggered()),this,SLOT(slot_changeSize()));
    changeZoom = new QAction(tr("Change magnifier zoom"),popup);
    popup->addAction(changeZoom);
    connect(changeZoom,SIGNAL(triggered()),this,SLOT(slot_changeZoom()));
    closeMe = new QAction(tr("Close magnifier"),popup);
    popup->addAction(closeMe);
    connect(closeMe,SIGNAL(triggered()),this,SLOT(slot_closeMe()));




    int zoom=qMin(10,Settings::getSetting("magnifierZoom","3").toInt());
    if(zoom*proj->getScale()>scalemax)
        zoom=floor((double)scalemax/proj->getScale());
    width=proj->getW()*zoom;
    height=proj->getH()*zoom;
    myProj=new Projection(width,height,proj->getCX(),proj->getCY());
    myProj->setScale(proj->getScale()*zoom);
    GshhsReader * reader = parent->get_gshhsReader();
    reader->setProj(myProj);
    imgEarth= QPixmap(width,height);
    QColor b=Qt::blue;
    b=b.lighter();
    imgEarth.fill(b);

    QPainter pnt1(&imgEarth);
    pnt1.setRenderHint(QPainter::Antialiasing, true);
    pnt1.setRenderHint(QPainter::SmoothPixmapTransform, true);
    pnt1.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor landColor = Settings::getSetting("landColor", QColor(200,200,120)).value<QColor>();
    reader->drawContinents(pnt1, myProj, Qt::transparent, landColor);
    QPen seaBordersPen;
    seaBordersPen.setColor(Settings::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>());
    seaBordersPen.setWidthF(Settings::getSetting("seaBordersLineWidth", 1.8).toDouble());
    pnt1.setPen(seaBordersPen);
    reader->drawSeaBorders(pnt1, myProj);
    if(parent->getSelectedBoat())
    {
        if(parent->getSelectedBoat()->get_boatType()==BOAT_VLM)
        {
            if(!parent->get_shOpp_st())
            {
                opponentList * oppList=parent->getOppList();
                for (int n=0;n<oppList->getList()->count();++n)
                {
                    oppList->getList()->at(n)->drawOnMagnifier(myProj,&pnt1);
                    oppList->getList()->at(n)->getTraceDrawing()->drawInMagnifier(&pnt1,myProj);
                }
            }
        }
        parent->getSelectedBoat()->drawOnMagnifier(myProj,&pnt1);
        QList<ROUTE*> routeList=parent->getRouteList();
        foreach(ROUTE * route,routeList)
        {
            route->getLine()->drawInMagnifier(&pnt1,myProj);
        }

        if(parent->getSelectedBoat()->get_boatType()==BOAT_VLM)
        {
            QList<vlmLine *>gates=((boatVLM *)parent->getSelectedBoat())->getGates();
            foreach(vlmLine * gate,gates)
            {
                gate->drawInMagnifier(&pnt1,myProj);
            }
        }
        parent->getSelectedBoat()->getTraceDrawing()->drawInMagnifier(&pnt1,myProj);
    }
    pnt1.end();
    reader->setProj(proj);
    this->setPos(proj->getW()/2.0-sizeMagnifier.width()/2.0,proj->getH()/2.0-sizeMagnifier.height()/2.0);
    this->setZValue(Z_VALUE_MAGNIFIER);
    parent->getScene()->addItem(this);
    this->drawMe();
    show();
//    reader->clearCells();
}
Magnifier::~Magnifier()
{
    delete myProj;
}
void Magnifier::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    popup->exec(QCursor::pos());
}
QVariant Magnifier::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change==QGraphicsItem::ItemPositionHasChanged)
    {
        double X=this->pos().x();
        double Y=this->pos().y();
        X=qBound(0.0-sizeMagnifier.width()/2.0,X,(double)parent->getProj()->getW()-sizeMagnifier.width()/2.0);
        Y=qBound(0.0-sizeMagnifier.height()/2.0,Y,(double)parent->getProj()->getH()-sizeMagnifier.height()/2.0);
        setPos(X,Y);
        drawMe();
    }
    return value;
}
void Magnifier::drawMask()
{
    int c=12-Settings::getSetting("magnifierSize","5").toInt();

    sizeMagnifier=QSize(parent->getProj()->getW()/c,parent->getProj()->getW()/c);
    imgMask=QPixmap(sizeMagnifier);
    imgMask.fill(Qt::transparent);
    QPainter pnt(&imgMask);
    pnt.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(QRectF(QPointF(0,0),sizeMagnifier));
    QColor color=QColor(179,206,246);
    QRadialGradient radialGrad(QPointF(sizeMagnifier.width()/2.0,
                                       sizeMagnifier.height()/2.0),
                                       sizeMagnifier.width()/2.0);
    //radialGrad.setSpread(QGradient::ReflectSpread);
    radialGrad.setColorAt(0, Qt::transparent);
    radialGrad.setColorAt(0.7, Qt::transparent);
    color.setAlpha(150);
    radialGrad.setColorAt(0.9, color);
    color.setAlpha(200);
    radialGrad.setColorAt(0.95, color);
    radialGrad.setColorAt(1.0,Qt::white);
    pnt.setCompositionMode(QPainter::CompositionMode_Source);
    pnt.fillPath(path,QBrush(radialGrad));
    pnt.end();
}
void Magnifier::drawMe()
{
    QImage img=QImage(sizeMagnifier,QImage::Format_ARGB32);
    double X,Y;
    double lon,lat;
    X=pos().x()+sizeMagnifier.width()/2.0;
    Y=pos().y()+sizeMagnifier.height()/2.0;
    parent->getProj()->screen2mapDouble(X,Y,&lon,&lat);
    myProj->map2screenDouble(lon,lat,&X,&Y);
    QPoint p(qRound(X),qRound(Y));
    QRect r=QRect(p,sizeMagnifier);
    r.moveCenter(p);
    QPainter pnt(&img);
    pnt.setRenderHint(QPainter::Antialiasing);
    pnt.setRenderHint(QPainter::SmoothPixmapTransform);
    pnt.setRenderHint(QPainter::HighQualityAntialiasing);
    pnt.setCompositionMode(QPainter::CompositionMode_Source);
    QPainterPath path;
    path.addEllipse(QRectF(QPointF(0,0),sizeMagnifier));
    pnt.drawPixmap(QPoint(0,0),imgEarth,r);
    pnt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    //pnt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    pnt.drawPixmap(QPoint(0,0),imgMask);
    pnt.setCompositionMode(QPainter::CompositionMode_Source);
    path.addRect(-1,-1,sizeMagnifier.width()+2,sizeMagnifier.height()+2);
    path.setFillRule(Qt::OddEvenFill);
    pnt.fillPath(path,Qt::transparent);
    QColor color=Qt::black;
    color.setAlpha(100);
    QPen pen(color);
    pen.setWidth(2);
    QPainterPath path_bis;
    QRectF rr(QPointF(0,0),sizeMagnifier);
    path_bis.addEllipse(rr.center(),rr.width()/2-1,rr.height()/2-1);
    pnt.strokePath(path_bis,pen);
    pnt.end();
    QPixmap pix;
    pix.convertFromImage(img);
    setPixmap(pix);
}
void Magnifier::slot_closeMe()
{
    parent->get_toolBar()->magnify->setChecked(false);
    parent->setMagnifier(NULL);
    this->deleteLater();
}
void Magnifier::slot_changeSize()
{
    bool ok=false;
    int currentSize=Settings::getSetting("magnifierSize","5").toInt();
    int newSize=QInputDialog::getInt(0,tr("New magnifier size"),tr("magnifier size"),
                                     currentSize,1,10,1,&ok);
    if(!ok || currentSize==newSize) return;
    Settings::setSetting("magnifierSize",newSize);
    this->drawMask();
    this->setPos(parent->getProj()->getW()/2.0-sizeMagnifier.width()/2.0,parent->getProj()->getH()/2.0-sizeMagnifier.height()/2.0);
    this->drawMe();
}
void Magnifier::slot_changeZoom()
{
    bool ok=false;
    int currentZoom=Settings::getSetting("magnifierZoom","3").toInt();
    int newZoom=QInputDialog::getInt(0,tr("New magnifier zoom"),tr("magnifier zoom"),
                                     currentZoom,2,10,1,&ok);
    if(!ok || currentZoom==newZoom) return;
    Settings::setSetting("magnifierZoom",newZoom);
    parent->setMagnifier(NULL);
    parent->slot_magnify();
    parent->getMagnifier()->setPos(this->pos());
    this->deleteLater();
}
