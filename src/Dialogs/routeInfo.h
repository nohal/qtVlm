/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

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
    void setValues(double twd, double tws, double twa, double bs, double hdg, double cnm, double dnm, bool engineUsed, bool south, double cog, double sog, double cs, double cd, double wh, double wd, bool night, double wsh);
public slots:
    void slot_screenResize();
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
