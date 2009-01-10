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

#include "xmlBoatData.h"

#define VERSION_NUMBER    1
#define DOM_FILE_TYPE     "qtVLM_config"
#define ROOT_NAME         "qtVLM_boat"
#define VERSION_NAME      "Version"
#define BOAT_GROUP_NAME   "Boat"
#define LOGIN_NAME        "Login"
#define PASS_NAME         "Pass"
#define ACTIVATED_NAME    "Activated"
#define POLAR_NAME        "Polar"
#define LOCK_NAME         "Lock"

#define OLD_DOM_FILE_TYPE "zygVLM_config"
#define OLD_ROOT_NAME     "zygVLM_boat"

xml_boatData::xml_boatData(Projection * proj,QWidget * main, QWidget * parent)
: QWidget(parent)
{
	this->proj=proj;
	this->main=main;
	this->parent=parent;
	connect(this,SIGNAL(showMessage(QString)),main,SLOT(slotShowMessage(QString)));
}

bool xml_boatData::writeBoatData(QList<boatAccount*> & boat_list,QString fname)
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
     }
     
     QFile file(fname);
     if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
         return false;
         
     QTextStream out(&file);
     
     doc.save(out,4);
     
     file.close();
     
     return true;
}

bool xml_boatData::readBoatData(QList<boatAccount*> & boat_list,QString fname)
{
     QString  errorStr;
     int errorLine;
     int errorColumn;
	 bool hasVersion = false;
     bool forceWrite = false;
     int version=VERSION_NUMBER;

     QFile file(fname);
     
     showMessage("Read xml");

     if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
         return false;
         
	 boat_list.clear();

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

	 showMessage("Process root");

	 QDomElement root = doc.documentElement();
	 if(root.tagName() != ROOT_NAME && root.tagName() != OLD_ROOT_NAME)
	 {
         showMessage("Wrong root name: " + root.tagName());
	     return false;
	 }
	 
	 QDomNode node = root.firstChild();
	 QDomNode subNode;
	 QDomNode dataNode;
	 
	 while(!node.isNull())
	 {
		  showMessage("Processing: " + node.toElement().tagName());
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
			  showMessage("Processing subnodes");
			  while(!subNode.isNull())
			  {
				   showMessage("Sub node: "+subNode.toElement().tagName());
				   
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
                              showMessage("Processing old version of password");
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
                          showMessage("Polar:"+polar);
                          if(polar=="none") polar="";
                      }
                   }
                   if(subNode.toElement().tagName() == LOCK_NAME)
                   {
                      dataNode = subNode.firstChild();
                      if(dataNode.nodeType() == QDomNode::TextNode)
                          locked = dataNode.toText().data() == "1";
                   }
				   subNode = subNode.nextSibling();
			  }
			  if(!login.isEmpty() && !pass.isEmpty() && ! activated.isEmpty())
			  {
				   showMessage(QString("Boat info present => create item %1 %2 %3").arg(login)
                              .arg(pass).arg(activated));
                   boatAccount * acc = new boatAccount(login,pass,activated == "1",
                    		   proj,main,parent);
                   acc->setPolar(polar);
                   acc->setLockStatus(locked);

                   boat_list.append(acc);
			  }
			  else
                   showMessage("Incomplete boat info");
		  }
		  node = node.nextSibling();
	 }

	 if(hasVersion)
	 {
		 showMessage("all ok");
         if(forceWrite)
             writeBoatData(boat_list,fname);
	 	 return true;
	 }
	 else
	 {
		 showMessage("no version");
         boat_list.clear();
         return false;
	 }
}

