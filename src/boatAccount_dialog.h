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



#ifndef BOATACCOUNT_DIALOG_H
#define BOATACCOUNT_DIALOG_H

#include <QDialog>

#include "ui_boatAccount_dialog.h"
#include "boatAccount.h"
#include "Projection.h"
#include "xmlBoatData.h"

class boatAccount_dialog : public QDialog, public Ui::boatAccount_dialog
{
    Q_OBJECT
    public:
        boatAccount_dialog(QList<boatAccount*> & acc_list, Projection * proj,QWidget * main, QWidget * parent = 0);
        ~boatAccount_dialog();
        void done(int result);
        void initList(QList<boatAccount*> & acc_list);

    public slots:
        void slot_addBoat(void);
        void slot_delBoat(void);
        void slot_chgBoat(void);
        void slot_selectItem(QListWidgetItem * item);
        void slot_accHasChanged(void);
        void chkAlias_changed(int);
        void chkPolar_changed(int);

    signals:
        void vlmSync(void);
        void showMessage(QString str);
        void accountListUpdated(void);

    private:
        QListWidgetItem * blank;
        QList<boatAccount*> * acc_list;
        Projection * proj;
        QWidget * main, * parent;
        xml_boatData * xmlData;

};



#endif
