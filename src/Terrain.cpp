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
    selX0 = selY0 = 0;
    selX1 = selY1 = 0;
    isResizing = true;
    timerResize = new QTimer(this);
    assert(timerResize);
    timerResize->setSingleShot(true);
    connect(timerResize, SIGNAL(timeout()), this, SLOT(slotTimerResize()));
    connect(this,SIGNAL(showMessage(QString)),parent,SLOT(slotShowMessage(QString)));

    //---------------------------------------------------------------------
    showCountriesBorders  = Util::getSetting("showCountriesBorders", true).toBool();
    showOrthodromie   = Util::getSetting("showOrthodromie", false).toBool();
    showRivers   = Util::getSetting("showRivers", false).toBool();
    showCitiesNamesLevel = Util::getSetting("showCitiesNamesLevel", 0).toInt();
    showCountriesNames = Util::getSetting("showCountriesNames", false).toBool();

    showIsobars  = Util::getSetting("showIsobars", true).toBool();
    showIsobarsLabels = Util::getSetting("showIsobarsLabels", false).toBool();
    isobarsStep = Util::getSetting("isobarsStep", 2).toInt();
    showPressureMinMax = Util::getSetting("showPressureMinMax", false).toBool();

    showWindColorMap  = Util::getSetting("showWindColorMap", true).toBool();
    showRainColorMap  = Util::getSetting("showRainColorMap", false).toBool();
    showCloudColorMap  = Util::getSetting("showCloudColorMap", false).toBool();
    showHumidColorMap  = Util::getSetting("showHumidColorMap", false).toBool();

    colorMapSmooth = Util::getSetting("colorMapSmooth", true).toBool();
    showWindArrows  = Util::getSetting("showWindArrows", true).toBool();
    showBarbules = Util::getSetting("showBarbules", true).toBool();

    showTemperatureLabels = Util::getSetting("showTemperatureLabels", false).toBool();
    showGribGrid = Util::getSetting("showGribGrid", false).toBool();
    //----------------------------------------------------------------------------

    imgEarth = NULL;

    imgWind  = NULL;
    imgAll   = NULL;
    isEarthMapValid = false;
    isWindMapValid  = false;
    mustRedraw = true;

    proj = proj_;
    gshhsReader = NULL;
    gribPlot = new GribPlot();
    assert(gribPlot);
    setIsobarsStep(isobarsStep);

    gisReader = new GisReader();
    assert(gisReader);

    isSelectionZoneEnCours = false;
    setPalette(QPalette(backgroundColor));
    setAutoFillBackground(true);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    updateGraphicsParameters();
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

    isobarsPen.setColor(Util::getSetting("isobarsLineColor", QColor(80,80,80)).value<QColor>());
    isobarsPen.setWidthF(Util::getSetting("isobarsLineWidth", 2.0).toDouble());

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

            gribPlot->show_GRIB_CoverZone(pnt, proj);

            windArrowsColor.setRgb(255, 255, 255);

            if (showWindColorMap) {
//QTime t1 = QTime::currentTime();
                gribPlot->draw_WIND_Color(pnt, proj, colorMapSmooth);
//printf("time showWindColorMap=%d\n", t1.elapsed());
            }
            else if (showRainColorMap) {
                windArrowsColor.setRgb(140, 120, 100);
                gribPlot->draw_RAIN_Color(pnt, proj, colorMapSmooth);
            }
            else if (showCloudColorMap) {
                windArrowsColor.setRgb(180, 180, 80);
                gribPlot->draw_CLOUD_Color(pnt, proj, colorMapSmooth);
            }
            else if (showHumidColorMap) {
                windArrowsColor.setRgb(180, 180, 80);
                gribPlot->draw_HUMID_Color(pnt, proj, colorMapSmooth);
            }

            if (showIsobars) {
                pnt.setPen(isobarsPen);
                gribPlot->draw_PRESSURE_Isobars(pnt, proj);
            }

            if (showWindArrows) {
                gribPlot->draw_WIND_Arrows(pnt, proj, showBarbules, windArrowsColor);
            }

            if (showIsobarsLabels) {
                gribPlot->draw_PRESSURE_IsobarsLabels(pnt, proj);
            }
            if (showPressureMinMax) {
                gribPlot->draw_PRESSURE_MinMax (pnt, proj);
            }
            if (showTemperatureLabels) {
                gribPlot->draw_TEMPERATURE_Labels (pnt, proj);
            }


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
            // Grille GRIB
            //===================================================
            if (showGribGrid) {
                gribPlot->draw_GribGrid(pnt, proj);
            }
            //===================================================
            // Dates de la prévision
            //===================================================
            gribPlot->draw_ForecastDates(pnt, proj);

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
    if (recurs > 100)
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

