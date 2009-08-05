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

#include <iostream>
#include <cassert>

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>

#include "Terrain.h"
#include "Orthodromie.h"
#include "POI.h"

//---------------------------------------------------------
// Constructeur
//---------------------------------------------------------
Terrain::Terrain(QWidget *parent, Projection *proj_)
    : QWidget(parent)
{
    quality = 0;
    drawingMap=false;
    selX0 = selY0 = 0;
    selX1 = selY1 = 0;
    isResizing = true;
    timerResize = new QTimer(this);
    assert(timerResize);
    timerResize->setSingleShot(true);
    connect(timerResize, SIGNAL(timeout()), this, SLOT(slotTimerResize()));
    connect(this,SIGNAL(getRaceVacLen(boatAccount *,int*)),parent,SLOT(getRaceVacLen(boatAccount *,int*)));

    //---------------------------------------------------------------------
    showCountriesBorders  = Util::getSetting("showCountriesBorders", true).toBool();
    showOrthodromie   = Util::getSetting("showOrthodromie", false).toBool();
    showRivers   = Util::getSetting("showRivers", false).toBool();
    showCitiesNamesLevel = Util::getSetting("showCitiesNamesLevel", 0).toInt();
    showCountriesNames = Util::getSetting("showCountriesNames", false).toBool();
    showWindColorMap  = Util::getSetting("showWindColorMap", true).toBool();

    colorMapSmooth = Util::getSetting("colorMapSmooth", true).toBool();
    showWindArrows  = Util::getSetting("showWindArrows", true).toBool();
    showBarbules = Util::getSetting("showBarbules", true).toBool();
    //----------------------------------------------------------------------------

    imgEarth = NULL;

    imgWind  = NULL;
    imgAll   = NULL;
    isEarthMapValid = false;
    isWindMapValid  = false;
    mustRedraw = true;

    proj = proj_;
    gshhsReader = NULL;
    grib = new Grib();
    assert(grib);
    gisReader = new GisReader();
    assert(gisReader);

    isSelectionZoneEnCours = false;
    setPalette(QPalette(backgroundColor));
    setAutoFillBackground(true);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    updateGraphicsParameters();

    gribDateDialog = new dialog_gribDate();
}

//-------------------------------------------
void Terrain::updateGraphicsParameters()
{
    backgroundColor  = Util::getSetting("backgroundColor", QColor(0,0,45)).value<QColor>();
    seaColor  = Util::getSetting("seaColor", QColor(50,50,150)).value<QColor>();
    landColor = Util::getSetting("landColor", QColor(200,200,120)).value<QColor>();

    seaBordersPen.setColor(Util::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>());
    seaBordersPen.setWidthF(Util::getSetting("seaBordersLineWidth", 1.8).toDouble());

    boundariesPen.setColor(Util::getSetting("boundariesLineColor", QColor(40,40,40)).value<QColor>());
    boundariesPen.setWidthF(Util::getSetting("boundariesLineWidth", 1.4).toDouble());

    riversPen.setColor(Util::getSetting("riversLineColor", QColor(50,50,150)).value<QColor>());
    riversPen.setWidthF(Util::getSetting("riversLineWidth", 1.0).toDouble());

    int v = 180;
    selectColor     = QColor(v,v,v);

    isEarthMapValid = false;
    mustRedraw = true;
    update();
}

//---------------------------------------------------------
// Cartes GSHHS
//---------------------------------------------------------
void Terrain::setGSHHS_map(GshhsReader *map)
{
    gshhsReader = map;
    isEarthMapValid = false;
}

