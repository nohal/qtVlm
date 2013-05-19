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

#ifndef BARRIER_H
#define BARRIER_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QList>
#include <QMenu>
#include <QColor>


#include "dataDef.h"
#include "class_list.h"

class BarrierPoint: public QObject, public QGraphicsRectItem {
    Q_OBJECT
    public:
        BarrierPoint(MainWindow * mainWindow,Barrier * barrier,QColor color=Qt::black);

        void set_position(QPointF position);
        FCT_GET(QPointF,position)
        QPointF get_scenePosition(void);

        FCT_GET(Barrier*,barrier)

        void set_editMode(bool mode);        

        bool is_same(QPointF position);

    public slots:
        void slot_removePoint(void);
        void slot_insertAfter(void);

    signals:
        void positionChanged(void);

    private:
        MainWindow * mainWindow;
        Projection * projection;
        myCentralWidget * centralWidget;

        QPointF position;
        int pointSize;

        QMenu *popUpMenu;
        QAction * ac_remove;
        QAction * ac_insertAfter;
        QAction * ac_edit;
        QAction * ac_deleteBarrier;
        void contextMenuEvent (QGraphicsSceneContextMenuEvent *);

        Barrier * barrier;

        QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

};

class Barrier: public QObject, QGraphicsPathItem {
    Q_OBJECT
    public:
        Barrier(MainWindow * mainWindow,BarrierSet * barrierSet);
        ~Barrier(void);

        void initBarrier(QList<QPointF> pointList);

        void removePoint(BarrierPoint * point);
        BarrierPoint *appendPoint(QPointF point);

        BarrierPoint * add_pointAfter(BarrierPoint *basePoint,QPointF position);

        void set_editMode(bool mode);
        FCT_GET(bool,editMode)
        void set_barrierIsEdited(bool state);

        FCT_SETGET(QString,name)
        FCT_GET(BarrierSet *,barrierSet)
        QList<BarrierPoint*> * get_points(void) { return &points; }

        FCT_GET(QColor,color)
        void set_color(QColor color);

        int is_firstLast(QPointF screenPosition);

        bool cross(QLineF line);

    public slots:
        void slot_pointPositionChanged(void);
        void slot_insertPoint(void);
        void slot_deleteBarrier(void);
        void slot_adjustWidthF(void);

    private:
        MainWindow * mainWindow;
        Projection * projection;
        //MyView * myView;
        //QGraphicsScene * myScene;

        BarrierSet * barrierSet;

        int width;

        QList<BarrierPoint * > points;

        QPainterPath shapePath;
        QPainterPath shape() const;

        void clearBarrier(void);

        QMenu *popUpMenu;
        QAction * ac_insert;
        QAction * ac_edit;
        QAction * ac_delete;
        void contextMenuEvent (QGraphicsSceneContextMenuEvent *);
        QPointF cursorPosition;

        QString name;
        QColor color;
        bool editMode;
};

#endif // BARRIER_H
