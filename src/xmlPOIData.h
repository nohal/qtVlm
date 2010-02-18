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

#ifndef XML_POI_DATA_H
#define XML_POI_DATA_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

class xml_POIData;

#include "POI.h"
#include "Projection.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "route.h"

class xml_POIData: public QWidget
{
    Q_OBJECT

    public:
        xml_POIData(Projection * proj,MainWindow * main, myCentralWidget * parent);

    public slots:
        void slot_writeData(QList<ROUTE*> & route_list,QList<POI*> & poi_list,QString fname);
        void slot_readData(QString fname);
        void slot_importZyGrib();

    signals:
        void addRoute_list(ROUTE * route);
        void delRoute_list(ROUTE * route);
        void addPOI_list(POI * poi);
        void delPOI_list(POI * poi);

    private:
        Projection * proj;
        MainWindow * main;
        myCentralWidget * parent;
};

#endif
