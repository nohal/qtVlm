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
#include "dataDef.h"
//===================================================================
#include "ui_Routage_Editor.h"
class DialogRoutage : public QDialog, public Ui::ROUTAGE_Editor_ui
{ Q_OBJECT
    public:
        DialogRoutage(ROUTAGE *route, myCentralWidget *parent,POI *endPOI=NULL);
        ~DialogRoutage();
        void done(int result);
    public slots:
        void GybeTack(int i);
        void slot_screenResize();
    signals:

    private:
        ROUTAGE   *routage;
        myCentralWidget *parent;
        InputLineParams *inputTraceColor;

    private slots:
        void slot_default();
};

#endif
