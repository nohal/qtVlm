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

#ifndef DIALOGVLM_GRIB_H
#define DIALOGVLM_GRIB_H

#include <QDialog>
#include <QMessageBox>
#include <QRadioButton>

#include "inetConnexion.h"
#include "ui_DialogVLM_grib.h"

class DialogVLM_grib : public QDialog, public Ui::DialogVLM_grib_ui
{
    Q_OBJECT
    public:
        DialogVLM_grib(QWidget * main,QWidget * parent = 0);
        void done(int res);
        void showDialog(void);

    public slots:
        void requestFinished (int,QByteArray);

    signals:
        void signalGribFileReceived(QString);

    private:
        /* http connection */
        inetConnexion * conn;
        QRadioButton * listRadio[4];
        QMessageBox * waitBox;

        QString filename;

        bool doRequest(int reqType);
        int parseFolderListing(QString data);
        bool gribFileReceived(QByteArray * content);
};

#endif // DIALOGVLM_GRIB_H
