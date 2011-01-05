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
#include "mycentralwidget.h"
#include "MainWindow.h"
#include "Projection.h"
#include "route.h"
#include "POI.h"
#include "boatVLM.h"

#define VERSION_NUMBER    1
#define DOM_FILE_TYPE     "qtVLM_config"
#define ROOT_NAME         "qtVLM_POI"
#define VERSION_NAME      "Version"
/* ROUTE */
#define ROUTE_GROUP_NAME  "ROUTE"
#define ROUTE_NAME        "name"
#define ROUTE_BOAT        "boat"
#define ROUTE_START       "startFromBoat"
#define ROUTE_DATEOPTION  "startTimeOption"
#define ROUTE_DATE        "startTime"
#define ROUTE_WIDTH       "width"
#define ROUTE_COLOR_R     "color_red"
#define ROUTE_COLOR_G     "color_green"
#define ROUTE_COLOR_B     "color_blue"
#define ROUTE_LIVE        "liveUpdate"
#define ROUTE_FROZEN      "frozen"
#define ROUTE_HIDEPOIS    "hidePois"
#define ROUTE_MULTVAC     "vacStep"
#define ROUTE_HIDDEN      "hidden"
#define ROUTE_VBVMG_VLM   "vbvmg-vlm"
/* POI */
#define POI_GROUP_NAME    "POI"
#define POI_NAME          "name"
#define LAT_NAME          "Lat"
#define LON_NAME          "Lon"
#define LON_NAME_OLD      "Pass"
#define WPH_NAME          "Wph"
#define TYPE_NAME         "type"
#define TSTAMP_NAME       "timeStamp"
#define USETSTAMP_NAME    "useTimeStamp"
#define POI_ROUTE         "route"
#define POI_NAVMODE       "NavMode"
#define POI_LABEL_HIDDEN  "LabelHidden"

xml_POIData::xml_POIData(Projection * proj,MainWindow * main, myCentralWidget * parent)
: QWidget(parent)
{
    this->proj=proj;
    this->main=main;
    this->parent=parent;

    /* signals */
    connect(parent,SIGNAL(writePOIData(QList<ROUTE*>&,QList<POI*>&,QString)),
            this,SLOT(slot_writeData(QList<ROUTE*>&,QList<POI*>&,QString)));
    connect(parent,SIGNAL(readPOIData(QString)),this,SLOT(slot_readData(QString)));
    connect(parent,SIGNAL(importZyGrib()),this,SLOT(slot_importZyGrib()));

    connect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));
    this->loaded=false;
}

void xml_POIData::slot_writeData(QList<ROUTE*> & route_list,QList<POI*> & poi_list,QString fname)
{
     if(!loaded) return;
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

     /* ROUTE */
     QListIterator<ROUTE*> h (route_list);
     while(h.hasNext())
     {
          ROUTE * route=h.next();
          if(route->isImported()) continue;
          if(!route->getBoat()->getStatus()) continue; //if boat has been deactivated do not save route
          group = doc.createElement(ROUTE_GROUP_NAME);
          root.appendChild(group);

          tag = doc.createElement(ROUTE_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(route->getName().toUtf8().toBase64());
          tag.appendChild(t);

          if(route->getBoat()!=NULL)
          {
              tag = doc.createElement(ROUTE_BOAT);
              group.appendChild(tag);
              t = doc.createTextNode(QString().setNum(route->getBoat()->getId()));
              tag.appendChild(t);
          }

          tag = doc.createElement(ROUTE_START);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getStartFromBoat()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_DATEOPTION);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getStartTimeOption()));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_MULTVAC);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getMultVac()));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_DATE);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getStartTime().toUTC().toTime_t()));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_WIDTH);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getWidth()));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_COLOR_R);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getColor().red()));
          tag.appendChild(t);
          tag = doc.createElement(ROUTE_COLOR_G);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getColor().green()));
          tag.appendChild(t);
          tag = doc.createElement(ROUTE_COLOR_B);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getColor().blue()));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_LIVE);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->isLive()?1:0));
          tag.appendChild(t);


          tag = doc.createElement(ROUTE_FROZEN);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getFrozen()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_HIDEPOIS);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getHidePois()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_HIDDEN);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getHidden()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(ROUTE_VBVMG_VLM);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(route->getUseVbvmgVlm()?1:0));
          tag.appendChild(t);

          tag.appendChild(t);
     }

     /* POI */
     QListIterator<POI*> i (poi_list);
     while(i.hasNext())
     {
          POI * poi = i.next();
          if(poi->getRoute()!=NULL && poi->getRoute()->isImported()) continue;
          if(poi->isPartOfTwa()) continue;
          group = doc.createElement(POI_GROUP_NAME);
          root.appendChild(group);

          tag = doc.createElement(POI_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(poi->getName().toUtf8().toBase64());
          tag.appendChild(t);

          tag = doc.createElement(TYPE_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getType()));
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

          tag = doc.createElement(TSTAMP_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getTimeStamp()));
          tag.appendChild(t);

          tag = doc.createElement(USETSTAMP_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getUseTimeStamp()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(POI_LABEL_HIDDEN);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getMyLabelHidden()?1:0));
          tag.appendChild(t);

          tag = doc.createElement(POI_ROUTE);
          group.appendChild(tag);
          if(poi->getRoute()!=NULL)
            t = doc.createTextNode(poi->getRoute()->getName().toUtf8().toBase64()); //do not use poi->routeName since route->name might have changed */
          else
              t = doc.createTextNode(QString("").toUtf8().toBase64());
          tag.appendChild(t);

          tag = doc.createElement(POI_NAVMODE);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(poi->getNavMode()));
          tag.appendChild(t);

          tag.appendChild(t);
     }

     QFile file(fname);
     if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
         return ;

     QTextStream out(&file);

     doc.save(out,4);

     file.close();
}

