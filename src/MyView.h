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
    void myScale(double scale, double lon, double lat);
    void startPaning(QGraphicsSceneMouseEvent *e);
    void pane(int x,int y);
    void stopPaning(int x, int y);
    bool isPaning(){return paning;}
    void hideViewPix();
signals:
    
public slots:
private:
    Projection * proj;
    myScene * scene;
    QGraphicsPixmapItem * viewPix;
    QGraphicsPixmapItem * backPix;
    bool paning;
    int px,py;
};

#endif // MYVIEW_H
