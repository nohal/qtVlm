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

#ifndef SELECTIONWIDGET_H
#define SELECTIONWIDGET_H

#include <QGraphicsWidget>

#include "class_list.h"

class selectionWidget : public QGraphicsWidget
{ Q_OBJECT
    public:
        selectionWidget(Projection * proj, QGraphicsScene * myScene);

        QPainterPath shape() const;
        QRectF boundingRect() const;
        void startSelection(int start_x,int start_y);
        bool tryMoving(int mouse_x,int mouse_y);
        void stopSelection(void);
        void clearSelection(void);
        bool isSelecting(void) { return selecting; }

        bool getShowOrthodromie(void) { return showOrthodromie; }
        bool getZone(double * x0, double * y0, double * x1, double * y1);
        bool getZoneWithSens(double * x0, double * y0, double * x1, double * y1);

    public slots:
        void slot_setDrawOrthodromie(bool b);

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        int xa,xb,ya,yb;
        int width,height;
        bool selecting;
        bool showOrthodromie;

        Projection * proj;
        orthoSegment * seg;

        void updateSize(void);
};
Q_DECLARE_TYPEINFO(selectionWidget,Q_MOVABLE_TYPE);

#endif // SELECTIONWIDGET_H
