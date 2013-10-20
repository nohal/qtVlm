#ifndef MAGNIFIER_H
#define MAGNIFIER_H

#include <QGraphicsPixmapItem>
#include "class_list.h"

#include <QMenu>

class Magnifier : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit Magnifier(myCentralWidget *parent);
    ~Magnifier();
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
signals:
public slots:
    void slot_closeMe();
    void slot_changeSize();
    void slot_changeZoom();
private:
    myCentralWidget * parent;
    Projection * myProj;
    QPixmap imgEarth;
    QPixmap imgMask;
    void drawMe();
    void drawMask();
    int width,height;
    QSize sizeMagnifier;
    QMenu * popup;
    QAction *changeSize;
    QAction *changeZoom;
    QAction *closeMe;
};

#endif // MAGNIFIER_H
