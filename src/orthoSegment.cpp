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
#include "orthoSegment.h"
#include "Projection.h"

orthoSegment::orthoSegment(Projection * proj, QGraphicsScene * myScene,int z_level,bool roundedEnd) : QGraphicsWidget()
{
    xa=xb=ya=yb=0;
    this->proj=proj;
    this->roundedEnd=roundedEnd;
    myScene->addItem(this);
    this->setZValue(z_level);
    this->alsoDrawLoxo=false;
    isOrtho=true;
    hide();
    connect(this->proj,SIGNAL(projectionUpdated()),this,SLOT(slot_update()));
}

QRectF orthoSegment::boundingRect() const
{
    return myPath.boundingRect();
}
QPainterPath orthoSegment::shape() const
{
    return myPath;
}

void orthoSegment::initSegment(double xa,double ya,double xb, double yb)
{
    this->xa=xa;
    this->ya=ya;
    this->xb=xb;
    this->yb=yb;
    if(xa==xb && ya==yb) hide();
    else show();
}

void orthoSegment::moveSegment(double x,double y)
{
    this->xb=x;
    this->yb=y;
    update();
}


void orthoSegment::hideSegment(void)
{
    hide();
}

void orthoSegment::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    QPainterPath path;
    pnt->setPen(linePen);
    pnt->setRenderHint(QPainter::Antialiasing);
    if(!isOrtho)
    {        
        QPolygonF p;
        p.append(QPointF(xa-x(),ya-y()));
        p.append(QPointF(xb-x(),yb-y()));
        path.addPolygon(p);
    }
    else
    {
        draw_orthoSegment(pnt,xa,ya,xb,yb,0,&path);
    }
    if(alsoDrawLoxo && isOrtho)
    {
        QPolygonF p;
        p.append(QPointF(xa-x(),ya-y()));
        p.append(QPointF(xb-x(),yb-y()));
        path.addPolygon(p);
    }
    if(roundedEnd)
    {
        double w=linePen.widthF();
        linePen.setWidthF(w*3);
        pnt->setPen(linePen);
        pnt->drawPoint(xb-x(),yb-y());
        linePen.setWidthF(w);
        pnt->setPen(linePen);
    }
    prepareGeometryChange();
    myPath=path;
    pnt->drawPath(myPath);
}

void orthoSegment::draw_orthoSegment(QPainter * pnt,double i0,double j0, double i1, double j1, int recurs, QPainterPath * path)
{
    if (recurs > 1000)
    {
        qWarning() << "Stop recursing";
        return;
    }
    QLineF line(i0,j0,i1,j1);
    if (line.length()>10)
    {
        double xm, ym,x0,y0,x1,y1;
        double im,jm;

        proj->screen2mapDouble(i0, j0,&x0,&y0);
        proj->screen2mapDouble(i1, j1,&x1,&y1);
        Orthodromie ortho(x0,y0,x1,y1);
        ortho.getMidPoint(&xm, &ym);

        xm *= 180.0/M_PI;
        ym *= 180.0/M_PI;
        while (ym > 90)
            ym -= 180;
        while (ym < -90)
            ym += 180;
        proj->map2screenDouble(xm, ym, &im, &jm);
        draw_orthoSegment(pnt, i0,j0, im,jm, recurs+1,path);
        draw_orthoSegment(pnt, im,jm, i1,j1, recurs+1,path);
    }
    else
    {
        QPolygonF p;
        p.append(QPointF(i0-x(),j0-y()));
        p.append(QPointF(i1-x(),j1-y()));
        path->addPolygon(p);
    }
}
void orthoSegment::slot_update()
{
    update();
}
