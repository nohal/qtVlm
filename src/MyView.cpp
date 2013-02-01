#include "MyView.h"
#include <QDebug>
#include <QScrollBar>

MyView::MyView(Projection *proj, myScene *scene, myCentralWidget * mcp) :
    QGraphicsView(scene,mcp)
{
    this->scene=scene;
    this->proj=proj;
    this->setTransformationAnchor(QGraphicsView::NoAnchor);
    this->setRenderHints(QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform |
                         QPainter::HighQualityAntialiasing);
    viewPix=new QGraphicsPixmapItem();
    viewPix->setZValue(100);
    viewPix->setPos(0,0);
    paning=false;
    px=0;
    py=0;
    backPix=new QGraphicsPixmapItem();
    backPix->setPos(0,0);
    backPix->setZValue(90);
    scene->addItem(backPix);
    scene->addItem(viewPix);
    backPix->hide();
    viewPix->hide();
}
void MyView::startPaning(QGraphicsSceneMouseEvent *e)
{
    px=e->scenePos().x();
    py=e->scenePos().y();
    QPixmap pix(proj->getW(),proj->getH());
    pix.fill(Qt::blue);
    backPix->setPixmap(pix);
    backPix->setPos(0,0);
    QPainter pnt(&pix);
    pnt.setRenderHints(QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform |
                         QPainter::HighQualityAntialiasing);
    this->render(&pnt);
    pnt.end();
    viewPix->setPixmap(pix);
    viewPix->setPos(0,0);
    backPix->show();
    viewPix->show();
    paning=true;
}
void MyView::pane(int x, int y)
{
    viewPix->setPos(x-px,y-py);
   // backPix->setPos(px-x,py-y);
}
void MyView::stopPaning(int x, int y)
{
    if(px-x!=0 && py-y!=0)
    {
        QRectF r(QPointF(0,0),QSize(proj->getW(),proj->getH()));
        r.translate(px-x,py-y);
        proj->setCentralPixel(r.center());
    }
    else
    {
        backPix->hide();
        viewPix->hide();
    }
    paning=false;
}
void MyView::hideViewPix()
{
    viewPix->setScale(1);
    backPix->hide();
    viewPix->hide();
    paning=false;
}

void MyView::myScale(double scale, double lon, double lat)
{
#if 0
    this->setUpdatesEnabled(false);
    this->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    this->setSceneRect(0,0,proj->getW(),proj->getH());
    this->resetTransform();
    double X1,Y1,X2,Y2;
    proj->map2screenDouble(lon,lat,&X1,&Y1);
    this->scale(scale,scale);
    QPointF P=mapToScene(X1,Y1);
    X2=P.x();
    Y2=P.y();
    this->setUpdatesEnabled(true);
    this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    this->translate(X2-X1,Y2-Y1);
#else
    QPixmap pix(proj->getW(),proj->getH());
    if(!viewPix->isVisible())
    {
        pix.fill(Qt::blue);
        backPix->setPixmap(pix);
        QPainter pnt(&pix);
        pnt.setRenderHints(QPainter::Antialiasing |
                             QPainter::SmoothPixmapTransform |
                             QPainter::HighQualityAntialiasing);
        this->render(&pnt);
        pnt.end();
        viewPix->setPixmap(pix);
        viewPix->setPos(0,0);
        backPix->show();
        viewPix->show();
    }
    double X1,Y1;
    proj->map2screenDouble(lon,lat,&X1,&Y1);
    viewPix->setTransformOriginPoint(X1,Y1);
    viewPix->setScale(scale);
#endif
}
