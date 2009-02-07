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

#include <QMessageBox>
#include <QSettings>
#include <QDebug>

#include "xmlPOIData.h"

#define VERSION_NUMBER    1
#define DOM_FILE_TYPE     "qtVLM_config"
#define ROOT_NAME         "qtVLM_POI"
#define VERSION_NAME      "Version"
#define POI_GROUP_NAME    "POI"
#define POI_NAME          "name"
#define LAT_NAME          "Lat"
#define LON_NAME          "Lon"
#define LON_NAME_OLD      "Pass"
#define WPH_NAME          "Wph"
#define TYPE_NAME         "type"
#define TSTAMP_NAME       "timeStamp"
#define USETSTAMP_NAME    "useTimeStamp"

xml_POIData::xml_POIData(Projection * proj,QWidget * main, QWidget * parent)
: QWidget(parent)
{
    this->proj=proj;
    this->main=main;
    this->parent=parent;
}

bool xml_POIData::writeData(QList<POI*> & poi_list,QString fname)
{
     QDomDocument doc(DOM_FILE_TYPE);
     QDomElement root = doc.createElement(ROOT_NAME);
     doc.appendChild(root);

     QDomElement group;
     QDomElement tag;
     QDomText t;

     group = doc.createElement(VERSION_NAME);
     root.appendChild(group);
     t = doc.createTextNode(QString().setNum(VERSION_NUMBER));
     group.appendChild(t);

     QListIterator<POI*> i (poi_list);
     while(i.hasNext())
     {
          POI * poi = i.next();

          group = doc.createElement(POI_GROUP_NAME);
          root.appendChild(group);

          tag = doc.createElement(POI_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(poi->getName().toUtf8().toBase64());
          tag.appendChild(t);

          tag = doc.createElement(LAT_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getLatitude()));
          tag.appendChild(t);

          tag = doc.createElement(LON_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getLongitude()));
          tag.appendChild(t);

          tag = doc.createElement(WPH_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getWph()));
          tag.appendChild(t);

          tag = doc.createElement(TYPE_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getType()));
          tag.appendChild(t);

          tag = doc.createElement(TSTAMP_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getTimeStamp()));
          tag.appendChild(t);

          tag = doc.createElement(USETSTAMP_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getUseTimeStamp()?1:0));
          tag.appendChild(t);

          tag.appendChild(t);
     }

     QFile file(fname);
     if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
         return false;

     QTextStream out(&file);

     doc.save(out,4);

     file.close();

     return true;
}

