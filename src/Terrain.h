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
#include "GshhsReader.h"
#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QToolBar>
#include <QBitmap>
#include <QMutex>
#include "routage.h"

#include "class_list.h"

class Terrain : public QGraphicsWidget
{
    Q_OBJECT

public:
    Terrain(myCentralWidget *parent, Projection *proj);

    void  setGSHHS_map(GshhsReader *map);
    void setColorMapMode(int mode);

    void updateSize(int width, int height);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    ROUTAGE * getRoutageGrib();
    void setRoutageGrib(ROUTAGE * routage);

    enum DrawGribPlainDataMode {
             drawNone,
             drawWind,
             drawCurrent,
             drawCloud,
             drawRain,
             drawCAPEsfc,
             drawSnowDepth,
             drawSnowCateg,
             drawFrzRainCateg,
             drawHumid,
             drawTemp,
             drawTempPot,
             drawTempMin,
             drawTempMax,
             drawDewpoint,
             drawDeltaDewpoint
    };
    void switchGribDisplay(bool windArrowOnly);

public slots :
    // Map
    void setDrawRivers(bool);
    void setDrawCountriesBorders(bool);
    void setCountriesNames(bool);

    void updateGraphicsParameters();

    void slot_setDrawWindColors    (bool);

    void setColorMapSmooth (bool);
    void setDrawWindArrows    (bool);
    void setBarbules          (bool);
    void setCitiesNamesLevel  (int level);
    void setDrawIsobars       (bool);
    void setDrawIsobarsLabels (bool);
    void setIsobarsStep		  (double step);
    void setPressureMinMax    (bool);
    void slotTemperatureLabels(bool b);

    void setDrawIsotherms0       (bool);
    void setDrawIsotherms0Labels (bool);
    void setIsotherms0Step	     (double step);

    void redrawAll();
    void redrawGrib(void);



signals:
    void showContextualMenu(QGraphicsSceneContextMenuEvent * event);
    void mousePress(QGraphicsSceneMouseEvent* e);
    void mouseRelease(QGraphicsSceneMouseEvent* e);
    int  getRaceVacLen(boatVLM *,int*);

protected:
    void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);

private:    
    //-----------------------------------------------
    int width,height;
    //-----------------------------------------------
    GshhsReader *gshhsReader;
    GisReader   *gisReader;
    Projection  *proj;
    myCentralWidget *parent;    

#ifdef __TERRAIN_QIMAGE
    QImage     *imgEarth;
#else
    QPixmap     *imgEarth;
#endif
    QPixmap     *imgWind;
    QPixmap     *imgAll;

    bool        isEarthMapValid;
    bool        isWindMapValid;
    bool        mustRedraw;
    QCursor     enterCursor;

    QColor  seaColor, landColor, backgroundColor, tranparent;
    QColor  selectColor;
    QColor  windArrowsColor;

    double isobarsStep;
    bool   showIsobars;
    bool   showIsobarsLabels;
    bool   showPressureMinMax;

    double isotherms0Step;
    bool   showIsotherms0;
    bool   showIsotherms0Labels;

    bool  showTemperatureLabels;

    QPen    seaBordersPen;
    QPen    boundariesPen;
    QPen    riversPen;
    QPen    isobarsPen;
    QPen    isotherms0Pen;


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

    int	  colorMapMode;

    //-----------------------------------------------
    void draw_GSHHSandGRIB(void);
    void drawGrib(QPainter &pnt, Grib *gribPlot);
    void indicateWaitingMap(void);
    void updateRoutine(void);
    bool toBeRestarted;
    ROUTAGE * routageGrib;
    QMutex mutex;
};
Q_DECLARE_TYPEINFO(Terrain,Q_MOVABLE_TYPE);

#endif
