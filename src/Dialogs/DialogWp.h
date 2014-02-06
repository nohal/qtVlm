/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGWP_H
#define DIALOGWP_H

#include "class_list.h"
#include "ui_WP_dialog.h"

class DialogWp: public QDialog, public Ui::WP_dialog_ui
{ Q_OBJECT
    public:
        DialogWp(myCentralWidget *parent);
        void show_WPdialog(boat * boat);
        void setLocked(const bool &locked);
        void show_WPdialog(POI * poi, boat * boat);
        ~DialogWp();

    public slots:
        void chgLat();
        void chgLon();
        void doPaste();
        void doCopy();
        void done(int result);
        void doClearWP();
        void doSelPOI();

        void slot_screenResize();
        void slot_selectPOI(POI *poi);
signals:
        void selectPOI(void);

    private:
        boat * currentBoat;
        void initDialog(double WPLat,double WPLon,double WPHd);
};

#endif // DIALOGWP_H
