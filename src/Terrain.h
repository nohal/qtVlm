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

#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QToolBar>
#include <QBitmap>

class POI;
class Terrain;

#include "GshhsReader.h"
#include "GisReader.h"

#include "Projection.h"
#include "mycentralwidget.h"

class Terrain : public QGraphicsWidget
{
    Q_OBJECT

public:
    Terrain(myCentralWidget *parent, Projection *proj);

    void  setGSHHS_map(GshhsReader *map);

    void updateSize(int width, int height);

    QRectF boundingRect() const;
    QPainterPath shape() const;

public slots :
    // Map
    void setDrawRivers(bool);
    void setDrawCountriesBorders(bool);
    void setCountriesNames(bool);
    void slot_setMapQuality(int q);

    void updateGraphicsParameters();

    void slot_setDrawWindColors    (bool);

    void setColorMapSmooth (bool);
    void setDrawWindArrows    (bool);
    void setBarbules          (bool);
    void setCitiesNamesLevel  (int level);

    void redrawAll(void);
    void redrawGrib(void);



signals:
    void showContextualMenu(QGraphicsSceneContextMenuEvent * event);
    void mousePress(QGraphicsSceneMouseEvent* e);
    void mouseRelease(QGraphicsSceneMouseEvent* e);
    int  getRaceVacLen(boatAccount *,int*);

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

    QImage     *imgEarth;   // images précalculées pour accélérer l'affichage
    QImage     *imgSea;
    QPixmap     *imgWind;
    QPixmap     *imgAll;

    bool        isEarthMapValid;
    bool        isWindMapValid;
    bool        mustRedraw;
    QCursor     enterCursor;




    //-----------------------------------------------
    // ox01
    //void  paintEvent(QPaintEvent *event);
    //void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);


    //void  mousePressEvent (QMouseEvent * e);
    //void  mouseReleaseEvent (QMouseEvent * e);
    //void  mouseDoubleClickEvent(QMouseEvent * event);
    //void  mouseMoveEvent (QGraphicsSceneMouseEvent * event);
    //void  enterEvent (QEvent * e);
    //void  leaveEvent (QEvent * e);



    //-----------------------------------------------

    QColor  seaColor, landColor, backgroundColor, tranparent;
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
    void draw_GSHHSandGRIB(void);
    void indicateWaitingMap(void);
    void updateRoutine(void);
    bool isWaiting;
};

#endif
