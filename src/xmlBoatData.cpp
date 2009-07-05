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
#include <QByteArray>
#include <QDebug>

#include "xmlBoatData.h"

#define VERSION_NUMBER    1
#define DOM_FILE_TYPE     "qtVLM_config"
#define ROOT_NAME         "qtVLM_boat"
#define VERSION_NAME      "Version"
/* BOAT data */
#define BOAT_GROUP_NAME   "Boat"
#define LOGIN_NAME        "Login"
#define PASS_NAME         "Pass"
#define ACTIVATED_NAME    "Activated"
#define POLAR_NAME        "Polar"
#define LOCK_NAME         "Lock"
#define POLAR_CHK_NAME    "UsePolar"
#define ALIAS_CHK_NAME    "UseAlias"
#define ALIAS_NAME        "Alias"
#define ZOOM_NAME         "Zoom"
#define ESTIME_NAME       "Estime"
/* RACE DATA */
#define RACE_GROUP_NAME   "Race"
#define RACEID_NAME       "raceId"
#define OPPLIST_NAME      "oppList"
#define VACLEN_NAME       "vacLen"

#define OLD_DOM_FILE_TYPE "zygVLM_config"
#define OLD_ROOT_NAME     "zygVLM_boat"

xml_boatData::xml_boatData(Projection * proj,QWidget * main, QWidget * parent)
: QWidget(parent)
{
        this->proj=proj;
        this->main=main;
        this->parent=parent;
}

