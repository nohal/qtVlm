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

#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "class_list.h"

#include "ui_Route_Editor.h"
#include <QScrollArea>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QTimer>

//===================================================================
class DialogRoute : public QDialog, public Ui::ROUTE_Editor_ui
{ Q_OBJECT
    public:
        DialogRoute(ROUTE *route, myCentralWidget *parent);
        ~DialogRoute();
        void done(int result);
        void fillPilotView(bool f=false);

    public slots:
        void GybeTack(int i);
        void slotLoadPilototo();
        void slotLoadPilototoCustom();
        void slotEnvoyer();
        void slotApply();
        void slotIntervalTimer(int);
        void slotInterval();
        void slotTabChanged(int);
    protected:
        void resizeEvent ( QResizeEvent * event );
    signals:

    private:
        QTimer  * intervalTimer;
        QStandardItemModel *rmModel;
        QScrollArea *scroll;
        ROUTE   *route;
        myCentralWidget *parent;
        bool  modeCreation;
        InputLineParams *inputTraceColor;
        QStandardItemModel * model;
        QList<POI*> listPois;
        int tabWidthRatio;
        int roadMapWidthRatio;
        int tabWidth;
        int roadMapWidth;
};
class DateBoxDelegate : public QItemDelegate
{
     Q_OBJECT

 public:
     DateBoxDelegate(QObject *parent = 0);

     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const;

     void updateEditorGeometry(QWidget *editor,
         const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };
#endif
