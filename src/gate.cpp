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
#include <QMessageBox>
#include <QGridLayout>

#include "gate.h"
#include "Util.h"

#define FONT_SIZE 11

gate::gate(QString name,float lat_1, float lon_1,float lat_2, float lon_2,
           Projection *proj, QWidget *ownerMeteotable,
           QWidget *parentWindow, QString color) : QWidget(parentWindow)
{
    this->parent = parentWindow;
    this->owner = ownerMeteotable;
    this->name = name;
    this->lon_1 = lon_1;
    this->lat_1 = lat_1;
    this->lon_2 = lon_2;
    this->lat_2 = lat_2;
    this->proj=proj;
    if(color.isEmpty())
        this->myColor=QColor(Qt::black);
    else
        this->myColor=QColor(color);

    notDrawing=false;
    isIn=false;

    createPopUpMenu();
    paramChanged();

    connect(parentWindow, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()));
    connect(ownerMeteotable,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));
    connect(this,SIGNAL(delGate_list(gate*)),owner,SLOT(delGate_list(gate*)));
#warning mettre un lien directe vers gate editor ?
    //    connect(this,SIGNAL(editGate(gate*)),owner,SLOT(slotEditGate(gate*)));

    setName(name);
    updateProjection();

    setMouseTracking (true);

    //show();
}

gate::~gate()
{
}

void gate::rmSignal(void)
{
    disconnect(parent, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()) );
    disconnect(owner,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));
    disconnect(this,SIGNAL(delGate_list(gate*)),owner,SLOT(delGate_list(gate*)));
    disconnect(this,SIGNAL(editGate(gate*)),owner,SLOT(slotEditGate(gate*)));
}

void gate::updateProjection(void)
{
    int pi,pj,li,lj;
    int a,b,w,h;

    notDrawing=false;

    Util::computePos(proj, lat_1, lon_1, &pi, &pj);

    if(!proj->isInBounderies(pi,pj))
    {
        notDrawing=true;
        return;
    }

    Util::computePos(proj, lat_2, lon_2, &li, &lj);
    if(!proj->isInBounderies(li,lj))
    {
        notDrawing=true;
        return;
    }

    /*find upper corner*/
    a=pi<li?pi:li;
    a-=4;
    b=pj<lj?pj:lj;
    b-=4;
    /*find size */
    w=pi-li;
    w=w<0?-w:w;
    w+=9;
    h=pj-lj;
    h=h<0?-h:h;
    h+=9;

    QFontMetrics fm(QFont("Helvetica",FONT_SIZE));
    l_w=fm.width(name);
    l_h=fm.height();

    if(w<l_w)
    {
        a=a-(l_w-w)/2;
        w=l_w;
        l_x=0;
    }
    else
        l_x=(w-l_w)/2;

    if(h<l_h)
    {
        b=b-(l_h-h)/2;
        h=l_h;
        l_y=0;
    }
    else
        l_y=(h-l_h)/2;

    i_1=pi-a-3;
    i_2=li-a-3;
    j_1=pj-b-3;
    j_2=lj-b-3;

    setGeometry(a,b,w,h);
}

void gate::paramChanged(void)
{

}

void gate::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_editGate = new QAction(tr("Editer"),popup);
    popup->addAction(ac_editGate);
    connect(ac_editGate,SIGNAL(triggered()),this,SLOT(slot_editGate()));

    ac_delGate = new QAction(tr("Supprimer"),popup);
    popup->addAction(ac_delGate);
    connect(ac_delGate,SIGNAL(triggered()),this,SLOT(slot_delGate()));

    ac_copyGate = new QAction(tr("Copier"),popup);
    popup->addAction(ac_copyGate);
    connect(ac_copyGate,SIGNAL(triggered()),this,SLOT(slot_copyGate()));
}

void gate::slot_editGate(void)
{
    emit editGate(this);
}

void gate::slot_delGate(void)
{
    int rep = QMessageBox::question (this,
            tr("Detruire la Porte"),
            tr("La destruction d'une porte est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
    if (rep == QMessageBox::Yes) {
        delGate_list(this);
        rmSignal();
        close();
    }
}

void gate::slot_copyGate(void)
{

}

void gate::setName(QString name)
{
    this->name=name;
    setToolTip(tr("Porte: ")+name);
    updateProjection();
}

void  gate::paintEvent(QPaintEvent *)
{
    if(notDrawing)
        return;
    QPainter pnt(this);
    QPen pen(myColor);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.setFont(QFont("Helvetica",FONT_SIZE));
    pnt.fillRect(i_1,j_1,7,7, QBrush(myColor));
    pnt.fillRect(i_2,j_2,7,7, QBrush(myColor));
    //pnt.drawPie(i_1,j_1,7,7, 0,5760);
    //pnt.drawPie(i_2,j_2,7,7, 0,5760);

    pnt.drawLine(i_1+3,j_1+3,i_2+3,j_2+3);

    pnt.fillRect(l_x,l_y, l_w,l_h, QColor(255,255,255,150));

    pen.setColor(Qt::black);
    pnt.setPen(pen);
    pnt.drawText(l_x,l_y+l_h-2,name);

    //qWarning() << "Gate paint done";
}

bool gate::validateCoord(int x, int y)
{
    /* txt zone */
    if(x>=l_x && x<=(l_x+l_w) && y>=l_y && y<=(l_y+l_h))
    {
        qWarning() << "Validate ok: " << x << "," << y  << " (" << l_x << " " << l_y << " " << l_w << " " << l_h;
        return true;
    }
    else
    {
        qWarning() << "Validate ko: " << x << "," << y  << " (" << l_x << " " << l_y << " " << l_w << " " << l_h;
        return false;
    }
}

//-------------------------------------------------------------------------------
/*void gate::enterEvent (QEvent *)
{
    QPoint mouse_pos = QWidget::mapFromGlobal(QCursor::pos());
    if(validateCoord(mouse_pose->x(), mouse_pos->y()))
    {
        enterCursor = cursor();
        setCursor(Qt::PointingHandCursor);
    }
}

void gate::leaveEvent (QEvent *)
{
    setCursor(enterCursor);
}
*/

void gate::mouseMoveEvent (QMouseEvent * e)
{
    if (validateCoord(e->x(),e->y()) && !isIn)
    {
        enterCursor = cursor();
        setCursor(Qt::PointingHandCursor);
        isIn=true;
    }
    else
        if(isIn)
        {
            setCursor(enterCursor);
            isIn=false;
        }
}

//-------------------------------------------------------------------------------


void  gate::mouseReleaseEvent(QMouseEvent *e)
{
    if (!validateCoord(e->x(),e->y()))
    {
        e->ignore();
        return;
    }
    if (e->button() == Qt::LeftButton)
    {
        emit editGate(this);
        e->accept();
    }
}

void gate::contextMenuEvent(QContextMenuEvent *)
{
    popup->exec(QCursor::pos());
}
