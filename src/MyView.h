#ifndef MYVIEW_H
#define MYVIEW_H

#include <QGraphicsView>
#include "Projection.h"
#include "mycentralwidget.h"
#include "class_list.h"
class MyView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyView(Projection * proj, myScene * scene,myCentralWidget * mcw);
    void myScale(const double &scale, const double &lon, const double &lat, const bool &screenCoord=false);
    void startPaning(const QGraphicsSceneMouseEvent *e);
    void pane(const int &x, const int &y);
    void stopPaning(const int &x, const int &y);
    bool isPaning(){return paning;}
    void hideViewPix();
signals:
    
public slots:
private:
    Projection * proj;
    myScene * scene;
    myCentralWidget * mcw;
    QGraphicsPixmapItem * viewPix;
    QGraphicsPixmapItem * backPix;
    bool paning,pinching;
    int px,py;
};

#endif // MYVIEW_H
