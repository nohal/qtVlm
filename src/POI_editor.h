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

#ifndef POI_EDITOR_H
#define POI_EDITOR_H

class POI_Editor;

#include "POI.h"
#include "Projection.h"
#include "ui_POI_editor.h"
#include "MainWindow.h"
#include "mycentralwidget.h"

//===================================================================
class POI_Editor : public QDialog, public Ui::POI_editor_ui
{ Q_OBJECT
    public:
        POI_Editor(MainWindow * main,myCentralWidget * parent);
        void done(int result);

    public slots:
        void btDeleteClicked();
        void btPasteClicked();
        void btCopyClicked();
        void btSaveWPClicked();
        void chkTStamp_chg(int);
        void nameHasChanged(QString);
        void editPOI(POI *);
        void newPOI(float lon, float lat,Projection *proj, boatAccount *);
        void lat_deg_chg(int);
        void lat_min_chg(double);
        void lon_deg_chg(int);
        void lon_min_chg(double);
        void lat_val_chg(double);
        void lon_val_chg(double);
        void type_chg(int);

    signals:
        void addPOI_list(POI*);
        void delPOI_list(POI*);
        void doChgWP(float,float,float);
        void updateRoute();

    private:
        POI   *poi;
        bool  modeCreation;
        bool lock;
        int oldType;

        MainWindow * main;
        myCentralWidget * parent;

        void initPOI(void);
        float getValue(int type);
        void setValue(int type,float val);
        void data_chg(int type);
        void val_chg(int type);
};

#endif