//-------------------------------------------------------
bool Terrain::draw_GSHHSandGRIB(QPainter &pntGlobal)
{
    bool longJob = false;
    if (mustRedraw  ||  !isEarthMapValid  || !isWindMapValid)
    {
        pleaseWait = true;
        QCursor oldcursor = cursor();
        setCursor(Qt::WaitCursor);

            if (imgAll != NULL) {
                delete imgAll;
                imgAll = NULL;
            }
            imgAll = new QPixmap(proj->getW(), proj->getH());
            assert(imgAll);
            QPainter pnt(imgAll);
            pnt.setRenderHint(QPainter::Antialiasing, true);

            //===================================================
            // Dessin du fond de carte et des données GRIB
            //===================================================
            if (!isEarthMapValid)
            {
                if (imgEarth != NULL) {
                    delete imgEarth;
                    imgEarth = NULL;
                }

                imgEarth = new QPixmap(proj->getW(), proj->getH());
                assert(imgEarth);

                if (gshhsReader != NULL)
                {
                    QPainter pnt1(imgEarth);
                    pnt1.setRenderHint(QPainter::Antialiasing, true);
                    gshhsReader->drawBackground(pnt1, proj, seaColor, backgroundColor);
                    gshhsReader->drawContinents(pnt1, proj, seaColor, landColor);
                }
            }
            pnt.drawPixmap(0,0, *imgEarth);

            //===================================================
            // Dessin des données GRIB
            //===================================================

            grib->draw_WIND_Color(pnt, proj, colorMapSmooth,showWindColorMap,showWindArrows,showBarbules);

            //===================================================
            // Dessin des bordures et frontières
            //===================================================
            if (gshhsReader != NULL)
            {
                pnt.setPen(seaBordersPen);
                gshhsReader->drawSeaBorders(pnt, proj);

                if (showCountriesBorders) {
                    pnt.setPen(boundariesPen);
                    gshhsReader->drawBoundaries(pnt, proj);
                }
                if (showRivers) {
                    pnt.setPen(riversPen);
                    gshhsReader->drawRivers(pnt, proj);
                }
            }

            if (showCountriesNames) {
                gisReader->drawCountriesNames(pnt, proj);
            }
            if (showCitiesNamesLevel > 0) {
                gisReader->drawCitiesNames(pnt, proj, showCitiesNamesLevel);
            }

            //===================================================

            grib->drawCartouche(pnt);

            //===================================================
            isEarthMapValid = true;
            isWindMapValid = true;
            mustRedraw = false;

        setCursor(oldcursor);
        pleaseWait = false;
        longJob = true;
    }

    // Recopie l'image complète
    pntGlobal.drawPixmap(0,0, *imgAll);
    return longJob;
}

//-------------------------------------------------------
// Tracé récursif
void Terrain::draw_OrthodromieSegment(QPainter &pnt,
                            float x0,float y0, float x1,float y1,
                            int recurs
                            )
{
    if (recurs > 10) // this is bugging under win :100)
        return;
    Orthodromie *ortho;
    int i0,j0, i1,j1, im,jm;
    float eps = 0.5;
    if (y0 > 90-eps) y0 = 90-eps;
    if (y0 <-90+eps) y0 =-90+eps;
    if (y1 > 90-eps) y1 = 90-eps;
    if (y1 <-90+eps) y1 =-90+eps;

    if (fabs(x0-x1)>180)  // il faut faire le tour du monde par derrière
    {
        if (x0 < x1) {
            draw_OrthodromieSegment(pnt, x1-360,y1, x0,y0, recurs+1);
            draw_OrthodromieSegment(pnt, x0+360,y0, x1,y1, recurs+1);
        }
        else {
            draw_OrthodromieSegment(pnt, x0-360,y0, x1,y1, recurs+1);
            draw_OrthodromieSegment(pnt, x1+360,y1, x0,y0, recurs+1);
        }
    }
    else
    {
        proj->map2screen(x0, y0, &i0, &j0);
        proj->map2screen(x1, y1, &i1, &j1);
        if (abs(i0-i1) > 10)
        {
            float xm, ym;

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
            //printf("%5d: (%5d %5d) (%5d %5d) (%5d %5d)      %f %f   %f %f\n",recurs,i0,j0, im,jm, i1,j1,x0,y0,x1,y1);
            draw_OrthodromieSegment(pnt, x0,y0, xm,ym, recurs+1);
            draw_OrthodromieSegment(pnt, xm,ym, x1,y1, recurs+1);
        }
        else {
                pnt.drawLine(i0,j0, i1,j1);
        }
    }
}

