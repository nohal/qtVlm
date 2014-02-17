/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOG_VIEW_PC_H
#define DIALOG_VIEW_PC_H

#ifdef QT_V5
#include <QtWidgets/QDialog>
#include "QScroller"
#else
#include <QDialog>
#endif

#include "class_list.h"

#ifdef QT_V5
#define INIT_DIALOG { \
    setupUi(this); \
    QScroller::grabGesture(this->scrollArea->viewport()); \
    initDialog(); \
}
#else
#define INIT_DIALOG { \
    setupUi(this); \
    initDialog(); \
}
#endif

class Dialog_view_pc: public QDialog
{ Q_OBJECT
    public:
        Dialog_view_pc(myCentralWidget * centralWidget);

        void initDialog(void);

    public slots:
        void slot_screenResize(void);

    private:
        myCentralWidget * cWt;
};

#endif // DIALOG_VIEW_PC_H
