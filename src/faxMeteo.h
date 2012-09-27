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

#ifndef FAXMETEO_H
#define FAXMETEO_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QMenu>

#include "class_list.h"
#include "mycentralwidget.h"
#define MIN_ALPHA 0.2

//===================================================================
class faxMeteo : public QGraphicsWidget
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        faxMeteo(Projection *proj, myCentralWidget * parent);

        ~faxMeteo();

        /* accés aux données */
        void setImgFileName(QString imgFileName);
        void setLonLat(double lon, double lat){this->lat=lat;this->lon=lon;}
        void setLonLatRange(double lonRange,double latRange){this->lonRange=lonRange;this->latRange=latRange;}
        void setAlpha(double alpha){this->alpha=alpha;this->setOpacity(alpha);}
        QString getFileName(){return this->imgFileName;}
        QPointF getLonLat(){return QPointF(lon,lat);}
        double getLatRange(){return latRange;}
        double getLonRange(){return lonRange;}
        double getAlpha(){return this->alpha;}
        void setPresetNb(QString i){this->presetNb=i;}
        QString getPresetNb(){return this->presetNb;}
        void savePreset();
        void loadPreset();
        /* event propagé par la scene */
        bool tryMoving(int x, int y);

    public slots:
        void slot_updateProjection();

    protected:
        void   mousePressEvent(QGraphicsSceneMouseEvent * e);
        void   mouseReleaseEvent(QGraphicsSceneMouseEvent * e);
        void   paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
        QRectF boundingRect() const;
        QPainterPath shape() const;

    private:
        /* parent, main, etc */
        myCentralWidget   *parent;
        Projection * proj;

        /* data */
        double  lon, lat, latRange, lonRange;
        double  alpha;
        QString imgFileName;
        /*internal*/
        QPixmap faxImg;
        bool    isMoving;
        QRectF  br;
        int     mouse_x,mouse_y;
        int     modifier;
        QString presetNb;

        /*popup menu*/
};

#endif
