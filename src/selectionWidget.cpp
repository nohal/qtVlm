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
#include <QMenu>

#include "selectionWidget.h"
#include "settings.h"
#include "Projection.h"
#include "mycentralwidget.h"
#include "orthoSegment.h"

selectionWidget::selectionWidget(myCentralWidget *centralWidget, Projection * proj, QGraphicsScene * myScene) : QGraphicsWidget() {
    this->centralWidget=centralWidget;
    this->proj=proj;
    seg=new orthoSegment(proj,myScene,Z_VALUE_SELECTION);

    setZValue(Z_VALUE_SELECTION);
    setData(0,SELECTION_WTYPE);

    popup = new QMenu(centralWidget->getMainWindow());
    connect(this->popup,SIGNAL(aboutToShow()),centralWidget,SLOT(slot_resetGestures()));
    connect(this->popup,SIGNAL(aboutToHide()),centralWidget,SLOT(slot_resetGestures()));
    connect(this->popup,SIGNAL(aboutToShow()),this,SLOT(slot_protect()));
    connect(this->popup,SIGNAL(aboutToHide()),this,SLOT(slot_unprotect()));
    isProtected=false;

    ac_delAllPOIs = new QAction(tr("Effacer toutes les marques"),popup);
    ac_delSelPOIs = new QAction(tr("Effacer les marques..."),popup);
    ac_notSimpSelPOIs = new QAction(tr("Rendre toutes les marques non-simplifiables"),popup);
    ac_simpSelPOIs = new QAction(tr("Rendre toutes les marques simplifiables"),popup);
    ac_dwnldZygrib = new QAction(tr("Download with ZyGrib"),popup);
    ac_mailSailDoc = new QAction(tr("Mail SailsDoc"),popup);
    ac_zoomSelection = new QAction(tr("Zoom on selection"),popup);

    popup->addAction(ac_delAllPOIs);
    popup->addAction(ac_delSelPOIs);
    popup->addSeparator();
    popup->addAction(ac_notSimpSelPOIs);
    popup->addAction(ac_simpSelPOIs);
    popup->addSeparator();
    popup->addAction(ac_dwnldZygrib);
    popup->addAction(ac_mailSailDoc);
    popup->addSeparator();
    popup->addAction(ac_zoomSelection);

    connect(ac_delAllPOIs, SIGNAL(triggered()), centralWidget, SLOT(slot_delAllPOIs()));
    connect(ac_delSelPOIs, SIGNAL(triggered()), centralWidget, SLOT(slot_delSelPOIs()));
    connect(ac_notSimpSelPOIs, SIGNAL(triggered()), centralWidget, SLOT(slot_notSimpAllPOIs()));
    connect(ac_simpSelPOIs, SIGNAL(triggered()), centralWidget, SLOT(slot_simpAllPOIs()));
    connect(ac_dwnldZygrib,SIGNAL(triggered()),centralWidget, SLOT(slot_fileLoad_GRIB()));
    connect(ac_mailSailDoc,SIGNAL(triggered()),centralWidget, SLOT(slotLoadSailsDocGrib()));
    connect(ac_zoomSelection,SIGNAL(triggered()),centralWidget,  SLOT(slot_Zoom_Sel()));

    xa=xb=ya=yb=0;
    width=height=0;
    selecting=false;
    showOrthodromie   = Settings::getSetting("showOrthodromie", false).toBool();
    hide();
}

void selectionWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent * e) {
    int compassMode = centralWidget->getCompassMode(e->scenePos().x(),e->scenePos().y());

    if(compassMode==0)
    {
        ac_delAllPOIs->setEnabled(true);
        ac_delSelPOIs->setEnabled(true);
        ac_simpSelPOIs->setEnabled(true);
        ac_notSimpSelPOIs->setEnabled(true);
    }
    else
    {
        ac_delAllPOIs->setEnabled(false);
        ac_delSelPOIs->setEnabled(false);
        ac_simpSelPOIs->setEnabled(false);
        ac_notSimpSelPOIs->setEnabled(false);
    }

    popup->exec(QCursor::pos());
}
void selectionWidget::slot_protect()
{
    this->isProtected=true;
    xa=old_xa;
    ya=old_ya;
    xb=old_xb;
    yb=old_yb;
    updateSize();
    update();
    show();
    if(showOrthodromie)
    {
        seg->initSegment(xa,ya,xb,yb);
        seg->show();
    }
}
void selectionWidget::slot_unprotect()
{
    this->isProtected=false;
}

void selectionWidget::startSelection(int start_x,int start_y)
{
    if(isProtected) return;
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

#if 0
    double xE,xW,yN,yS;
    double sX;
    double PY;

    proj->screen2map(xa<xb?xa:xb,ya<yb?ya:yb,xW,yN);
    proj->screen2map(xa<xb?xb:xa,ya<yb?yb:ya,xE,yS);

    sX=W/fabs(xE-xW);

    y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);

#endif

#if 0
    yb=xb*(((double)proj->getH())/((double)proj->getW()));
#endif
    updateSize();

    update();

    if(showOrthodromie)
        seg->moveSegment(xb,yb);
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
    old_xa=xa;
    old_ya=ya;
    old_xb=xb;
    old_yb=yb;
    selecting=false;
}

void selectionWidget::clearSelection(void)
{
    if(isProtected) return;
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
    if(xa==xb || ya==yb)
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
