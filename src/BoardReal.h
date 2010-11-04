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

#ifndef BOARDREAL_H
#define BOARDREAL_H

#include <QApplication>
#include <QMainWindow>

#include "class_list.h"
#include "ui_BoardReal.h"

class boardReal: public QWidget , public Ui::boardReal_ui
{ Q_OBJECT
    public:
        boardReal(MainWindow * mainWin, board * parent);

        void boatUpdated(void);
        void setChangeStatus(bool);

    public slots:
        void disp_boatInfo();
        void paramChanged(void);
        void slot_hideShowCompass();

    signals:
        void showMessage(QString,int);

    protected:
        void contextMenuEvent(QContextMenuEvent  *);

    private:
        QMainWindow * mainWin;
        board * parent;

        /* contextual menu */
        QMenu *popup;
        QAction * ac_showHideCompass;
};

#endif // BOARDREAL_H
