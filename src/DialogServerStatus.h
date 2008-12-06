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

#ifndef DIALOGSERVERSTATUS_H
#define DIALOGSERVERSTATUS_H

#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QtNetwork>
#include <QBuffer>

class DialogServerStatus : public QDialog
{ Q_OBJECT
    public:
        DialogServerStatus();
        ~DialogServerStatus();
    
    public slots:
        void slotBtOK();
        void slotBtCancel();
        void requestFinished (int id, bool error);
    
    
    private:
        QString getNoaaRunDate(QStringList lsbuf);
        QString getNoaaRunHour(QStringList lsbuf);
        
        QFrame *frameGui;
        QGridLayout *layout;
        
        QPushButton *btOK;
        
        QHttp    *http;
        QString  host;
        QBuffer  ioBuffer;
        int      idHttpGetFile;
        
        QTime   timeLoad;
        QLabel *lbResponseStatus;
        QLabel *lbResponseTime;
        QLabel *lbRunDate;
        QLabel *lbRunHour;
        QLabel *lbMessage;
        
        QFrame * createFrameGui(QWidget *parent);
};


#endif
