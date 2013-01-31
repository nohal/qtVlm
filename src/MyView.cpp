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
}
void MyView::myScale(double scale, double lon, double lat)
{
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
    return;
}
