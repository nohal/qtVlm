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

#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QDomDocument>
#include <QFile>

#include "dataDef.h"

#include "XmlFile.h"


/******************************************************
 * Search a node in a given file
 ******************************************************/
QDomNode * XmlFile::get_dataNodeFromDisk(QString fileName,QString nodeName) {
    QDomNode * resNode=NULL;
    /* scan all nodes to find 'nodeName' node*/
    QDomElement * root=get_fisrtDataNodeFromDisk(fileName);
    if(!root)
        return NULL;
    QDomNode node = root->firstChild();
    while(!node.isNull()) {
        //qWarning() << "Node: " << node.toElement().tagName();
        if(node.toElement().tagName() == nodeName) {
            resNode=new QDomNode(node);
            break;
        }
        node = node.nextSibling();
    }
    return resNode;
}

QDomElement * XmlFile::get_fisrtDataNodeFromDisk(QString fileName) {
    QString  errorStr;
    int errorLine;
    int errorColumn;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text )) {
        /*QMessageBox::warning(0,QObject::tr("Reading XML file"),
                             QString(QObject::tr("File %1 can't be opened"))
                             .arg(fileName);*/
        return NULL;
    }

    QDomDocument doc;
    if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn)) {
        QMessageBox::warning(0,QString(QObject::tr("Reading XML file %1"))
                             .arg(fileName),
                             QString(QObject::tr("Error line %1, column %2:\n%3"))
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return NULL;
    }

    return new QDomElement(doc.documentElement());
}

/******************************************************
 * Save node date in a given file
 * if newNode=NULL, we erase previous node data
 * if file doesn't exists or contains error, we create it
 ******************************************************/
bool XmlFile::set_dataNodeOnDisk(QString fileName,QString nodeName,QDomNode * newNode,QString rootName) {
    QString  errorStr;
    int errorLine;
    int errorColumn;
    /* open file and try to remove node */
    QFile file(fileName);

    bool needNewDoc=false;

    QDomDocument doc;
    QDomElement root;

    /* if file doesn't exists ==> create a new one */
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text )) {
        //qWarning() << "Need a new file";
        needNewDoc=true;
    }
    else {
        if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn)) {
            QMessageBox::warning(0,QString(QObject::tr("Reading XML file %1"))
                                 .arg(fileName),
                                 QString(QObject::tr("Error line %1, column %2:\n%3"))
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
            /* Error reading file => create a new one */
            needNewDoc=true;
            //qWarning() << "file has errors => new file";
            file.close();
        }
        else {
            file.close();
            root = doc.documentElement();
            /* all is ok, let's find existing node with same name */
            bool found=false;
            QDomNode node=root.firstChild();
            while(!node.isNull()) {
                if(node.toElement().tagName() == nodeName) {
                    found=true;
                    break;
                }
                node = node.nextSibling();
            }

            if(found) { /* we have it => remove it from doc */
                //qWarning() << "Existing node found ==> remove it";
                root.removeChild(node);
            }
        }

    }

    if(needNewDoc) { /* file doesn't exist or contains error */
        doc = QDomDocument(DOM_FILE_TYPE);
        root = doc.createElement(rootName);
        doc.appendChild(root);
    }

    if(newNode) {
        root.appendChild(*newNode);
    }

    /* write back data in file */
    if (file.isOpen())
        file.close();
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    QTextStream out(&file);
    doc.save(out,4); /*write doc using 4 chars to indent node*/
    file.close();

    return true;
}

void XmlFile::saveRoot(QDomElement * root,QString fname) {
    QDomDocument doc(DOM_FILE_TYPE);
    doc.appendChild(*root);
    QFile file(fname);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        return ;
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
}

