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
#include <QDebug>

#include "POI.h"
#include "Util.h"
#include "MainWindow.h"

//-------------------------------------------------------------------------------
POI::POI(QString name, POI_TYPE type, float lat, float lon,
                 Projection *proj, QWidget *ownerMeteotable, QWidget *parentWindow,
                 float wph,int tstamp,bool useTstamp)
    : QWidget(parentWindow)
{
    this->parent = parentWindow;
    this->owner = ownerMeteotable;
    this->name = name;
    this->lon = lon;
    this->lat = lat;
    this->wph=wph;
    this->timeStamp=tstamp;
    this->useTstamp=useTstamp;
    this->type=type;
    this->typeMask=1<<type;

    WPlon=WPlat=-1;
    isWp=false;

    this->proj = proj;


    countClick = 0;

    createWidget();
    paramChanged();

    connect(parentWindow, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()) );

    connect(this , SIGNAL(clearSelection()), parentWindow, SLOT(clearSelection()) );

    connect(this,SIGNAL(chgWP(float,float,float)),ownerMeteotable,SLOT(slotChgWP(float,float,float)));

    connect(this,SIGNAL(addPOI_list(POI*)),ownerMeteotable,SLOT(addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),ownerMeteotable,SLOT(delPOI_list(POI*)));

    connect(this,SIGNAL(editPOI(POI*)),ownerMeteotable,SLOT(slotEditPOI(POI *)));

    connect(this,SIGNAL(selectPOI(POI*)),ownerMeteotable,SLOT(slotPOIselected(POI*)));

    connect(this,SIGNAL(setGribDate(int)),ownerMeteotable,SLOT(slotSetGribDate(int)));

    connect(ownerMeteotable,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));
    connect(ownerMeteotable,SIGNAL(WPChanged(float,float)),this,SLOT(WPChanged(float,float)));

    ((MainWindow*)owner)->getBoatWP(&WPlat,&WPlon);

    setName(name);
    updateProjection();
    chkIsWP();
    setMouseTracking(true);
}

POI::~POI()
{
}

void POI::rmSignal(void)
{
    disconnect(parent, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()) );
    disconnect(this , SIGNAL(clearSelection()), parent, SLOT(clearSelection()) );

    disconnect(this,SIGNAL(chgWP(float,float,float)),owner,SLOT(slotChgWP(float,float,float)));

    disconnect(this,SIGNAL(addPOI_list(POI*)),owner,SLOT(addPOI_list(POI*)));
    disconnect(this,SIGNAL(delPOI_list(POI*)),owner,SLOT(delPOI_list(POI*)));

    disconnect(this,SIGNAL(editPOI(POI*)),owner,SLOT(slotEditPOI(POI *)));

    disconnect(this,SIGNAL(setGribDate(int)),owner,SLOT(slotSetGribDate(int)));

    disconnect(owner,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));
    disconnect(owner,SIGNAL(WPChanged(float,float)),this,SLOT(WPChanged(float,float)));
}

bool POI::event(QEvent * e)
{
    if(e->type() == QEvent::ToolTip && ((Terrain*)parent)->getHasCompassLine())
    {
        e->accept();
        return true;
    }
    return QWidget::event(e);
}

bool POI::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == label)
    {
        if (event->type() == QEvent::MouseMove)
        {
            event->ignore();
            return true;
        }
        else
            return false;

    }
    else
    {
        // pass the event on to the parent class
        //qWarning() << "Event for POI widget" << event->type();
        return QWidget::eventFilter(obj, event);
    }
}

//-------------------------------------------------------------------------------
void POI::createWidget()
{
    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    QGridLayout * layout = new QGridLayout();
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

    label->installEventFilter(this);
    label->setMouseTracking(true);

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
    setToolTip(tr("Marque") + " - " + getTypeStr() + " : "+str);
    adjustSize();
}

QString POI::getTypeStr(int index)
{
    QString type_str[3] = { "POI", "WP", "Balise" };
    return type_str[index];
}

//-------------------------------------------------------------------------------
void POI::updateProjection()
{
    Util::computePos(proj,lat, lon, &pi, &pj);
    int dy = height()/2;
    move(pi-3, pj-dy);
}

