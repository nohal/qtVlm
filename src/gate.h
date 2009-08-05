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

#ifndef GATE_H
#define GATE_H

#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>

#include "Projection.h"

class gate : public QWidget
{ Q_OBJECT
    public:
        gate(QString name,float lat_1, float lon_1,float lat_2, float lon_2,
             Projection *proj, QWidget *ownerMeteotable,QWidget *parentWindow,QString color="");
        ~gate();

        void setName(QString name);
        QString getName(void) { return name; }
        void setColor(QColor val) { myColor=val; }
        QColor getColor(void) { return myColor; }
        void setLat(int num,float val) {if(num==0) lat_1=val; else lat_2=val;}
        float getLat(int num) { return (num==0?lat_1:lat_2);}
        void setLon(int num,float val) {if(num==0) lon_1=val; else lon_2=val;}
        float getLon(int num) { return (num==0?lon_1:lon_2);}

    public slots:
        void updateProjection(void);
        void paramChanged(void);
        void slot_editGate(void);
        void slot_delGate(void);
        void slot_copyGate(void);

    signals:
        void delGate_list(gate*);
        void editGate(gate*);

    private:
        QString name;
        float lat_1;
        float lon_1;
        float lat_2;
        float lon_2;

        int i_1,j_1,i_2,j_2;
        int l_x,l_y;
        int l_w,l_h;
        bool notDrawing;

        Projection *proj;
        QWidget *owner;
        QWidget *parent;

        QCursor enterCursor;
        bool isIn;
        QColor myColor;

        void rmSignal(void);

        void createPopUpMenu(void);
        QMenu *popup;
        QAction * ac_editGate;
        QAction * ac_delGate;
        QAction * ac_copyGate;

        void paintEvent(QPaintEvent *event);
        //void enterEvent (QEvent * e);
        //void leaveEvent (QEvent * e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseMoveEvent (QMouseEvent * e);
        void contextMenuEvent(QContextMenuEvent *);
        bool validateCoord(int x, int y);
};

#endif // GATE_H
