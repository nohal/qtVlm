/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QAction>

#include "dataDef.h"
#include "class_list.h"


class MyToolBar: public QToolBar {
    Q_OBJECT
    public:
        MyToolBar(QString name,QString title,ToolBar * toolBar,QWidget * parent,bool canHide=true);
        FCT_GET(QString,name)
        FCT_SETGET(bool,displayed)
        bool get_canHide(void) { return (canHide && !forceMenuHide); }

        void chgVisibilty(bool visibility);

        void initCanHide(void);
    public slots:
        void slot_visibilityChanged(bool visibility);

    signals:
        void visibility_changed(MyToolBar *,bool);

    protected:
        //void closeEvent ( QCloseEvent * event );

    private:
        ToolBar * toolBar;
        QString name;
        bool displayed;
        bool canHide;
        bool forceMenuHide;
};




class ToolBar : public QObject
{ Q_OBJECT
    public:
        ToolBar(MainWindow * mainWindow);

        int build_showHideMenu(QMenu * menu);

        /* Misc toolbar */
        //QAction * acQuit;

        /* Grib toolbar */
        QToolButton * gribDwnld;
        QMenu * gribSubMenu;
        QAction * acWindZygrib;
        QAction * acWindVlm;
        QAction * acWindSailsDoc;
        QAction * acOpenGrib;
        QComboBox * cbGribStep;
        QAction * acDatesGrib_prev;
        QAction * acDatesGrib_next;
        QAction * acGrib_play;
        QAction * datesGrib_now;
        QAction * datesGrib_sel;
        QAction * acGrib_dialog;
        void update_gribBtn(void);
        int get_gribStep(void);
        bool isPlaying(void) { return acGrib_play->data().toInt()==1; }
        void stopPlaying(void);
        void update_gribDownloadBtn(void);

        /* selection toolBar */
        QAction *acMap_Zoom_In;
        QAction *acMap_Zoom_Out;
        QAction *acMap_Zoom_Sel;
        QAction *acMap_Zoom_All;
        QAction *selectionMode;
        QAction *magnify;

        /* estime toolBar */
        QLabel * lbEstime;
        QSpinBox *spnEstime;
        QComboBox * cbEstime;
        QCheckBox * chkEstime;
        void change_estimeParam(void);

        /* boat toolBar */
        QAction * acLock;
        QComboBox *boatList;
        void updateBoatList(QList<boatVLM*> & boat_list);
        void setSelectedBoatIndex(int index);


        /* BarrierSet toolBar */
        QAction *barrierAdd;

        /* gen functions */

        FCT_GET(QList<MyToolBar*>,toolBarList)

        void load_settings(void);
        void save_settings(void);

        void chgBoatType(int boatType);

        void chg_barrierAddState(bool state);

        void mySetIcon(QToolButton * button, QString iconFile);
        void mySetIcon(QAction * action, QString iconFile);
        QSize getIconSize() const {return iconSize;}
public slots:
        void slot_estimeValueChanged(int);
        void slot_estimeTypeChanged(int);
        void slot_estimeStartChanged(int);

        void slot_gribDwnld(void);
        void slot_gribZygrib(void);
        void slot_gribVlm(void);
        void slot_gribSailsDoc(void);

        void slot_gribPlay(void);

        void slot_loadEstimeParam(void);

        void slot_updateLockIcon(QString ic);

        void manageToolbarBreak();
signals:
        void estimeParamChanged(void);
        void gribZygrib(void);
        void gribVlm(void);
        void gribSailsDoc(void);

    private:
        MainWindow * mainWindow;
        myCentralWidget * centralWidget;

        MyToolBar * gribToolBar;
//        MyToolBar * miscToolBar;
        MyToolBar * mapToolBar;
        MyToolBar * estimeToolBar;
        MyToolBar * boatToolBar;
        MyToolBar * barrierToolBar;

        QList<MyToolBar*> toolBarList;

        QAction* init_Action(QString title, QString shortcut, QString statustip,QString iconFileName, QToolBar *toolBar);
        QSize iconSize;
};

#endif // TOOLBAR_H