bool xml_boatData::writeBoatData(QList<boatAccount*> & boat_list,QList<raceData*> & race_list,QString fname)
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

     /* managing boat data */
         QListIterator<boatAccount*> i (boat_list);
     while(i.hasNext())
     {
          boatAccount * acc = i.next();

                  group = doc.createElement(BOAT_GROUP_NAME);
                  root.appendChild(group);

                  tag = doc.createElement(LOGIN_NAME);
                  group.appendChild(tag);
                  t = doc.createTextNode(acc->getLogin());
                  tag.appendChild(t);

                  tag = doc.createElement(PASS_NAME);
                  group.appendChild(tag);
                  t = doc.createTextNode(acc->getPass().toAscii().toBase64());
                  tag.appendChild(t);

          tag = doc.createElement(ACTIVATED_NAME);
                  group.appendChild(tag);
                  bool status = acc->getStatus();
                  t = doc.createTextNode(status?"1":"0");
                  tag.appendChild(t);

          tag = doc.createElement(LOCK_NAME);
          group.appendChild(tag);
          status = acc->getLockStatus();
          t = doc.createTextNode(status?"1":"0");
          tag.appendChild(t);

          tag = doc.createElement(POLAR_NAME);
          group.appendChild(tag);
          QString polarName = acc->getPolarName();
          if(polarName.isEmpty()) polarName="none";
          t = doc.createTextNode(polarName);
          tag.appendChild(t);

          tag = doc.createElement(ALIAS_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(acc->getAlias());
          tag.appendChild(t);

          tag = doc.createElement(ALIAS_CHK_NAME);
          group.appendChild(tag);
          status = acc->getAliasState();
          t = doc.createTextNode(status?"1":"0");
          tag.appendChild(t);

          tag = doc.createElement(POLAR_CHK_NAME);
          group.appendChild(tag);
          status = acc->getPolarState();
          t = doc.createTextNode(status?"1":"0");
          tag.appendChild(t);

          tag = doc.createElement(ZOOM_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(acc->getZoom()));
          tag.appendChild(t);

          tag = doc.createElement(ESTIME_NAME);
          group.appendChild(tag);
          status = acc->getForceEstime();
          t = doc.createTextNode(status?"1":"0");
          tag.appendChild(t);
     }

     /* managing race info */
     QListIterator<raceData*> j (race_list);
     while(j.hasNext())
     {
          raceData * race_data = j.next();

          group = doc.createElement(RACE_GROUP_NAME);
          root.appendChild(group);

          tag = doc.createElement(RACEID_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(race_data->idrace);
          tag.appendChild(t);

          tag = doc.createElement(OPPLIST_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(race_data->oppList);
          tag.appendChild(t);
          tag = doc.createElement(VACLEN_NAME);
          group.appendChild(tag);
          t = doc.createTextNode(QString().setNum(race_data->vac_len));
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

bool xml_boatData::readBoatData(QList<boatAccount*> & boat_list,QList<raceData*> & race_list,QString fname)
{
     QString  errorStr;
     int errorLine;
     int errorColumn;
         bool hasVersion = false;
     bool forceWrite = false;
     int version=VERSION_NUMBER;

     QFile file(fname);

     if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
         return false;

     while (!boat_list.isEmpty())
        delete boat_list.takeFirst();
     while (!race_list.isEmpty())
        delete race_list.takeFirst();

     //QTextStream in(&file);

     QDomDocument doc;
     if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
     {
         QMessageBox::warning(0,QObject::tr("Lecture de parametre bateau"),
                              QString("Erreur ligne %1, colonne %2:\n%3")
                              .arg(errorLine)
                              .arg(errorColumn)
                              .arg(errorStr));
         return false;
     }

     QDomElement root = doc.documentElement();
     if(root.tagName() != ROOT_NAME && root.tagName() != OLD_ROOT_NAME)
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
                 hasVersion = true;
             }
         }
         else if(node.toElement().tagName() == BOAT_GROUP_NAME)
         {
             subNode = node.firstChild();
             QString login = "";
             QString pass = "";
             QString activated = "";
             QString polar="";
             bool locked=false;
             bool chk_polar=false;
             bool chk_alias=false;
             bool force_estime=false;
             QString alias="";
             float zoom=-1;

             while(!subNode.isNull())
             {
                 if(subNode.toElement().tagName() == LOGIN_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         login = dataNode.toText().data();
                 }
                 if(subNode.toElement().tagName() == PASS_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                     {
                         if(version==0)
                         {
                             pass = dataNode.toText().data();
                             forceWrite=true;
                         }
                         else
                             pass = QByteArray::fromBase64(dataNode.toText().data().toAscii());
                     }
                 }
                 if(subNode.toElement().tagName() == ACTIVATED_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         activated = dataNode.toText().data();
                 }
                 if(subNode.toElement().tagName() == POLAR_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                     {
                         polar = dataNode.toText().data();
                         if(polar=="none") polar="";
                     }
                 }
                 if(subNode.toElement().tagName() == LOCK_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         locked = dataNode.toText().data() == "1";
                 }
                 if(subNode.toElement().tagName() == ALIAS_CHK_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         chk_alias = dataNode.toText().data() == "1";
                 }
                 if(subNode.toElement().tagName() == POLAR_CHK_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         chk_polar = dataNode.toText().data() == "1";
                 }
                 if(subNode.toElement().tagName() == ALIAS_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         alias = dataNode.toText().data();
                 }
                 if(subNode.toElement().tagName() == ZOOM_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         zoom = dataNode.toText().data().toFloat();
                 }
                 if(subNode.toElement().tagName() == ESTIME_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         force_estime = dataNode.toText().data() == "1";
                 }

                 subNode = subNode.nextSibling();
             }
             if(!login.isEmpty() && !pass.isEmpty() && ! activated.isEmpty())
             {
                 qWarning() << "Boat info present => create item " <<  login << " state " << activated;
                 boatAccount * acc = new boatAccount(login,pass,activated == "1",
                                                     proj,main,parent);
                 acc->setPolar(chk_polar,polar);
                 acc->setAlias(chk_alias,alias);
                 acc->setLockStatus(locked);
                 acc->setZoom(zoom);
                 acc->setForceEstime(force_estime);
                 boat_list.append(acc);
             }
             else
                 qWarning("Incomplete boat info");
         }
         else if(node.toElement().tagName() == RACE_GROUP_NAME)
         {
             subNode = node.firstChild();
             QString race = "";
             QString opp_list = "";
             int vacLen=0;

             while(!subNode.isNull())
             {
                 if(subNode.toElement().tagName() == RACEID_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         race = dataNode.toText().data();
                 }
                 if(subNode.toElement().tagName() == OPPLIST_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         opp_list = dataNode.toText().data();
                 }
                 if(subNode.toElement().tagName() == VACLEN_NAME)
                 {
                     dataNode = subNode.firstChild();
                     if(dataNode.nodeType() == QDomNode::TextNode)
                         vacLen = dataNode.toText().data().toInt();
                 }
                 subNode = subNode.nextSibling();
             }
             if(!race.isEmpty() /*&& !opp_list.isEmpty()*/)
             {
                 /* control nb boats*/
                 QStringList lst=opp_list.split(";");
                 if(lst.size()>RACE_MAX_BOAT)
                 {
                     QMessageBox::warning(this,tr("Paramétrage des courses"),
                                          tr("Nombre maximum de concurrent dépassé")+" ("+QString().setNum(RACE_MAX_BOAT)+")");
                     while(lst.size()>RACE_MAX_BOAT)
                         lst.removeLast();
                     opp_list=lst.join(";");
                 }

                 struct raceData * race_data = new raceData();
                 qWarning() << "Race info present => id " <<  race << " opp list " << opp_list;
                 race_data->idrace=race;
                 race_data->oppList=opp_list;
                 race_data->vac_len=vacLen;
                 race_list.append(race_data);
             }
             else
                 qWarning("Incomplete race info");
         }
         node = node.nextSibling();
     }

     if(hasVersion)
     {
         if(forceWrite)
             writeBoatData(boat_list,race_list,fname);
         return true;
     }
     else
     {
         qWarning("no version");
         while (!boat_list.isEmpty())
             delete boat_list.takeFirst();
         return false;
     }
}

