/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

#include <QDebug>

#include "Orthodromie.h"
#include "vlmLine.h"

vlmLine::vlmLine(Projection * proj, QGraphicsScene * myScene,int z_level) : QGraphicsWidget()
{
    poly.resize(0);
    this->proj=proj;
    this->myScene=myScene;
    connect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_showMe()));
    myScene->addItem(this);
    this->setZValue(z_level);
    this->linePen=QPen(Qt::red);
    linePen.setBrush(Qt::red);
    linePen.setWidth(2);
    setFocusPolicy(Qt::NoFocus);
    pt_color=QColor(Qt::red);
    mode=VLMLINE_LINE_MODE;
    show();
}

vlmLine::~vlmLine()
{
    myScene->removeItem(this);
}

QRectF vlmLine::boundingRect() const
{
    return poly.boundingRect();
}

void vlmLine::addPoint(float lat,float lon)
{
    vlmPoint point;
    point.lat=lat;
    point.lon=lon;
    line.append(point);
}

void vlmLine::setPoly(QList<vlmPoint> & poly)
{
    line=poly;
}

void vlmLine::slot_showMe()
{
    calculatePoly();
    show();
    update();
}

void vlmLine::setLineMode()
{
    mode = VLMLINE_LINE_MODE;
    update();
}

void vlmLine::setPointMode(QColor pt_color)
{
    mode = VLMLINE_POINT_MODE;
    this->pt_color = pt_color;
    update();
}

void vlmLine::calculatePoly(void)
{
    int n=0;
    prepareGeometryChange();
    poly.resize(0);
    int X,Y;
    if(line.count()>1)
    {
        QListIterator<vlmPoint> i (line);
        while(i.hasNext())
        {
            vlmPoint worldPoint=i.next();
            Util::computePos(proj,worldPoint.lat, worldPoint.lon, &X, &Y);
            poly.putPoints(n,1,X-(int)x(),Y-(int)y());
            n++;
        }
    }
}

void vlmLine::deleteAll()
{
    while(line.count()!=0)
        line.removeFirst();
    calculatePoly();
    update();
}

void vlmLine::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    pnt->setPen(linePen);
    switch(mode)
    {
        case VLMLINE_LINE_MODE:
            pnt->drawPolyline(poly);
            break;
        case VLMLINE_POINT_MODE:
        {
            if(poly.isEmpty()) break;
            int nbVac=12*Util::getSetting("trace_length",12).toInt();
            int step=Util::getSetting("trace_step",60/5-1).toInt()+1;

            int x0=poly.point(0).x();
            int y0=poly.point(0).y();

            for(int i=1;i<poly.count() && i<nbVac;i++)
            {
                int x,y;
                if(i%step) /* not taking all vac*/
                    continue;
                x=poly.point(i).x();
                y=poly.point(i).y();
                pnt->fillRect(x-1,y-1,3,3,pt_color);
                pnt->drawLine(x0,y0,x,y);
                x0=x;
                y0=y;
            }
            break;
        }
    }
}