//-------------------------------------------------------
void Terrain::draw_Orthodromie(QPainter &pnt)
{
    draw_OrthodromieSegment(pnt, selX0, selY0, selX1, selY1);
}

void Terrain::drawBoats(QPainter &pnt)
{
    //qWarning() << "Drawing boats";
    QListIterator<boatAccount*> i (*boat_list);

    QPen cur_pen=pnt.pen();
    QPen penLine1(QColor(Qt::black),1,Qt::DotLine);
    penLine1.setWidthF(1.6);
    QPen penLine2(QColor(Qt::darkMagenta),1,Qt::DotLine);
    penLine2.setWidthF(1.6);

    QColor myColor = QColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString());
    QColor selColor = QColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString());
    QColor curColor;

    QPen penLine3(myColor,1);
    penLine3.setWidthF(1);
    QPen penLine4(selColor,1);
    penLine4.setWidthF(1);

    int estime_param,estime_param_2;
    int type=Util::getSetting("estimeType",0).toInt();
    float estime;
    switch(type)
    {
        case 0: /* time */
            estime_param = Util::getSetting("estimeTime",60).toInt();
            break;
        case 1: /* nb vac */
            estime_param = Util::getSetting("estimeVac",10).toInt();
            break;
        default: /* dist */
            estime_param = Util::getSetting("estimeLen",100).toInt();
            break;
    }


    while(i.hasNext())
    {
        boatAccount * boat = i.next();

        /*can we draw something?*/
        if(boat->isUpdating() || !boat->getStatus())
            continue;

        /* draw estime */
        if(boat->getIsSelected() || boat->getForceEstime())
        {
            float lat,lon,tmp_lat,tmp_lon,WPLat,WPLon;
            lat=boat->getLat();
            lon=boat->getLon();
            WPLat=boat->getWPLat();
            WPLon=boat->getWPLon();

            switch(type)
            {
                case 0: /* time */
                    estime = (float)((float)(estime_param/60))*boat->getSpeed();
                    //qWarning() << "Estime (time) " << boat->getLogin() << " " << estime<< "(" << estime_param << "," << boat->getSpeed() << ")";
                    break;
                case 1: /* nb vac */
                    emit getRaceVacLen(boat,&estime_param_2);
                    estime = (float)((((float)(estime_param*estime_param_2))/3660)*boat->getSpeed());
                    /*qWarning() << "Estime (vac) " << boat->getLogin() << " " << estime << "("
                            << estime_param << "," << estime_param_2
                            << "," << boat->getSpeed() << ")";*/
                    break;
                default: /* dist */
                    estime = estime_param;
                    //qWarning() << "Estime (len) " << boat->getLogin() << " " << estime;
                    break;
            }

            Util::getCoordFromDistanceAngle(lat,lon,estime,boat->getHeading(),&tmp_lat,&tmp_lon);
            pnt.setPen(penLine1);
            draw_OrthodromieSegment(pnt, lon,lat,tmp_lon,tmp_lat);
            /* draw ortho to wp */
            if(WPLat != 0 && WPLon != 0)
            {
                pnt.setPen(penLine2);
                draw_OrthodromieSegment(pnt, lon,lat,WPLon,WPLat);
            }
        }

        /*draw trace*/
        QList<position*> * trace = boat->getTrace();
        int x,y,x0=0,y0=0;
        if(boat->getIsSelected())
        {
            pnt.setPen(penLine4);
            curColor=selColor;
        }
        else
        {
            pnt.setPen(penLine3);
            curColor=myColor;
        }

        int nbVac=12*Util::getSetting("trace_length",12).toInt();
        int step=Util::getSetting("trace_step",60/5-1).toInt()+1;

        for(int i=0;i<trace->size() && i<nbVac;i++)
        {
            if(i%step) /* not taking all vac*/
                continue;
            Util::computePos(proj,trace->at(i)->lat,trace->at(i)->lon,&x,&y);
            if(!proj->isInBounderies(x,y))
                break;
            pnt.fillRect(x-1,y-1,3,3,curColor);
            if(i!=0)
                pnt.drawLine(x0,y0,x,y);
            x0=x;
            y0=y;
        }
    }
    pnt.setPen(cur_pen);
    //qWarning() << "Drawing boats done";
}

