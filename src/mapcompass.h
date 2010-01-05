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

#ifndef MAPCOMPASS_H
#define MAPCOMPASS_H


#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QPainter>

class mapCompass;

#include "Projection.h"
#include "MainWindow.h"
#include "orthoSegment.h"
#include "Orthodromie.h"
#include "mycentralwidget.h"

class mapCompass : public QGraphicsWidget
{ Q_OBJECT
    public:
        mapCompass(Projection * proj,MainWindow * main,myCentralWidget * parent);
        double getWindAngle(void) { return wind_angle; }
        bool tryMoving(int x, int y);
        bool hasCompassLine(void) { return drawCompassLine; }

        QPainterPath shape() const;
        QRectF boundingRect() const;

    public slots:
        void slot_compassLine(int mouse_x, int mouse_y);
        void slot_stopCompassLine(void);
        void slot_paramChanged(void);

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );        

        void mousePressEvent(QGraphicsSceneMouseEvent *);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
        void slotMouseDblClicked(QGraphicsSceneMouseEvent * e);
        //void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);

    private:
        int size;
        int mouse_x;
        int mouse_y;
        bool isMoving;

        bool mouseEvt;
        Projection * proj;
        myCentralWidget * parent;
        MainWindow * main;
        QCursor enterCursor;
        double wind_angle;
        float bvmg_up;
        float bvmg_down;

        /* Compass Line */        
        bool drawCompassLine;
        orthoSegment * compassLine;
        QGraphicsSimpleTextItem * hdg_label;
        QGraphicsSimpleTextItem * windAngle_label;
        void updateCompassLineLabels(int x, int y);
};

#endif // MAPCOMPASS_H
