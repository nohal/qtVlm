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
#include "vlmLine.h"
#include "class_list.h"

class orthoSegment : public QGraphicsWidget
{Q_OBJECT
    public:
        orthoSegment(Projection * proj, QGraphicsScene * myScene,int z_level,bool roundedEnd=false);
        ~orthoSegment();
        void initSegment(const double &lon1,const double &lat1,const double &lon2, const double &lat2);
        void moveSegment(const double &lon2,const double &lat2);
        void hideSegment(void);
        void setOrthoMode(const bool &mode) { isOrtho=mode; }
        bool orthoMode(void) const { return isOrtho; }
        void getStartPoint(double * xx,double * yy) const {*xx=lon1;*yy=lat1;}
        void setLinePen ( const QPen & pen ) {linePen = pen;myLine->setLinePen(linePen); }
        void setAlsoDrawLoxo(const bool &b){this->alsoDrawLoxo=b;}
        void set_toolTip(const QString &mes){this->myLine->setToolTip(mes);}

        void showSegment();
private:
        double lon1,lat1,lon2,lat2;
        bool isOrtho;
        QPen linePen;

        Projection * proj;

        void draw_orthoSegment(const double &longitude1, const double &latitude1, const double longitude2, const double latitude2, const int &recurs);
        bool roundedEnd;
        bool alsoDrawLoxo;
        vlmLine * myLine;
        void calculatePoly();
};
Q_DECLARE_TYPEINFO(orthoSegment,Q_MOVABLE_TYPE);

#endif // ORTHOSEGMENT_H