void xml_POIData::importZyGrib(QList<POI*> & poi_list)
{
    /* let's see if we have some zyGrib POI here */
     QSettings settings("zyGrib");
     settings.beginGroup("poi");
     QString key;
     QStringList slist = settings.childKeys();
     if(slist.size()>0)
     {
        int rep = QMessageBox::question (this,
            tr("%1 POI de zyGrib trouve").arg(slist.size()),
            tr("Supprimer les POI de zyGrib apres importation?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        QString key;
        QString serialized;
        if (rep == QMessageBox::Yes || rep == QMessageBox::No)
        {
            for (int i=0; i<slist.size(); ++i)
            {
                key = slist.at(i);
                serialized=settings.value(key, "").toString();
                if(serialized.isEmpty())
                    continue;
                QStringList  lst = serialized.split(";");
                if(lst.size()<5)
                {
                    serialized+=";0;-1";
                                    }
                else if(lst.size()<6)
                {
                    serialized+=";-1";
                                    }
                lst = serialized.split(";");
                POI * poi = new POI(QString(QByteArray::fromBase64(lst[1].toUtf8())),
                                    lst[2].toFloat(),lst[3].toFloat(),proj,main,parent,
                                    lst[4].toInt(),lst[5].toInt(),-1,false);
                poi_list.append(poi);
                /*removing zyGrib POI*/
                if (rep == QMessageBox::Yes)
                    settings.remove(key);
            }
            QMessageBox::information (this,
            tr("POI de zyGrib").arg(slist.size()),
            tr("POI importer, pensez a sauvegarder les POI"));
        }
     }
     else
     {
         QMessageBox::information (this,
            tr("POI de zyGrib").arg(slist.size()),
            tr("Pas de POI de zyGrib trouv�e"));
     }
     settings.endGroup();
}

bool xml_POIData::readData(QList<POI*> & poi_list,QString fname)
{
     QString  errorStr;
     int errorLine;
     int errorColumn;
     bool hasVersion = false;
     bool needSave=false;
     int version=VERSION_NUMBER;

     poi_list.clear();

     QFile file(fname);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
         return false;

     //QTextStream in(&file);

     QDomDocument doc;
     if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
     {
        QMessageBox::warning(0,QObject::tr("Lecture de parametre bateau"),
             QString(tr("Erreur ligne %1, colonne %2:\n%3"))
             .arg(errorLine)
             .arg(errorColumn)
             .arg(errorStr));
        return false;
     }

     QDomElement root = doc.documentElement();
     if(root.tagName() != ROOT_NAME)
     {
         qWarning() << "Wrong root name: " << root.tagName();
         return false;
     }

     QDomNode node = root.firstChild();
     QDomNode subNode;
     QDomNode dataNode;

     while(!node.isNull())
     {
          if(node.toElement().tagName() == VERSION_NAME)
          {
              dataNode = node.firstChild();
              if(dataNode.nodeType() == QDomNode::TextNode)
              {
                  version = dataNode.toText().data().toInt();
                  if(version != VERSION_NUMBER && version !=0)
                  {
                      qWarning("Bad version");
                      poi_list.clear();;
                      return false;
                  }
                  else
                  {
                      hasVersion = true;
                  }
              }
          }
          else if(node.toElement().tagName() == POI_GROUP_NAME)
          {
              subNode = node.firstChild();
              QString name="";
              float lon=-1, lat=-1,wph=-1;
              int type = -1;
              int tstamp=-1;
              bool useTstamp=false;

              while(!subNode.isNull())
              {
                   if(subNode.toElement().tagName() == POI_NAME)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          name = QString(QByteArray::fromBase64(dataNode.toText().data().toUtf8()));
                   }
                   if(subNode.toElement().tagName() == LAT_NAME)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          lat = dataNode.toText().data().toFloat();
                   }
#warning should remove LON_NAME_OLD in a while
                   if(subNode.toElement().tagName() == LON_NAME
                      || subNode.toElement().tagName() == LON_NAME_OLD)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          lon = dataNode.toText().data().toFloat();
                   }
                   if(subNode.toElement().tagName() == WPH_NAME)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          wph = dataNode.toText().data().toFloat();
                   }
                   if(subNode.toElement().tagName() == TYPE_NAME)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          type = dataNode.toText().data().toInt();
                   }
#warning should remove V0 part
                    if(hasVersion && version==0)
                    {
                        tstamp=-1;
                        useTstamp=false;
                    }
                    else
                    {
                        if(subNode.toElement().tagName() == TSTAMP_NAME)
                        {
                            dataNode = subNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                                tstamp = dataNode.toText().data().toInt();
                        }
                        if(subNode.toElement().tagName() == USETSTAMP_NAME)
                        {
                            dataNode = subNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                                useTstamp = dataNode.toText().data().toInt()==1;
                        }
                    }

                   subNode = subNode.nextSibling();
              }
              if(!name.isEmpty() && lat!=-1 && lon != -1 && type != -1)
              {
                   qWarning() << "POI info present => create item " << name << " " 
                        << lat << "," << lon << "@" << wph << " -type: " << type;
                   POI * poi = new POI(name,lon,lat,proj,main,parent,type,wph,tstamp,useTstamp);
                   poi_list.append(poi);
              }
              else
                   qWarning("Incomplete boat info");
          }
          node = node.nextSibling();
     }

    if(needSave)
        writeData(poi_list,fname);

     if(hasVersion)
     {
         return true;
     }
     else
     {
         qWarning("no version");
         poi_list.clear();
         return false;
     }
}

