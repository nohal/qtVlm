/**********************************************************************
qtVlm
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

#ifndef XMLFILE_H
#define XMLFILE_H

#include <QString>
#include <QDomDocument>

#include "class_list.h"

class XmlFile
{
    public:
        static QDomNode * get_dataNodeFromDisk(QString fileName, QString nodeName);
        static QDomElement * get_fisrtDataNodeFromDisk(QString fileName);
        static bool set_dataNodeOnDisk(QString fileName, QString nodeName, QDomNode * node,QString rootName);
        static void saveRoot(QDomElement * root,QString fname);

};

#endif // XMLFILE_H
