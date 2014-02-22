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

#ifndef DIALOGCHOOSEMULTIPLE_VIEW_H
#define DIALOGCHOOSEMULTIPLE_VIEWH

#include <QList>
#include <QVariant>
#include <QString>

#include "class_list.h"

struct dataItem {
    QString str;
    QVariant dataVariant;
    bool checked;
};

class DialogChooseMultiple_view
{
    public:
        DialogChooseMultiple_view(myCentralWidget *centralWidget);
        virtual ~DialogChooseMultiple_view(void) { }

        virtual QList<dataItem> launchDialog(QString dialogTitle, QString title, QList<dataItem> dataItemList) =0;

    protected:
        myCentralWidget *centralWidget;
};

#endif // DIALOGCHOOSEMULTIPLE_VIEW_H
