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

#ifndef VLMLINE_H
#define VLMLINE_H

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QPainter>
#include <QObject>

#include "Projection.h"

#define VLMLINE_LINE_MODE   0
#define VLMLINE_POINT_MODE  1
#define VLMLINE_GATE_MODE   2

class vlmLine : public QGraphicsWidget
{ Q_OBJECT
    public:
        vlmLine(Projection * proj, QGraphicsScene * myScene,int z_level);

        void addPoint(float lat,float lon);
        void deleteAll(void);
        void setLinePen ( const QPen & pen ) {linePen = pen;update(); }
        void setPoly(QList<vlmPoint> & points);

        void setLineMode();
        void setPointMode(QColor pt_color);
        void setGateMode(QString desc);
        void setNbVacPerHour(int nbVacPerHour) {this->nbVacPerHour=nbVacPerHour;}
        void setPorteOnePoint(void){this->onePoint=true;}
        void setHidden(bool hidden) {this->hidden=hidden;update();}

        int count(void) { return line.count(); }

        ~vlmLine();

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
        QRectF boundingRect() const;

    public slots:
        void slot_showMe(void);
        void slot_shLab(){this->labelHidden=!this->labelHidden;update();}

    private:
        QList<vlmPoint> line;
        QPen linePen;
        QColor pt_color;
        QList<QPolygon*> polyList;
        bool onePoint;
        int mode;
        void calculatePoly(void);
        int nbVacPerHour;
        Projection * proj;
        QGraphicsScene * myScene;
        QString desc;
        QRectF boundingR;
        bool hidden;
        bool labelHidden;
};


#endif // VLMLINE_H
