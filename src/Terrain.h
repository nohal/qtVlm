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


class Terrain;

#include "GshhsReader.h"
#include "GisReader.h"
#include "Grib.h"
#include "Projection.h"
#include "boatAccount.h"
#include "opponentBoat.h"
#include "POI.h"
#include "dialog_gribDate.h"

class Terrain : public QWidget
{
    Q_OBJECT

public:
    Terrain(QWidget *parent, Projection *proj);

    void       setGSHHS_map(GshhsReader *map);
    Grib * getGrib()     {return grib;};
    void       setCurrentDate(time_t t);
    time_t getCurrentDate(void);

    void  indicateWaitingMap();    // Affiche un message d'attente

    bool  isSelectingZone()      {return isSelectionZoneEnCours;}

    bool  getSelectedRectangle (double *x0, double *y0, double *x1, double *y1);
    bool  getSelectedLine      (float *x0, float *y0, float *x1, float *y1);

    void setBoatList(QList<boatAccount*> & boat_list) {this->boat_list=&boat_list; update();}
    void setOpponents(opponentList * opponents) { this->opponents=opponents; };

    void showGribDate_dialog(void);


public slots :
    // Map
    void setProjection();
    void setDrawRivers(bool);
    void setDrawCountriesBorders(bool);
    void setDrawOrthodromie(bool);
    void setCountriesNames(bool);
    void setMapQuality(int q);
    void slot_Zoom_In();
    void slot_Zoom_Out();
    void slot_Zoom_Sel();
    void slot_Zoom_All();
    void slot_Go_Left();
    void slot_Go_Right();
    void slot_Go_Up();
    void slot_Go_Down();
    void setCentralPixel(int i, int j);
    void setCenterInMap(float x, float y);

    void updateGraphicsParameters();
    void clearSelection(void);
    void clearCompassLine(void);

    // Grib
    void loadGribFile(QString fileName, bool zoom);

    void setDrawWindColors    (bool);

    void setColorMapSmooth (bool);
    void setDrawWindArrows    (bool);
    void setBarbules          (bool);
    void setCitiesNamesLevel  (int level);

    void slotTimerResize();
    void slotMustRedraw();

    void showCompassLine(double,double,double);

signals:
    void selectionOK(float x0, float y0, float x1, float y1);
    void mouseClicked(QMouseEvent * e);
    void mouseDblClicked(QMouseEvent * e);
    void mouseMoved(QMouseEvent * e);
    void showContextualMenu(QContextMenuEvent * event);
    void projectionUpdated();
    void POI_selectAborted(POI*);
    int  getRaceVacLen(boatAccount *,int*);

private:
    //-----------------------------------------------
    GshhsReader *gshhsReader;
    GisReader   *gisReader;
    Projection  *proj;
    Grib        *grib;
    QTimer      *timerResize;

    QPixmap     *imgEarth;   // images précalculées pour accélérer l'affichage

    QPixmap     *imgWind;
    QPixmap     *imgAll;

    bool        isEarthMapValid;
    bool        isWindMapValid;
    bool        mustRedraw;
    QCursor     enterCursor;
    bool        isResizing;

    void  draw_OrthodromieSegment
            (QPainter &pnt, float x0,float y0, float x1,float y1, int recurs=0);

    //-----------------------------------------------
    void  paintEvent(QPaintEvent *event);
    void  resizeEvent (QResizeEvent *e);

    void  keyPressEvent (QKeyEvent *e);
    void  keyReleaseEvent (QKeyEvent *e);
    void  keyModif(QKeyEvent *e);

    void  mousePressEvent (QMouseEvent * e);
    void  mouseReleaseEvent (QMouseEvent * e);
    void  mouseDoubleClickEvent(QMouseEvent * event);
    void  mouseMoveEvent (QMouseEvent * e);
    void  enterEvent (QEvent * e);
    void  leaveEvent (QEvent * e);

    void contextMenuEvent(QContextMenuEvent * event);

    void zoomOnGribFile();

    //-----------------------------------------------
    bool    isSelectionZoneEnCours;
    bool    isCompassLineEnCours;
    float   selX0, selY0, selX1, selY1;   // sélection de zone (repère carte)
    double compass_windAngle;

    QColor  seaColor, landColor, backgroundColor;
    QColor  selectColor;
    QColor  windArrowsColor;

    QPen    seaBordersPen;
    QPen    boundariesPen;
    QPen    riversPen;

    int     quality;

    //-----------------------------------------------
    // Flags indiquant les éléments �  dessiner
    //-----------------------------------------------
    bool  showCountriesBorders;
    bool  showRivers;
    bool  showOrthodromie;
    bool  interpolateValues;
    bool  windArrowsOnGribGrid;

    bool  showWindColorMap;
    bool  colorMapSmooth;
    bool  showWindArrows;
    bool  showBarbules;
    int   showCitiesNamesLevel;
    bool  showCountriesNames;

    //-----------------------------------------------
    bool draw_GSHHSandGRIB(QPainter &painter);
    void draw_Orthodromie(QPainter &painter);

    bool pleaseWait;     // long task in progress

    bool drawingMap;


    QList<boatAccount*> * boat_list;
    opponentList * opponents;
    void drawBoats(QPainter &pnt);
    void drawOpponents(QPainter &pnt);

    dialog_gribDate * gribDateDialog;

};

#endif
