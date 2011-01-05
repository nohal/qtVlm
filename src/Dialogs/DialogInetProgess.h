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

#ifndef DIALOGINETPROGESS_H
#define DIALOGINETPROGESS_H

#include "ui_inetConn_progessDialog.h"

class DialogInetProgess : public QDialog, public Ui::inetConn_progressDialog_ui
{
    Q_OBJECT
    public:
        DialogInetProgess(QWidget * parent = 0);
        void showDialog(QString name);
        void hideDialog(void);

    public slots:
        void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // DIALOGINETPROGESS_H
