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

#ifndef XML_BOAT_DATA_H
#define XML_BOAT_DATA_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

#include "boatAccount.h"
#include "Projection.h"

class xml_boatData: public QWidget
{Q_OBJECT

      public:
            xml_boatData(Projection * proj,QWidget * main, QWidget * parent=0);
            bool writeBoatData(QList<boatAccount*> & boat_list,QString fname);
            bool readBoatData(QList<boatAccount*> & boat_list,QString fname);

	  signals:
            void showMessage(QString msg);
            
      private:
            Projection * proj;
            QWidget * main, * parent;
      
      
};

#endif