void Terrain::drawOpponents(QPainter &pnt)
{
    //qWarning() << "Drawing Opp";
    if(Util::getSetting("opp_trace","1")=="0")
        return;

    //qWarning() << "Drawing st2";

    int nbVac=12*Util::getSetting("trace_length",12).toInt();
    int step=Util::getSetting("trace_step",60/5-1).toInt()+1;

    QColor myColor;
    QPen cur_pen=pnt.pen();
    QPen penLine(QColor(myColor),1);
    penLine.setWidthF(1);

    QList<opponent*> * oppLst = opponents->getList();
    QListIterator<opponent*> i (*oppLst);
    while(i.hasNext())
    {
        opponent * opp = i.next();
        myColor = opp->getColor();
        penLine.setColor(myColor);
        QList<position*> * trace = opp->getTrace();
        int x,y,x0=0,y0=0;
        pnt.setPen(penLine);
        for(int i=0;i<trace->size() && i<nbVac;i++)
        {
            if(i%step) /* not taking all vac*/
                continue;
            Util::computePos(proj,trace->at(i)->lat,trace->at(i)->lon,&x,&y);
            if(!proj->isInBounderies(x,y))
                break;
            pnt.fillRect(x-1,y-1,3,3,myColor);
            if(i!=0)
                pnt.drawLine(x0,y0,x,y);
            x0=x;
            y0=y;
        }
    }
    pnt.setPen(cur_pen);
    //qWarning() << "Drawing Opp done";
}

//---------------------------------------------------------
void Terrain::indicateWaitingMap()
{
    pleaseWait = true;   // Affiche un message d'attente
    update();
    QCoreApplication::processEvents();
}

//-------------------------------------------------------
void Terrain::setProjection()
{
    indicateWaitingMap();
    isEarthMapValid = false;
    isWindMapValid = false;
    update();
    emit projectionUpdated();
}

//=========================================================
void Terrain::setDrawRivers(bool b) {
    if (showRivers != b) {
        showRivers = b;
        Util::setSetting("showRivers", b);
        mustRedraw = true;
        update();
    }
}