void xml_POIData::slot_importZyGrib(void)
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
                POI * poi = new POI(QString(QByteArray::fromBase64(lst[1].toUtf8())), POI_TYPE_POI,
                                    lst[3].toFloat(),lst[2].toFloat(),proj,main,parent,lst[5].toInt(),-1,false,NULL);

                emit addPOI_list(poi);
                /*removing zyGrib POI*/
                if (rep == QMessageBox::Yes)
                    settings.remove(key);
            }
            QMessageBox::information (this,
            tr("POI de zyGrib").arg(slist.size()),
            tr("POI importes, pensez a sauvegarder les POI"));
        }
     }
     else
     {
         QMessageBox::information (this,
            tr("POI de zyGrib").arg(slist.size()),
            tr("Pas de POI de zyGrib trouves"));
     }
     settings.endGroup();
}

void xml_POIData::slot_readData(QString fname)
{
     this->loaded=true;
     QString  errorStr;
     int errorLine;
     int errorColumn;
     bool hasVersion = false;
     int version=VERSION_NUMBER;

     QFile file(fname);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
         return ;

     //QTextStream in(&file);

     qWarning() << "Loading POI";

     QDomDocument doc;
     if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
     {
        QMessageBox::warning(0,QObject::tr("Lecture de parametre bateau"),
             QString(tr("Erreur ligne %1, colonne %2:\n%3"))
             .arg(errorLine)
             .arg(errorColumn)
             .arg(errorStr));
        return ;
     }

     QDomElement root = doc.documentElement();
     if(root.tagName() != ROOT_NAME)
     {
         qWarning() << "Wrong root name: " << root.tagName();
         return ;
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
                      return ;
                  }
                  else
                  {
                      hasVersion = true;
                  }
              }
          }
          else if(node.toElement().tagName() == ROUTE_GROUP_NAME)
          {
              ROUTE * route = parent->addRoute();
              route->setFrozen(true);
              route->setBoat(NULL);
              QColor routeColor=Qt::red;
              bool toBeFreezed=false;
              subNode = node.firstChild();
              bool invalidRoute=true;
              while(!subNode.isNull())
              {
                  if(subNode.toElement().tagName() == ROUTE_NAME)
                  {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          route->setName(QString(QByteArray::fromBase64(dataNode.toText().data().toUtf8())));
                  }
                  if(subNode.toElement().tagName() == ROUTE_BOAT)
                  {
//a modif pour les real boat ?
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                       {
                           if(parent->getBoats())
                           {
                               QListIterator<boatVLM*> b (*parent->getBoats());

                               while(b.hasNext())
                               {
                                   boatVLM * boat = b.next();
                                   if(boat->getStatus())
                                   {
                                       if(boat->getId()==dataNode.toText().data().toInt())
                                       {
                                           route->setBoat(boat);
                                           invalidRoute=false;
                                           break;
                                       }
                                   }
                               }
                           }
                       }
                  }
                  if(subNode.toElement().tagName() == ROUTE_START)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setStartFromBoat(dataNode.toText().data().toInt()==1);
                  }
                  if(subNode.toElement().tagName() == ROUTE_DATEOPTION)
                  {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          route->setStartTimeOption(dataNode.toText().data().toInt());
                  }
                  if(subNode.toElement().tagName() == ROUTE_MULTVAC)
                  {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          route->setMultVac(dataNode.toText().data().toInt());
                  }
                  if(subNode.toElement().tagName() == ROUTE_DATE)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                       {
                           QDateTime date;
                           date.setTime_t(dataNode.toText().data().toInt());
                           route->setStartTime(date);
                       }
                  }
                  if(subNode.toElement().tagName() == ROUTE_WIDTH)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setWidth(dataNode.toText().data().toFloat());
                  }
                  if(subNode.toElement().tagName() == ROUTE_COLOR_R)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                       {
                           routeColor.setRed(dataNode.toText().data().toInt());
                           route->setColor(routeColor);
                       }
                  }
                  if(subNode.toElement().tagName() == ROUTE_COLOR_G)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                       {
                           routeColor.setGreen(dataNode.toText().data().toInt());
                           route->setColor(routeColor);
                       }
                  }
                  if(subNode.toElement().tagName() == ROUTE_COLOR_B)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                       {
                           routeColor.setBlue(dataNode.toText().data().toInt());
                           route->setColor(routeColor);
                       }
                  }

                  if(subNode.toElement().tagName() == ROUTE_LIVE)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setLive(dataNode.toText().data().toInt()==1);
                  }
                  if(subNode.toElement().tagName() == ROUTE_FROZEN)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           toBeFreezed=(dataNode.toText().data().toInt()==1);
                  }


                  if(subNode.toElement().tagName() == ROUTE_HIDEPOIS)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setHidePois(dataNode.toText().data().toInt()==1);
                  }

                  if(subNode.toElement().tagName() == ROUTE_HIDDEN)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setHidden(dataNode.toText().data().toInt()==1);
                  }
                  if(subNode.toElement().tagName() == ROUTE_VBVMG_VLM)
                  {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           route->setUseVbVmgVlm(dataNode.toText().data().toInt()==1);
                  }

                  subNode = subNode.nextSibling();
              }
              if(invalidRoute) /*route->boat does not exist anymore*/
              {
                  route->setBoat(NULL);
              }
              if(!toBeFreezed)
                  route->setFrozen(false);
          }
          else if(node.toElement().tagName() == POI_GROUP_NAME)
          {
              subNode = node.firstChild();
              QString name="";
              QString routeName="";
              float lon=-1, lat=-1,wph=-1;
              int type = -1;
              int tstamp=-1;
              bool useTstamp=false;
              bool labelHidden=false;
              int navMode=0;

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
                   if(subNode.toElement().tagName() == LON_NAME)
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

                   if(subNode.toElement().tagName() == POI_LABEL_HIDDEN)
                   {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           labelHidden = dataNode.toText().data().toInt()==1;
                   }

                   if(subNode.toElement().tagName() == POI_ROUTE)
                   {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                          routeName = QString(QByteArray::fromBase64(dataNode.toText().data().toUtf8()));
                   }

                   if(subNode.toElement().tagName() == POI_NAVMODE)
                   {
                       dataNode = subNode.firstChild();
                       if(dataNode.nodeType() == QDomNode::TextNode)
                           navMode = dataNode.toText().data().toInt();
                   }


                   subNode = subNode.nextSibling();
              }
              if(!name.isEmpty() && lat!=-1 && lon != -1)
              {
                   qWarning() << "POI info present => create item " << name << " "
                        << lat << "," << lon << "@" << wph ;
                   if(type==-1) type=POI_TYPE_POI;
                   POI * poi = new POI(name,type,lat,lon,proj,main,parent,wph,tstamp,useTstamp,NULL);
                   poi->setRouteName(routeName);
                   poi->setNavMode(navMode);
                   poi->setMyLabelHidden(labelHidden);
                   emit addPOI_list(poi);
              }
              else
                   qWarning() << "Incomplete POI info " << name << " "
                        << lat << "," << lon << "@" << wph ;
          }          
          node = node.nextSibling();
     }
    parent->assignPois();
     if(!hasVersion)
     {
         qWarning("no version");
// remettre un appel a centralWidget pour effacer la liste ? idem dans ts les retour d erreur
     }

}

