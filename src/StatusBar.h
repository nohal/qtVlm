/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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


#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>

#include "class_list.h"
#include "dataDef.h"

class StatusBar : public QStatusBar
{
    Q_OBJECT
    public:
        StatusBar(MainWindow * mainWindow);

        void showGribData(double x,double y);
        void showSelectedZone(double x0, double y0, double x1, double y1);
    

        void update_eta(QDateTime eta_dtm);
        void clear_eta();
signals:
    
    public slots:

    private:
        MainWindow * mainWindow;
        myCentralWidget * my_centralWidget;


        QString compute_dataTxt(DataManager * dataManager, MapDataDrawer* mapDrawer, QMap<int,QStringList> * mapDataTypes,
                                int mode, int levelType, int levelValue, double x, double y);

        QLabel       *labelGrib;
#ifndef __ANDROID__
        QLabel       *labelEta;
        QLabel       *labelOrtho;
        QLabel       *separator;
#endif
    
};

#endif // STATUSBAR_H
