#ifndef VLMPOINTGRAPHIC_H
#define VLMPOINTGRAPHIC_H
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QPainter>
#include <QObject>
#include "Projection.h"
#include "class_list.h"

class vlmPointGraphic : QGraphicsWidget
{ Q_OBJECT
public:
    vlmPointGraphic(ROUTAGE * routage,int isoNb, int pointIsoNb, float lon, float lat, Projection * proj, QGraphicsScene * myScene,int z_level);
    ~vlmPointGraphic();
    void setIsoNb(int n){this->isoNb=n;}
    void setPointIsoNb(int n){this->pointIsoNb=n;}
    void setEta(time_t eta);
    void shown(bool b){if (b) show(); else hide();}
    void setDebug(QString f){this->debug=f;}
    void createPopupMenu();
protected:
    void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
    QRectF boundingRect() const;
//    void  hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
//    void  hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent * e);

public slots:
    void drawWay(void);
    void slot_showMe(void);
private:
    QGraphicsScene * myScene;
    Projection * proj;
    ROUTAGE * routage;
    int isoNb;
    int pointIsoNb;
    float lon,lat;
    time_t eta;
    QString debug;
};

#endif // VLMPOINTGRAPHIC_H
