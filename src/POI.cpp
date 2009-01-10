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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/


#include <cassert>

#include <QTimer>
#include <QMessageBox>

#include "POI.h"
#include "Util.h"
#include "MainWindow.h"

//-------------------------------------------------------------------------------
POI::POI(QString name, float lon, float lat,
                 Projection *proj, QWidget *ownerMeteotable, QWidget *parentWindow,
                 int type, float wph,int tstamp,bool useTstamp)
    : QWidget(parentWindow)
{
    this->parent = parentWindow;
    this->owner = ownerMeteotable;
    this->name = name;
    this->lon = lon;
    this->lat = lat;
    this->wph=wph;
    this->type = type;
    this->timeStamp=tstamp;
    this->useTstamp=useTstamp;

    connect(this,SIGNAL(showMessage(QString)),ownerMeteotable,SLOT(slotShowMessage(QString)));

    setProjection(proj);
    createWidget();

    connect(proj, SIGNAL(projectionUpdated(Projection * )), this, SLOT(projectionUpdated(Projection *)) );
    connect(this, SIGNAL(signalOpenMeteotablePOI(POI*)),
                            ownerMeteotable, SLOT(slotOpenMeteotablePOI(POI*)));
    
    connect(this,SIGNAL(chgWP(float,float,float)),ownerMeteotable,SLOT(slotChgWP(float,float,float)));

    connect(this,SIGNAL(addPOI_list(POI*)),ownerMeteotable,SLOT(addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),ownerMeteotable,SLOT(delPOI_list(POI*)));

    connect(this,SIGNAL(editPOI(POI*)),ownerMeteotable,SLOT(slotEditPOI(POI *)));

    setName(name);
}

POI::~POI()
{
    //printf("delete POI_Editor\n");
}

//-------------------------------------------------------------------------------
void POI::createWidget()
{
    countClick = 0;

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(10,0,2,0);
    layout->setSpacing(0);

    label = new QLabel(name, this);
    label->setFont(QFont("Helvetica",9));

    QPalette p;
    p.setBrush(QPalette::Active, QPalette::WindowText, fgcolor);
    p.setBrush(QPalette::Inactive, QPalette::WindowText, fgcolor);
    label->setPalette(p);
    label->setAlignment(Qt::AlignHCenter);

    layout->addWidget(label, 0,0, Qt::AlignHCenter|Qt::AlignVCenter);
    this->setLayout(layout);
    setAutoFillBackground (false);

    createPopUpMenu();
}

//-------------------------------------------------------------------------------
void POI::setName(QString name)
{
    this->name=name;
    QString str;
    QDateTime tm;
    tm.setTimeSpec(Qt::UTC);
    tm.setTime_t(getTimeStamp());
    if(getUseTimeStamp())
        str=tm.toString("dd-hh:mm");
    else
        str=name;

    label->setText(str);
    setToolTip(tr("Point d'intérêt : ")+str);
    adjustSize();
}

//-------------------------------------------------------------------------------
void POI::setProjection( Projection *proj)
{
    this->proj = proj;
    if (proj->isPointVisible(lon, lat)) {      // tour du monde ?
        proj->map2screen(lon, lat, &pi, &pj);
    }
    else if (proj->isPointVisible(lon-360, lat)) {
        proj->map2screen(lon-360, lat, &pi, &pj);
    }
    else {
        proj->map2screen(lon+360, lat, &pi, &pj);
    }

    int dy = height()/2;
    move(pi-3, pj-dy);
}

//-------------------------------------------------------------------------------
void POI::projectionUpdated(Projection * )
{
    setProjection(proj);
}

//-------------------------------------------------------------------------------
void  POI::paintEvent(QPaintEvent *)
{
    QPainter pnt(this);
    int dy = height()/2;

    pnt.fillRect(9,0, width()-10,height()-1, QBrush(bgcolor));

    QPen pen(Qt::black);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.fillRect(0,dy-3,7,7, QBrush(Qt::black));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt.setPen(pen);
    pnt.drawRect(9,0,width()-10,height()-1);

}

//-------------------------------------------------------------------------------
void  POI::enterEvent (QEvent *)
{
    enterCursor = cursor();
    setCursor(Qt::PointingHandCursor);

}
//-------------------------------------------------------------------------------
void  POI::leaveEvent (QEvent *)
{
    setCursor(enterCursor);
}

//-------------------------------------------------------------------------------
void  POI::mousePressEvent(QMouseEvent *)
{
}
//-------------------------------------------------------------------------------
void  POI::mouseDoubleClickEvent(QMouseEvent *)
{
}
//-------------------------------------------------------------------------------
void  POI::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        setCursor(Qt::BusyCursor);
        if (countClick == 0) {
            QTimer::singleShot(200, this, SLOT(timerClickEvent()));
        }
        countClick ++;
        if (countClick==2)
        {
            // Double Clic : Edit this Point Of Interest
            emit editPOI(this);
            countClick = 0;
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        // is currentBoat locked ?
        ac_setWp->setEnabled(!((MainWindow*)owner)->getBoatLockStatus());
        // Right clic : Edit this Point Of Interest
        popup->exec(QCursor::pos());
        e->accept();
    }
}
//-------------------------------------------------------------------------------
void  POI::timerClickEvent()
{
    if (countClick==1)
    {
        // Single clic : Meteotable for this Point Of Interest
        //emit signalOpenMeteotablePOI(this);
        slot_editPOI();

    }
    countClick = 0;
}

void POI::doChgWP(float lat,float lon, float wph)
{
    emit chgWP(lat,lon,wph);
}

void POI::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_edit = new QAction(tr("Editer"),popup);
    popup->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),this,SLOT(slot_editPOI()));

    ac_delPoi = new QAction(tr("Supprimer"),popup);
    popup->addAction(ac_delPoi);
    connect(ac_delPoi,SIGNAL(triggered()),this,SLOT(slotDelPoi()));

    ac_copy = new QAction(tr("Copier"),popup);
    popup->addAction(ac_copy);
    connect(ac_copy,SIGNAL(triggered()),this,SLOT(slot_copy()));

    ac_meteo = new QAction(tr("MeteoTable"),popup);
    popup->addAction(ac_meteo);
    connect(ac_meteo,SIGNAL(triggered()),this,SLOT(slot_meteoPOI()));

    ac_setWp = new QAction(tr("POI->WP"),popup);
    popup->addAction(ac_setWp);
    connect(ac_setWp,SIGNAL(triggered()),this,SLOT(slot_setWP()));
}

void POI::slot_editPOI()
{
    emit editPOI(this);
}

void POI::slot_copy()
{
    Util::setWPClipboard(lat,lon,wph);
}

void POI::slot_meteoPOI()
{
    emit signalOpenMeteotablePOI(this);
}

void POI::slot_setWP()
{
    emit chgWP(lat,lon,wph);
}

void POI::slotDelPoi()
{
    int rep = QMessageBox::question (this,
            tr("Détruire le POI"),
            tr("La destruction d'un point d'intérêt est définitive.\n\nEtes-vous sûr ?"),
            QMessageBox::Yes | QMessageBox::No);
    if (rep == QMessageBox::Yes) {

        delPOI_list(this);
        close();
    }
}