//---------------------------------------------------------
void Terrain::indicateWaitingMap()
{
    pleaseWait = true;   // Affiche un message d'attente
    update();
    QCoreApplication::processEvents();
}

//-------------------------------------------------------
void Terrain::setProjection(Projection *proj_)
{
//printf("Terrain::setProjection\n");
    indicateWaitingMap();
    proj = proj_;
    isEarthMapValid = false;
    isWindMapValid = false;
    update();
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
void Terrain::slotTemperatureLabels(bool b) {
    if (showTemperatureLabels != b) {
        showTemperatureLabels = b;
        Util::setSetting("showTemperatureLabels", b);
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
        if (b==true) {     // Désactive les autres cartes en couleurs
            showRainColorMap = false;
            Util::setSetting("showRainColorMap", false);
            showCloudColorMap = false;
            Util::setSetting("showCloudColorMap", false);
            showHumidColorMap = false;
            Util::setSetting("showHumidColorMap", false);
        }
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawRainColors (bool b) {
    if (showRainColorMap != b) {
        showRainColorMap = b;
        Util::setSetting("showRainColorMap", b);
        if (b==true) {     // Désactive les autres cartes en couleurs
            showWindColorMap = false;
            Util::setSetting("showWindColorMap", false);
            showCloudColorMap = false;
            Util::setSetting("showCloudColorMap", false);
            showHumidColorMap = false;
            Util::setSetting("showHumidColorMap", false);
        }
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawCloudColors (bool b) {
    if (showCloudColorMap != b) {
        showCloudColorMap = b;
        Util::setSetting("showCloudColorMap", b);
        if (b==true) {     // Désactive les autres cartes en couleurs
            showWindColorMap = false;
            Util::setSetting("showWindColorMap", false);
            showRainColorMap = false;
            Util::setSetting("showRainColorMap", false);
            showHumidColorMap = false;
            Util::setSetting("showHumidColorMap", false);
        }
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawHumidColors (bool b) {
    if (showHumidColorMap != b) {
        showHumidColorMap = b;
        Util::setSetting("showHumidColorMap", b);
        if (b==true) {     // Désactive les autres cartes en couleurs
            showWindColorMap = false;
            Util::setSetting("showWindColorMap", false);
            showRainColorMap = false;
            Util::setSetting("showRainColorMap", false);
            showCloudColorMap = false;
            Util::setSetting("showCloudColorMap", false);
        }
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
//-------------------------------------------------------
void Terrain::setGribGrid (bool b) {
    if (showGribGrid != b) {
        showGribGrid = b;
        Util::setSetting("showGribGrid", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setPressureMinMax (bool b) {
    if (showPressureMinMax != b) {
        showPressureMinMax = b;
        Util::setSetting("showPressureMinMax", b);
        mustRedraw = true;
        update();
    }
}

//-------------------------------------------------------
void Terrain::setDrawIsobars(bool b) {
    if (showIsobars != b) {
        showIsobars = b;
        Util::setSetting("showIsobars", b);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setIsobarsStep(int step)
{
    if (gribPlot != NULL) {
        gribPlot->setIsobarsStep(step);
        Util::setSetting("isobarsStep", step);
        mustRedraw = true;
        update();
    }
}
//-------------------------------------------------------
void Terrain::setDrawIsobarsLabels(bool b) {
    if (showIsobarsLabels != b) {
        showIsobarsLabels = b;
        Util::setSetting("showIsobarsLabels", b);
        mustRedraw = true;
        update();
    }
}

//=================================================================
bool  Terrain::getSelectedRectangle(float *x0, float *y0, float *x1, float *y1)
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
    gribPlot->loadGribFile(fileName);
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
    GribReader * gribReader= gribPlot->getGribReader();
    if (gribReader != NULL) {
        float x0,y0, x1,y1, mh, mv;
        if (gribReader->getGribExtension(&x0,&y0, &x1,&y1))
        {
            mh = fabs(x0-x1)*0.05;
            mv = fabs(y0-y1)*0.05;
            proj->updateZoneSelected(x0-mh,y0-mv, x1+mh,y1+mv);
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
    if (gribPlot->getCurrentDate() != t)
    {
        indicateWaitingMap();
        gribPlot->setCurrentDate(t);
        isWindMapValid = false;
        update();
    }
}


//---------------------------------------------------------
// Events
//---------------------------------------------------------
void  Terrain::keyPressEvent (QKeyEvent *e)
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
void  Terrain::keyReleaseEvent (QKeyEvent *e)
{
    this->keyPressEvent(e);
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
        isSelectionZoneEnCours = true;
        proj->screen2map(e->x(),e->y(), &selX1, &selY1);
/*        selX1 = floor(selX1*12)/12;
        selY1 = ceil (selY1*12)/12;*/
        selX0 = selX1;
        selY0 = selY1;
        update();
    }
}
//---------------------------------------------------------
void Terrain::mouseReleaseEvent (QMouseEvent * e) {


//printf("release\n");
//
    float x0, y0, x1, y1;
    if (isSelectionZoneEnCours)
    {
        isSelectionZoneEnCours = false;
        proj->screen2map(e->x(),e->y(), &selX1, &selY1);
/*        selX1 = ceil (selX1*12)/12;
        selY1 = floor(selY1*12)/12;*/
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


//---------------------------------------------------------
void Terrain::mouseMoveEvent (QMouseEvent * e) {
//printf("move %d %d\n",e->x(),e->y());
    if (isSelectionZoneEnCours)
    {
        proj->screen2map(e->x(),e->y(), &selX1, &selY1);
/*        selX1 = ceil (selX1*12)/12;
        selY1 = floor(selY1*12)/12;*/
        update();
    }
    emit mouseMoved(e);
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

//    update();
}
//---------------------------------------------------------
void Terrain::slotTimerResize () {
    if (isResizing) {
//printf("timer update\n");
        isResizing = false;
        update();
    }
}
//---------------------------------------------------------
void Terrain::slot_Zoom_In()
{
    proj->zoom(1.3);
    setProjection(proj);
}
void Terrain::slot_Zoom_Out()
{
    proj->zoom(0.7);
    setProjection(proj);
}
//---------------------------------------------------------
void Terrain::slot_Zoom_Sel()
{
    float x0, y0, x1, y1;
    if (getSelectedRectangle(&x0,&y0, &x1,&y1))
    {
        // zoom sur la zone sélectionnée
        proj->updateZoneSelected(x0,y0, x1,y1);
        setProjection(proj);
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
//---------------------------------------------------------
void Terrain::slot_Zoom_All()
{
    proj->init(this->width(), this->height(), 0,0);
    setProjection(proj);
}
//------------------------------------------------
void Terrain::slot_Go_Left()
{
    proj->move( 0.2, 0);
    setProjection(proj);
}
void Terrain::slot_Go_Right()
{
    proj->move(-0.2, 0);
    setProjection(proj);
}
void Terrain::slot_Go_Up()
{
    proj->move(0,  -0.2);
    setProjection(proj);
}
void Terrain::slot_Go_Down()
{
    proj->move(0,  0.2);
    setProjection(proj);
}

//---------------------------------------------------------
void Terrain::setShowPOIs(bool show)
{
    Util::setSetting("showPOIs", show);
    // list of all the POI's
    QList<POI*> lpois = findChildren <POI*>();
    for (int i=0; i<lpois.size(); i++)
    {
        if (show)
            lpois.at(i)->setVisible(true);
        else
            lpois.at(i)->setVisible(false);
    }
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

    if (!isResizing)
    {
        // Draw the map
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