//-------------------------------------------------------------------------------
void  POI::paintEvent(QPaintEvent *)
{
    QPainter pnt(this);
    int dy = height()/2;

    pnt.fillRect(9,0, width()-10,height()-1, QBrush(bgcolor));
    QColor myColor;
    if(isWp)
        myColor=mwpColor;
    else
        switch(type)
        {
            case 0:
                myColor=poiColor;
                break;
            case 1:
                myColor=wpColor;
                break;
            case 2:
                myColor=baliseColor;
                break;
         }

    QPen pen(myColor);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.fillRect(0,dy-3,7,7, QBrush(myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt.setPen(pen);
    pnt.drawRect(9,0,width()-10,height()-1);
}

//-------------------------------------------------------------------------------
void  POI::enterEvent (QEvent *)
{    
    if(!((Terrain*)parent)->getHasCompassLine())
    {
        //enterCursor = cursor();
        setCursor(Qt::PointingHandCursor);
    }

}
//-------------------------------------------------------------------------------
void  POI::leaveEvent (QEvent *)
{
    //setCursor(enterCursor);
}

//-------------------------------------------------------------------------------
void  POI::mousePressEvent(QMouseEvent * e)
{
    e->ignore();
}
//-------------------------------------------------------------------------------
/*void  POI::mouseDoubleClickEvent(QMouseEvent *)
{
}*/

void POI::mouseMoveEvent (QMouseEvent * e)
{
    if(((Terrain*)parent)->getHasCompassLine())
        unsetCursor();

    e->ignore();
}

//-------------------------------------------------------------------------------
void POI::mouseReleaseEvent(QMouseEvent *e)
{
    if(((Terrain*)parent)->getHasCompassLine())
    {
        e->ignore();
        return;
    }

    if(e->x() < 0 || e->x()>width() || e->y() < 0 || e->y() > height())
    {
        e->ignore();
        return;
    }
    if (e->button() == Qt::LeftButton)
    {
        emit clearSelection();

        setCursor(Qt::BusyCursor);
        if (countClick == 0) {
            QTimer::singleShot(200, this, SLOT(timerClickEvent()));
        }
        countClick ++;
        if (countClick==2)
        {
            // Double Clic : Edit this Point Of Interest

            if(((MainWindow*)owner)->get_selPOI_instruction())
                emit selectPOI(this);
            else
                emit editPOI(this);
            countClick = 0;
        }
    }
}

void POI::contextMenuEvent(QContextMenuEvent * e)
{
    bool onlyLineOff = false;
    switch(((MainWindow*)owner)->getCompassMode(x()+e->x(),y()+e->y()))
    {
        case 0:
            /* not showing menu line, default text*/
            ac_compassLine->setText("Tirer un cap");
            ac_compassLine->setEnabled(false);
            break;
        case 1:
            /* showing text for compass line off*/
            ac_compassLine->setText("Arret du cap");
            ac_compassLine->setEnabled(true);
            onlyLineOff=true;
            break;
        case 2:
        case 3:
            ac_compassLine->setText("Tirer un cap");
            ac_compassLine->setEnabled(true);
            break;
        }
    if(onlyLineOff)
    {
        // is currentBoat locked ?
        ac_setWp->setEnabled(false);
        ac_setGribDate->setEnabled(false);
        ac_edit->setEnabled(false);
        ac_delPoi->setEnabled(false);
        ac_copy->setEnabled(false);
    }
    else
    {
        // is currentBoat locked ?
        ac_setWp->setEnabled(!((MainWindow*)owner)->getBoatLockStatus());
        ac_setGribDate->setEnabled(useTstamp);
        ac_edit->setEnabled(true);
        ac_delPoi->setEnabled(true);
        ac_copy->setEnabled(true);

    }
    popup->exec(QCursor::pos());
}

//-------------------------------------------------------------------------------
void  POI::timerClickEvent()
{
    if (countClick==1)
    {
        if(((MainWindow*)owner)->get_selPOI_instruction())
                emit selectPOI(this);
        else
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

    ac_setWp = new QAction(tr("Marque->WP"),popup);
    popup->addAction(ac_setWp);
    connect(ac_setWp,SIGNAL(triggered()),this,SLOT(slot_setWP()));

    ac_setGribDate = new QAction(tr("Set Date"),popup);
    popup->addAction(ac_setGribDate);
    connect(ac_setGribDate,SIGNAL(triggered()),this,SLOT(slot_setGribDate()));

    ac_compassLine = new QAction(tr("Tirer un cap"),popup);
    popup->addAction(ac_compassLine);
    connect(ac_compassLine,SIGNAL(triggered()),owner,SLOT(slotCompassLine()));
}

void POI::slot_editPOI()
{
    emit editPOI(this);
    chkIsWP();
}

void POI::slot_copy()
{
    Util::setWPClipboard(lat,lon,wph);
}

void POI::slot_setWP()
{
    emit chgWP(lat,lon,wph);
}

void POI::slot_setGribDate()
{
    if(useTstamp)
        emit setGribDate(timeStamp);
}

void POI::slotDelPoi()
{
    int rep = QMessageBox::question (this,
            tr("Détruire la marque"),
            tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
    if (rep == QMessageBox::Yes) {

        delPOI_list(this);
        rmSignal();
        close();
    }
}

void POI::paramChanged()
{
    poiColor = QColor(Util::getSetting("POI_color",QColor(Qt::black).name()).toString());
    mwpColor = QColor(Util::getSetting("Marque_WP_color",QColor(Qt::red).name()).toString());
    wpColor  = QColor(Util::getSetting("WP_color",QColor(Qt::darkYellow).name()).toString());
    baliseColor  = QColor(Util::getSetting("Balise_color",QColor(Qt::darkMagenta).name()).toString());
    update();
}

void POI::WPChanged(float tlat,float tlon)
{
    WPlat=tlat;
    WPlon=tlon;
    chkIsWP();
}

void POI::chkIsWP(void)
{
    if(compFloat(lat,WPlat) && compFloat(lon,WPlon))
    {
        if(!isWp)
        {
            isWp=true;
            update();
        }
    }
    else
    {
        if(isWp)
        {
            isWp=false;
            update();
        }
    }
}
