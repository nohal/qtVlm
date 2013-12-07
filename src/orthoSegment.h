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

***********************************************************************/

#ifndef ORTHOSEGMENT_H
#define ORTHOSEGMENT_H

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QPainter>

#include "class_list.h"

class orthoSegment : public QGraphicsWidget
{Q_OBJECT
    public:
        orthoSegment(Projection * proj, QGraphicsScene * myScene,int z_level,bool roundedEnd=false);

        void initSegment(double xa,double ya,double xb, double yb);
        void moveSegment(double x,double y);
        void hideSegment(void);
        void setOrthoMode(bool mode) { isOrtho=mode; }
        bool orthoMode(void) { return isOrtho; }
        void getStartPoint(double * xx,double * yy){*xx=xa;*yy=ya;}
        void setLinePen ( const QPen & pen ) {linePen = pen;update(); }
        void setAlsoDrawLoxo(bool b){this->alsoDrawLoxo=b;}


public slots:
        void slot_update();
protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
        QRectF boundingRect() const;
        QPainterPath shape() const;

private:
        void myPrepareGeometryChange();
        double xa,xb,ya,yb;
        bool isOrtho;
        QPen linePen;

        Projection * proj;

        void draw_orthoSegment(QPainter * pnt, double i0, double j0, double i1, double j2, int recurs, QPainterPath *path);
        bool roundedEnd;
        bool alsoDrawLoxo;
        QPainterPath myPath;
};
Q_DECLARE_TYPEINFO(orthoSegment,Q_MOVABLE_TYPE);

#endif // ORTHOSEGMENT_H
