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
    size=1;
    this->proj=proj;
    this->roundedEnd=roundedEnd;
    myScene->addItem(this);
    this->setZValue(z_level);
    this->alsoDrawLoxo=false;
    isOrtho=true;
    setFocusPolicy(Qt::NoFocus);
    hide();
}

QRectF orthoSegment::boundingRect() const
{
    return QRectF(0,0,4*size,4*size);
}

void orthoSegment::initSegment(double xa,double ya,double xb, double yb)
{
    this->xa=xa;
    this->ya=ya;
    this->xb=xb;
    this->yb=yb;
    show();
    updateSizeAndPosition();    
    setFocusPolicy(Qt::NoFocus);
    if(xa==xb && ya==yb) hide();
}

void orthoSegment::moveSegment(double x,double y)
{
    this->xb=x;
    this->yb=y;
    updateSizeAndPosition();
    update();
}

void orthoSegment::updateSizeAndPosition(void)
{
    prepareGeometryChange();
    size=qMax(qAbs(xa-xb),qAbs(ya-yb));
    setPos(xa-size,ya-size);
}

void orthoSegment::hideSegment(void)
{
    hide();
}

void orthoSegment::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    pnt->setPen(linePen);
    pnt->setRenderHint(QPainter::Antialiasing);
    if(!isOrtho)
    {        
        pnt->drawLine(xa-x(),ya-y(),xb-x(),yb-y());
    }
    else
    {
        draw_orthoSegment(pnt,xa,ya,xb,yb);
    }
    if(alsoDrawLoxo && isOrtho)
        pnt->drawLine(xa-x(),ya-y(),xb-x(),yb-y());
    if(roundedEnd)
    {
        double w=linePen.widthF();
        linePen.setWidthF(w*3);
        pnt->setPen(linePen);
        pnt->drawPoint(xb-x(),yb-y());
        linePen.setWidthF(w);
        pnt->setPen(linePen);
    }
}

void orthoSegment::draw_orthoSegment(QPainter * pnt,double i0,double j0, double i1, double j1, int recurs)
{
    if (recurs > 10) // this is bugging under win :100)
    {
        //qWarning() << "Stop recursing";
        return;
    }

    if (abs(i0-i1) > 10)
    {
        double xm, ym,x0,y0,x1,y1;
        int im,jm;
        Orthodromie *ortho;

        proj->screen2map(i0, j0,&x0,&y0);
        proj->screen2map(i1, j1,&x1,&y1);

        ortho = new Orthodromie(x0, y0, x1, y1);
        ortho->getMidPoint(&xm, &ym);
        delete ortho;
        ortho = NULL;

        xm *= 180.0/M_PI;
        ym *= 180.0/M_PI;
        while (ym > 90)
            ym -= 180;
        while (ym < -90)
            ym += 180;
        proj->map2screen(xm, ym, &im, &jm);
        draw_orthoSegment(pnt, i0,j0, im,jm, recurs+1);
        draw_orthoSegment(pnt, im,jm, i1,j1, recurs+1);
    }
    else {
        pnt->drawLine(i0-x(),j0-y(), i1-x(),j1-y());
    }

}
