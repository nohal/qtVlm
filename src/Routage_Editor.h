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

#ifndef ROUTAGE_EDITOR_H
#define ROUTAGE_EDITOR_H

#include "class_list.h"
#include "ui_Routage_Editor.h"

//===================================================================
class ROUTAGE_Editor : public QDialog, public Ui::ROUTAGE_Editor_ui
{ Q_OBJECT
    public:
        ROUTAGE_Editor(ROUTAGE *route, myCentralWidget *parent);
        ~ROUTAGE_Editor();
        void done(int result);

    public slots:

    signals:

    private:
        ROUTAGE   *routage;
        myCentralWidget *parent;
        bool  modeCreation;
        InputLineParams *inputTraceColor;

private slots:
    void on_windForced_toggled(bool checked);
};

#endif