//-------------------------------------------------------
void Terrain::setDrawCountriesBorders(bool b) {
    if (showCountriesBorders != b) {
        showCountriesBorders = b;
        Util::setSetting("showCountriesBorders", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawOrthodromie(bool b) {
    if (showOrthodromie != b) {
        showOrthodromie = b;
        Util::setSetting("showOrthodromie", b);
        mustRedraw = false;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setCountriesNames(bool b) {
    if (showCountriesNames != b) {
        showCountriesNames = b;
        Util::setSetting("showCountriesNames", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setCitiesNamesLevel  (int level) {
    if (showCitiesNamesLevel != level) {
        showCitiesNamesLevel = level;
        Util::setSetting("showCitiesNamesLevel", level);
        mustRedraw = true;
        update();
    }
}

//-------------------------------------------------------
void Terrain::setMapQuality(int q) {
    indicateWaitingMap();
    if (quality != q) {
        if (gshhsReader == NULL)
            return;
        quality = q;
        pleaseWait = true;
        //update();
        QCursor oldcursor = cursor();
        setCursor(Qt::WaitCursor);
            gshhsReader->setUserPreferredQuality(q);
            isEarthMapValid = false;
            update();
        setCursor(oldcursor);
        pleaseWait = false;
    }
}

//-------------------------------------------------------
void Terrain::setDrawWindColors (bool b) {
    if (showWindColorMap != b) {
        showWindColorMap = b;
        Util::setSetting("showWindColorMap", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setColorMapSmooth (bool b) {
    if (colorMapSmooth != b) {
        colorMapSmooth = b;
        Util::setSetting("colorMapSmooth", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawWindArrows (bool b) {
    if (showWindArrows != b) {
        showWindArrows = b;
        Util::setSetting("showWindArrows", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setBarbules (bool b) {
    if (showBarbules != b) {
        showBarbules = b;
        Util::setSetting("showBarbules", b);
        mustRedraw = true;
        update();
    }
}

//=================================================================
bool  Terrain::getSelectedRectangle(double *x0, double *y0, double *x1, double *y1)
{
    if (selX0!=selX1 && selY0!=selY1)
    {   // Si nécessaire, réoriente le rectangle sélectionné
        if (selX0 > selX1) {
            *x0 = selX1;
            *x1 = selX0;
        }
        else {
            *x0 = selX0;
            *x1 = selX1;
        }
        if (selY0 > selY1) {
            *y0 = selY0;
            *y1 = selY1;
        }
        else {
            *y0 = selY1;
            *y1 = selY0;
        }
        return true;
    }
    else {
        return false;
    }
}
//---------------------------------------------------------
bool  Terrain::getSelectedLine(float *x0, float *y0, float *x1, float *y1)
{
    *x0 = selX0;
    *x1 = selX1;
    *y0 = selY0;
    *y1 = selY1;
    return true;
}

//---------------------------------------------------------
// Grib
//---------------------------------------------------------
void Terrain::loadGribFile(QString fileName, bool zoom)
{
    indicateWaitingMap();
    grib->loadGribFile(fileName);
    isSelectionZoneEnCours = false;
    selX0 = selY0 = 0;
    selX1 = selY1 = 0;
    isEarthMapValid = false;
    isWindMapValid = false;
    // Zoom sur la zone couverte par le fichier GRIB
    if (zoom) {
        zoomOnGribFile();
    }
    update();
}
//---------------------------------------------------------
void Terrain::zoomOnGribFile()
{
    if (grib != NULL && grib->isOk()) {
        double x0,y0, x1,y1, mh, mv;
        if (grib->getZoneExtension(&x0,&y0, &x1,&y1))
        {
            mh = fabs(x0-x1)*0.05;
            mv = fabs(y0-y1)*0.05;
            proj->updateZoneSelected(x0-mh,y0-mv, x1+mh,y1+mv);
            setProjection();
        }
    }
}
//---------------------------------------------------------
void Terrain::slotMustRedraw()
{
    indicateWaitingMap();
    isEarthMapValid = false;
    isWindMapValid = false;
    update();
}
//---------------------------------------------------------
void Terrain::setCurrentDate(time_t t)
{
    if (grib->getCurrentDate() != t)
    {
        indicateWaitingMap();
        grib->setCurrentDate(t);
        isWindMapValid = false;
        update();
    }
}

time_t Terrain::getCurrentDate(void)
{
    if(grib->isOk())
        return grib->getCurrentDate();
    return 0;
}

void Terrain::showGribDate_dialog(void)
{
    if(gribDateDialog && grib->isOk())
    {
        time_t res;
        gribDateDialog->showDialog(grib->getCurrentDate(),grib->getListDates(),&res);
        setCurrentDate(res);
    }
}



//---------------------------------------------------------
// Events
//---------------------------------------------------------

void Terrain::keyModif(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        setCursor(Qt::SizeAllCursor);
    }
    else if (e->modifiers() == Qt::ShiftModifier) {
        setCursor(Qt::UpArrowCursor);
    }
    else {
        setCursor(Qt::CrossCursor);
    }
}

void  Terrain::keyPressEvent (QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Minus:
        case Qt::Key_M:
            slot_Zoom_Out();
            setCursor(Qt::CrossCursor);
            break;
        case Qt::Key_Plus:
        case Qt::Key_P:
            slot_Zoom_In();
            setCursor(Qt::CrossCursor);
            break;
        case Qt::Key_Up:
            slot_Go_Up();
            break;
        case Qt::Key_Down:
            slot_Go_Down();
            break;
        case Qt::Key_Left:
            slot_Go_Left();
            break;
        case Qt::Key_Right:
            slot_Go_Right();
            break;
        case Qt::Key_Escape:
            emit POI_selectAborted(NULL);
            break;
        default:
            keyModif(e);
    }
}

void  Terrain::keyReleaseEvent (QKeyEvent *e)
{
    keyModif(e);
}
//---------------------------------------------------------
void Terrain::enterEvent (QEvent * /*e*/) {
//printf("enter\n");
    enterCursor = cursor();
    setCursor(Qt::CrossCursor);
}
//---------------------------------------------------------
void Terrain::leaveEvent (QEvent * /*e*/) {
//printf("leave\n");
    setCursor(enterCursor);
}
//---------------------------------------------------------
void Terrain::mousePressEvent (QMouseEvent * e) {
//printf("press\n");
    if (e->button() == Qt::LeftButton)
    {
        // Début de sélection de zone rectangulaire
        double x,y;
        isSelectionZoneEnCours = true;
        proj->screen2map(e->x(),e->y(), &x, &y);
        selX0 = selX1=x;
        selY0 = selY1=y;
        update();
    }
}
//---------------------------------------------------------
void Terrain::mouseReleaseEvent (QMouseEvent * e) {


//printf("release\n");
//
    double x0, y0, x1, y1;
    double x,y;
    if (isSelectionZoneEnCours)
    {
        isSelectionZoneEnCours = false;
        proj->screen2map(e->x(),e->y(), &x, &y);
        selX1=x;
        selY1=y;
        if (getSelectedRectangle(&x0,&y0, &x1,&y1))
        {
            emit selectionOK(x0, y0, x1, y1);
            // Zoom sur la sélection si Ctrl appuyé
            if (e->modifiers() == Qt::ControlModifier) {
                this->slot_Zoom_Sel();
            }
        }
        else {
            emit mouseClicked(e);
            //QMessageBox::information (NULL,"after","test 2");
        }
    }
    else {
        emit mouseClicked(e);
        //QMessageBox::information (NULL,"after","test 3");
    }
}

void Terrain::mouseDoubleClickEvent(QMouseEvent * event)
{
    emit mouseDblClicked(event);
}

//---------------------------------------------------------
void Terrain::mouseMoveEvent (QMouseEvent * e) {
//printf("move %d %d\n",e->x(),e->y());
    double x,y;
    if (isSelectionZoneEnCours)
    {
        proj->screen2map(e->x(),e->y(), &x, &y);
        selX1=x;
        selY1=y;
/*        selX1 = ceil (selX1*12)/12;
        selY1 = floor(selY1*12)/12;*/
        update();
    }
    emit mouseMoved(e);
}

void Terrain::contextMenuEvent(QContextMenuEvent * event)
{
    emit showContextualMenu(event);
}

//---------------------------------------------------------
void Terrain::resizeEvent (QResizeEvent * /*e*/)
{
//printf("resize %d %d\n", width(), height());
    indicateWaitingMap();
    proj->setScreenSize( width(), height());
    isEarthMapValid = false;
    isWindMapValid = false;
    isResizing = true;

    // Evite les multiples update() pendant les changements de taille
    timerResize->stop();
    timerResize->start(100);
}
//---------------------------------------------------------
void Terrain::slotTimerResize () {
    if (isResizing) {
//printf("timer update\n");
        isResizing = false;
        setProjection();
    }
}
//---------------------------------------------------------
void Terrain::slot_Zoom_In()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->zoom(1.3);
    setProjection();
}
void Terrain::slot_Zoom_Out()
{
    if(isResizing)
        return;
    proj->zoom(1/1.3);
    setProjection();
}
//---------------------------------------------------------
void Terrain::slot_Zoom_Sel()
{
    double x0, y0, x1, y1;

    if (getSelectedRectangle(&x0,&y0, &x1,&y1))
    {
        // zoom sur la zone sélectionnée
        proj->updateZoneSelected(x0,y0, x1,y1);
        setProjection();
        isSelectionZoneEnCours = false;
        selX0 = selY0 = 0;
        selX1 = selY1 = 0;
    }
    else {
        indicateWaitingMap();
        selX0 = selY0 = 0;
        selX1 = selY1 = 0;
        isSelectionZoneEnCours = false;
        isEarthMapValid = false;
        isWindMapValid = false;
        zoomOnGribFile();
        update();
    }
}

void Terrain::clearSelection()
{
    isSelectionZoneEnCours=false;
    selX0 = selX1 = 0;
    selY0 = selY1 = 0;
    update();
}

//---------------------------------------------------------
void Terrain::slot_Zoom_All()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->init(this->width(), this->height(), 0,0);
    setProjection();
}
//------------------------------------------------
void Terrain::slot_Go_Left()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->move( 0.2, 0);
    setProjection();
}
void Terrain::slot_Go_Right()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->move(-0.2, 0);
    setProjection();
}
void Terrain::slot_Go_Up()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->move(0,  -0.2);
    setProjection();
}
void Terrain::slot_Go_Down()
{
    if(isResizing /*|| drawingMap*/)
        return;
    proj->move(0,  0.2);
    setProjection();
}

void Terrain::setCentralPixel(int i, int j)
{
   proj->setCentralPixel(i,j);
   setProjection();
}

void Terrain::setCenterInMap(float x, float y)
{
        proj->setCenterInMap(x,y);
        setProjection();
}



//---------------------------------------------------------
// paintEvent
//---------------------------------------------------------
void Terrain::paintEvent(QPaintEvent * /*event*/)
{
    QPainter pnt(this);

    QColor transp;
    int r = 100;
    bool longJob = false;

    if (!isResizing && !drawingMap)
    {
        // Draw the map
        drawingMap = true;
        longJob = draw_GSHHSandGRIB(pnt);

        if (selX0!=selX1 && selY0!=selY1) {
            // Draw the rectangle of the selected zone
            pnt.setPen(selectColor);
            transp = QColor(r,r,r, 80);
            pnt.setBrush(transp);
            int x0,y0, x1,y1;
            proj->map2screen(selX0, selY0, &x0, &y0);
            proj->map2screen(selX1, selY1, &x1, &y1);
            pnt.drawRect(x0, y0, x1-x0, y1-y0);

            if (showOrthodromie)
            {
                QPen penLine(QColor(Qt::white));
                penLine.setWidthF(1.6);
                pnt.setPen(penLine);
                draw_Orthodromie(pnt);
            }
        }
        drawBoats(pnt);
        drawOpponents(pnt);
        drawingMap=false;
    }

    if (pleaseWait) {
        // Write the message "please wait..." on the map
        QFont fontWait("Helvetica", 12, QFont::Bold, true);
        QFontMetrics fmet(fontWait);
        pnt.setPen(QColor(Qt::white));
        r = 80;
        transp = QColor(r,r,r, 80);
        pnt.setFont(fontWait);
        pnt.setBrush(transp);
        QString txt = tr("  Calculs en cours...  ");
        QRect rect = fmet.boundingRect(txt);

        rect.moveTo(20,20);
        pnt.drawRect(rect);
        pnt.drawText(rect, Qt::AlignHCenter|Qt::AlignVCenter , txt);
    }
}
