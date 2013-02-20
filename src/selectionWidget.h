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
#ifdef QT_V5
#include <QtWidgets/QGraphicsWidget>
#else
#include <QGraphicsWidget>
#endif
#include "class_list.h"

class selectionWidget : public QGraphicsWidget
{ Q_OBJECT
    public:
        selectionWidget(myCentralWidget * centralWidget,Projection * proj, QGraphicsScene * myScene);

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
        void slot_protect();
        void slot_unprotect();

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *e);

    private:
        int xa,xb,ya,yb;
        int old_xa,old_xb,old_ya,old_yb;
        int width,height;
        bool selecting;
        bool showOrthodromie;

        myCentralWidget * centralWidget;
        Projection * proj;
        orthoSegment * seg;

        QAction *ac_delAllPOIs;
        QAction *ac_delSelPOIs;
        QAction *ac_notSimpSelPOIs;
        QAction *ac_simpSelPOIs;
        QAction *ac_dwnldZygrib;
        QAction *ac_mailSailDoc;
        QAction * ac_zoomSelection;

        QMenu *popup;

        void updateSize(void);
        bool isProtected;
};
Q_DECLARE_TYPEINFO(selectionWidget,Q_MOVABLE_TYPE);

#endif // SELECTIONWIDGET_H
