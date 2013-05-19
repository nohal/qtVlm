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

#ifndef BARRIERSET_H
#define BARRIERSET_H

#include <QLineF>
#include <QList>

#include "class_list.h"
#include "Barrier.h"

extern QList<BarrierSet*> barrierSetList;

class BarrierSet: public QObject
{ Q_OBJECT
    public:
        BarrierSet(MainWindow *mainWindow);
        ~BarrierSet(void);
        bool cross(QLineF line);

        static void readBarriersFromDisk(MainWindow * mainWindow, QString fileName, bool import=false);
        static void saveBarriersToDisk(QString fileName);

        static void clear_barrierSetList(void);
        static void init_barrierSetList(void) { barrierSetList.clear(); }

        static bool myLessThan(const BarrierSet * set_1,const BarrierSet* set_2) {return set_1->name < set_2->name;}

        FCT_GET(QString,name)
        void set_name(QString name);
        QList<Barrier*> * get_barriers(void) { return &barrierList; }
        FCT_GET(QColor,color)
        void set_color(QColor color);

        void add_barrier(Barrier * barrier) { if(barrier) barrierList.append(barrier); }
        void remove_barrier(Barrier * barrier) { if(barrier) barrierList.removeAll(barrier); }

        void set_editMode(bool mode);
        void set_barrierIsEdited(bool state);

        bool is_firstLast(QPointF screenPosition,Barrier ** barrierPtr,int * num);

    public slots:
        void slot_editBarrierSet(void);
        void slot_delBarrierSet(void);

    signals:
        void barrierSetEdited(void);

    private:
        QList<Barrier*> barrierList;

        QColor color;
        QString name;

        MainWindow * mainWindow;

};

#endif // BARRIERSET_H
