/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGEDITBARRIER_H
#define DIALOGEDITBARRIER_H

#include <QDialog>
#include <QList>

#include "class_list.h"
#include "ui_DialogEditBarrier.h"

class DialogEditBarrier: public QDialog, public Ui::DialogEditBarrier_ui
{ Q_OBJECT
    public:
        DialogEditBarrier(MainWindow *parent);
        ~DialogEditBarrier(void);

        void initDialog(BarrierSet *barrierSet, QList<boat *> boatList);

        void done(int result);

    public slots:
        void slot_chgColor(void);

        void slot_screenResize();
private:
        BarrierSet * barrierSet;
        QColor color;

        void updateBtnColor(void);
};

#endif // DIALOGEDITBARRIER_H
