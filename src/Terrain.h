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

#ifndef TERRAIN_H
#define TERRAIN_H

#include <QWidget>
#include <QToolBar>
#include <QBitmap>
#include <QTimer>

#include "GshhsReader.h"
#include "GisReader.h"
#include "GribPlot.h"
#include "Projection.h"

class Terrain : public QWidget
{
    Q_OBJECT

public:
    Terrain(QWidget *parent, Projection *proj);

    void       setGSHHS_map(GshhsReader *map);
    GribPlot * getGribPlot()     {return gribPlot;};
    void       setCurrentDate( time_t t);

    void  indicateWaitingMap();    // Affiche un message d'attente

    bool  isSelectingZone()      {return isSelectionZoneEnCours;}

    bool  getSelectedRectangle (float *x0, float *y0, float *x1, float *y1);
    bool  getSelectedLine      (float *x0, float *y0, float *x1, float *y1);



public slots :
    // Map
    void setProjection(Projection *);
    void setDrawRivers(bool);
    void setDrawCountriesBorders(bool);
    void setDrawOrthodromie(bool);
    void setCountriesNames(bool);
    void slotTemperatureLabels(bool);
    void setMapQuality(int q);
    void slot_Zoom_In();
    void slot_Zoom_Out();
    void slot_Zoom_Sel();
    void slot_Zoom_All();
    void slot_Go_Left();
    void slot_Go_Right();
    void slot_Go_Up();
    void slot_Go_Down();

    void setShowPOIs(bool);
    void updateGraphicsParameters();

    // Grib
    void loadGribFile(QString fileName, bool zoom);

    void setDrawWindColors    (bool);
    void setDrawRainColors    (bool);
    void setDrawCloudColors   (bool);
    void setDrawHumidColors   (bool);

    void setColorMapSmooth (bool);
    void setDrawWindArrows    (bool);
    void setBarbules          (bool);

    void setGribGrid          (bool);
    void setCitiesNamesLevel  (int level);
    void setDrawIsobars       (bool);
    void setDrawIsobarsLabels (bool);
    void setIsobarsStep(int step);
    void setPressureMinMax   (bool);

    void slotTimerResize();
    void slotMustRedraw();

signals:
    void selectionOK(float x0, float y0, float x1, float y1);
    void mouseClicked(QMouseEvent * e);
    void mouseMoved(QMouseEvent * e);
    void showMessage(QString);

private:
    //-----------------------------------------------
    GshhsReader *gshhsReader;
    GisReader   *gisReader;
    Projection  *proj;
    GribPlot    *gribPlot;
    QTimer      *timerResize;

    QPixmap     *imgEarth;   // images précalculées pour accélérer l'affichage

    QPixmap     *imgWind;
    QPixmap     *imgAll;

    bool        isEarthMapValid;
    bool        isWindMapValid;
    bool        mustRedraw;
    QCursor     enterCursor;
    bool    isResizing;

    void  draw_OrthodromieSegment
            (QPainter &pnt, float x0,float y0, float x1,float y1, int recurs=0);

    //-----------------------------------------------
    void  paintEvent(QPaintEvent *event);
    void  resizeEvent (QResizeEvent *e);

    void  keyPressEvent (QKeyEvent *e);
    void  keyReleaseEvent (QKeyEvent *e);

    void  mousePressEvent (QMouseEvent * e);
    void  mouseReleaseEvent (QMouseEvent * e);
    void  mouseMoveEvent (QMouseEvent * e);
    void  enterEvent (QEvent * e);
    void  leaveEvent (QEvent * e);

    void zoomOnGribFile();

    //-----------------------------------------------
    bool    isSelectionZoneEnCours;
    float   selX0, selY0, selX1, selY1;   // sélection de zone (repère carte)

    QColor  seaColor, landColor, backgroundColor;
    QColor  selectColor;
    QColor  windArrowsColor;

    QPen    isobarsPen;
    QPen    seaBordersPen;
    QPen    boundariesPen;
    QPen    riversPen;

    int     quality;
    int     isobarsStep;

    //-----------------------------------------------
    // Flags indiquant les éléments à dessiner
    //-----------------------------------------------
    bool  showCountriesBorders;
    bool  showRivers;
    bool  showOrthodromie;

    bool  showWindColorMap;
    bool  showRainColorMap;
    bool  showCloudColorMap;
    bool  showHumidColorMap;

    bool  showIsobars;
    bool  showIsobarsLabels;
    bool  colorMapSmooth;
    bool  showWindArrows;
    bool  showBarbules;
    bool  showGribGrid;
    bool  showPressureMinMax;
    int   showCitiesNamesLevel;
    bool  showCountriesNames;
    bool  showTemperatureLabels;

    //-----------------------------------------------
    bool draw_GSHHSandGRIB(QPainter &painter);
    void draw_Orthodromie(QPainter &painter);

    bool pleaseWait;     // long task in progress

    bool drawingMap;

};

#endif
