#ifndef ROUTEINFO_H
#define ROUTEINFO_H

#include <QDialog>
#include "ui_routeInfo.h"
#include "mycentralwidget.h"
#include "route.h"

class routeInfo : public QDialog, public Ui::routeInfo
{
    Q_OBJECT
    
public:
    routeInfo(myCentralWidget *parent, ROUTE *route);
    ~routeInfo();
    void setValues(double twd, double tws, double twa, double bs, double hdg, double cnm, double dnm, bool engineUsed, bool south, double cog, double sog, double cs, double cd);
protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
private:
    ROUTE *route;
    void drawWindArrowWithBarbs(QPainter &pnt,
                                int i, int j, double vkn, double ang,
                                bool south);
    void drawTransformedLine( QPainter &pnt,
                              double si, double co,int di, int dj, int i,int j, int k,int l);
    void drawPetiteBarbule(QPainter &pnt, bool south,
                double si, double co, int di, int dj, int b);
    void drawGrandeBarbule(QPainter &pnt,  bool south,
                double si, double co, int di, int dj, int b);
    void drawTriangle(QPainter &pnt, bool south,
                double si, double co, int di, int dj, int b);
    QPainterPath drawBoat;
    myCentralWidget *parent;
};

#endif // ROUTEINFO_H
