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
signals:
    
public slots:
private:
    Projection * proj;
    myScene * scene;
};

#endif // MYVIEW_H
