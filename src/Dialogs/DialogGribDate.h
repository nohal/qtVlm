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

#ifndef DIALOG_GRIBDATE_H
#define DIALOG_GRIBDATE_H

#include <set>
#include <vector>
#ifdef QT_V5
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif
#include "ui_dialog_gribDate.h"
#include "class_list.h"
class DialogGribDate : public QDialog, public Ui::dialog_gribDate_ui
{
    Q_OBJECT
    public:
        DialogGribDate(myCentralWidget * parent);
        void done(int res);
        void showDialog(time_t current,std::set<time_t>  * listGribDates,time_t * result);

        void slot_screenResize();
public slots:
        void listChanged(int index);
        void paramChanged(QDateTime date);

    private:
        std::vector<time_t> listGribDates;
        time_t startTime;
        time_t * result;
        bool listIsChanging;
};

#endif // DIALOG_GRIBDATE_H
