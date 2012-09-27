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
#include "bsb.h"

#define MIN_ALPHA 0.2

//===================================================================
class loadImg : public QObject, public QGraphicsPixmapItem
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        loadImg(Projection *proj, myCentralWidget * parent);

        ~loadImg();

        /* accés aux données */
        bool setMyImgFileName(QString s);
        QString getMyImgFileName(){return this->myImgFileName;}
        void setAlpha(double alpha){this->alpha=alpha;this->setOpacity(alpha);}
        void setGribOpacity(double d);
        double getAlpha(){return alpha;}
        QPolygonF getBorders(){return borders;}
        void setImgGribKap(QPixmap imgGribKap);
        bool getGribColored(){return gribColored;}
        double getGribAlpha(){return gribAlpha;}
        bool getDrawGribOverKap(){return drawGribOverKap;}
        void setParams(double alpha,double gribAlpha,bool drawGribOverKap,bool gribColored);
        void redraw(bool b1, bool b2);
    public slots:
        void slot_updateProjection();


    private:
        /* parent, main, etc */
        myCentralWidget   *parent;
        Projection * proj;

        /* data */
        double  alpha;
        QString myImgFileName;
        double  gribAlpha;
        bool drawGribOverKap;
        bool gribColored;
        /*internal*/
        void convertBsb2Pixmap(BSBImage * b);
        void map2Image(double lon, double lat,double * x, double * y);
        BSBImage * bsb;
        uint8_t * bsbBuf;
        uint8_t * getRow(int row);
        QPolygonF borders;
        QGraphicsPixmapItem * gribKap;
        QPixmap imgGribKap;
};

#endif
