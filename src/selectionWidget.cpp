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
#include <QPainter>

#include "selectionWidget.h"
#include "settings.h"
#include "Projection.h"
#include "mycentralwidget.h"
#include "orthoSegment.h"

selectionWidget::selectionWidget(Projection * proj, QGraphicsScene * myScene) : QGraphicsWidget()
{
    this->proj=proj;
    seg=new orthoSegment(proj,myScene,Z_VALUE_SELECTION);

    setZValue(Z_VALUE_SELECTION);
    setData(0,SELECTION_WTYPE);

    xa=xb=ya=yb=0;
    width=height=0;
    selecting=false;
    showOrthodromie   = Settings::getSetting("showOrthodromie", false).toBool();
    hide();
}

void selectionWidget::startSelection(int start_x,int start_y)
{
    selecting=true;
    show();
    xa=xb=start_x;
    ya=yb=start_y;
    updateSize();

    update();

    if(showOrthodromie)
    {
        seg->initSegment(xa,ya,xb,yb);
        seg->show();
    }
}

bool selectionWidget::tryMoving(int mouse_x,int mouse_y)
{
    if(!selecting)
        return false;
    xb=mouse_x;
    yb=mouse_y;
    updateSize();

    update();

    if(showOrthodromie)
        seg->moveSegment(mouse_x,mouse_y);
    return true;
}

void selectionWidget::updateSize(void)
{
    prepareGeometryChange();
    /* position */
    int pos_x = qMin(xa,xb);
    int pos_y = qMin(ya,yb);
    setPos(pos_x,pos_y);
    /* size */
    width=qMax(qAbs(xa-xb),1);
    height=qMax(qAbs(ya-yb),1);
}

void selectionWidget::slot_setDrawOrthodromie(bool b)
{
    qWarning() << "Set ortho: " << b;
    if (showOrthodromie != b)
    {
        showOrthodromie = b;
        Settings::setSetting("showOrthodromie", b);
        update();
        if(showOrthodromie)
            seg->initSegment(xa,ya,xb,yb);
        else
            seg->hideSegment();
    }
}

void selectionWidget::stopSelection(void)
{
    selecting=false;

}

void selectionWidget::clearSelection(void)
{
    stopSelection();
    hide();
    if(showOrthodromie)
        seg->hideSegment();
}

bool selectionWidget::getZoneWithSens(double * x0, double * y0, double * x1, double * y1)
{
    if(!this->isVisible())
        return false;

    if(!x0 || !y0 || !x1 || !y1)
        return false;

    proj->screen2map(xa,ya,x0,y0);
    proj->screen2map(xb,yb,x1,y1);
    return true;
}
bool selectionWidget::getZone(double * x0, double * y0, double * x1, double * y1)
{
    if(!this->isVisible())
        return false;

    if(!x0 || !y0 || !x1 || !y1)
        return false;

    proj->screen2map(xa<xb?xa:xb,ya<yb?ya:yb,x0,y0);
    proj->screen2map(xa<xb?xb:xa,ya<yb?yb:ya,x1,y1);
    return true;
}

QPainterPath selectionWidget::shape() const
{
    QPainterPath path;
    path.addRect(0,0,width,height);
    return path;
}

QRectF selectionWidget::boundingRect() const
{
    return QRectF(0,0,width,height);
}

void selectionWidget::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{

    int r = 100; /* fond */
    int v = 180; /* trait */

    pnt->setPen(QColor(v,v,v));
    pnt->setBrush(QColor(r,r,r, 80));
    pnt->drawRect(0, 0, width, height);
}
