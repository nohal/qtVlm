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

#ifndef GRIBVALIDATION_H
#define GRIBVALIDATION_H

#include <QDialog>

#include "ui_gribValidation.h"

#include "class_list.h"

class gribValidation: public QDialog, public Ui::gribValidation
{    Q_OBJECT
    public:
        gribValidation(myCentralWidget * my_centralWidget,MainWindow * mainWindow);
        ~gribValidation();
        void done(int result);
        void setMode(int mode);

    public slots:
        void inputChanged(void);
        void doNow(void);
        void interpolationChanged(int);

private:
        myCentralWidget * my_centralWidget;
        MainWindow * mainWindow;
        int curMode;

};

#endif // GRIBVALIDATION_H