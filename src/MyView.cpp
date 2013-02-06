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
void MyView::startPaning(const QGraphicsSceneMouseEvent *e)
{
    if(scene->getPinching()) return;
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
void MyView::pane(const int &x, const int &y)
{
    viewPix->setPos(x-px,y-py);
}
void MyView::stopPaning(const int &x, const int &y)
{
    if(scene->getPinching())
    {
        paning=false;
        return;
    }
    if(px-x!=0 && py-y!=0 && !scene->getPinching())
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

void MyView::myScale(const double &scale, const double &lon, const double &lat)
{
    if(!viewPix->isVisible())
    {
        QPixmap pix(proj->getW(),proj->getH());
        pix.fill(Qt::blue);
        backPix->setPixmap(pix);
        QPainter pnt(&pix);
        pnt.setRenderHints(QPainter::Antialiasing |
                             QPainter::SmoothPixmapTransform |
                             QPainter::HighQualityAntialiasing);
        this->render(&pnt);
        pnt.end();
        viewPix->resetTransform();
        viewPix->setPixmap(pix);
        viewPix->setPos(0,0);
        backPix->show();
        viewPix->show();
    }
    double X1,Y1;
    proj->map2screenDouble(lon,lat,&X1,&Y1);
    viewPix->resetTransform();
    viewPix->setTransformOriginPoint(X1,Y1);
    viewPix->setScale(scale);
}
