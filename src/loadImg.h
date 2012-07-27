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

#ifndef LOADIMG_H
#define LOADIMG_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QMenu>

#include "class_list.h"
#include "mycentralwidget.h"
#define MIN_ALPHA 0.2

//===================================================================
class loadImg : public QObject, public QGraphicsPixmapItem
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        loadImg(Projection *proj, myCentralWidget * parent);

        ~loadImg();

        /* accés aux données */
        void setMyImgFileName(QString s);
        void setLonLat(double lon1, double lat1, double lon2, double lat2);
        QString getMyImgFileName(){return this->myImgFileName;}
        void setAlpha(double alpha){this->alpha=alpha;this->setOpacity(alpha);}
        double getAlpha(){return alpha;}

    public slots:
        void slot_updateProjection();


    private:
        /* parent, main, etc */
        myCentralWidget   *parent;
        Projection * proj;

        /* data */
        double  lon1, lat1, lat2, lon2;
        double  alpha;
        QString myImgFileName;
        /*internal*/
        QPixmap img;
        QPixmap imgTemp;
        void map2Image(double lon, double lat,double * x, double * y);
};

#endif
