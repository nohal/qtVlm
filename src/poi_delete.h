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

#ifndef POI_DELETE_H
#define POI_DELETE_H

#include <QDialog>

#include "ui_poi_delete.h"

class POI_delete : public QDialog, public Ui::POI_delete_ui
{
    Q_OBJECT
    public:
        POI_delete(QWidget * parent = 0);
        void done(int result);
        int getResult(void) { return mask; }

    public slots:
        void do_chkAll(void);
        void do_chkNone(void);

     private:
        int mask;

};



#endif // POI_DELETE_H
