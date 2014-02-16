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

#ifndef DIALOGVLMGRIB_CTRL_H
#define DIALOGVLMGRIB_CTRL_H

#include <QStringList>

#include "inetClient.h"
#include "class_list.h"

class DialogVlmGrib_ctrl: public QObject, public inetClient
{
    Q_OBJECT
    public:
        DialogVlmGrib_ctrl(MainWindow * mainWindow,myCentralWidget * centralWidget,inetConnexion * inet);

        void requestFinished (QByteArray);

        void downloadGrib(void);
        void exitDialog(void);

    public slots:
        void slot_abort();

    signals:
        void signalGribFileReceived(QString);

    private:
        DialogVlmGrib_view * view;
        myCentralWidget * centralWidget;

        QString filename;
        QStringList lst_fname;

        bool doRequest(int reqType);
        QStringList parseFolderListing(QString data);
        bool gribFileReceived(QByteArray * content);

        void updateList(void);

};

#endif // DIALOGVLMGRIB_CTRL_H
