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

#include "opponentBoat.h"
#include "mycentralwidget.h"
#include "MainWindow.h"
#include "Projection.h"
#include "inetConnexion.h"
#include "boatVLM.h"
#include "boatReal.h"
#include "dataDef.h"
#include "BarrierSet.h"

#define VERSION_NUMBER    2
#define ROOT_NAME         "qtVLM_boat"
#define VERSION_NAME      "Version"

/* player data */
#define PLAYER_GROUP_NAME   "Player"
#define PLAYER_LOGIN_NAME   "Login"
#define PLAYER_PASS_NAME    "Pass"
#define PLAYER_NAME_NAME    "Name"
#define PLAYER_POLAR_NAME   "PolarName"
#define PLAYER_ID_NAME      "Idp"
#define PLAYER_TYPE_NAME    "Type"


/* BOAT data */
#define BOAT_GROUP_NAME     "Boat"
#define BOAT_NAME_NAME      "Name" //for compatibility
#define BOAT_PSEUDO_NAME    "Pseudo"
#define BOAT_IDU_NAME       "Idu"
#define BOAT_IDP_NAME       "Idp"
#define BOAT_ISOWN_NAME     "IsOwn"
#define BOAT_ACTIVATED_NAME "Activated"
#define BOAT_POLAR_NAME     "Polar"
#define BOAT_LOCK_NAME      "Lock"
#define BOAT_POLAR_CHK_NAME "UsePolar"
#define BOAT_ALIAS_CHK_NAME "UseAlias"
#define BOAT_ALIAS_NAME     "Alias"
#define BOAT_ZOOM_NAME      "Zoom"
#define BOAT_ESTIME_NAME    "Estime"
#define BOAT_LAT_NAME       "Latitude"
#define BOAT_LON_NAME       "Longitude"
#define BOAT_NPD            "NotePad"
#define BOAT_GROUP_SET_NAME "BarrierSetKey"
#define BOAT_KEY_NAME       "BarrierSetKey"
#define BOAT_USE_SKIN       "UseSkin"
#define BOAT_BOARD_SKIN     "BoardSkin"

/* RACE DATA */
#define RACE_GROUP_NAME   "Race"
#define RACEID_NAME       "raceId"
#define OPPLIST_NAME      "oppList"
#define DISPLAY_NSZ       "displayNSZ"
#define LAT_NSZ           "latNSZ"
#define WIDTH_NSZ         "widthNSZ"
#define COLOR_NSZ_R       "colorNSZ_R"
#define COLOR_NSZ_G       "colorNSZ_G"
#define COLOR_NSZ_B       "colorNSZ_B"
#define SHOWWHAT          "showWhat"
#define SHOWREAL          "showReal"
#define REALFILTER        "realFilter"

xml_boatData::xml_boatData(Projection * proj,MainWindow * main, myCentralWidget * parent,inetConnexion * inet)
: QWidget(parent)
{
    this->proj=proj;
    this->main=main;
    this->parent=parent;
    this->inet=inet;

    /* signals */
    connect(parent,SIGNAL(writeBoatData(QList<Player*>&,QList<raceData*>&,QString)),
            this,SLOT(slot_writeData(QList<Player*>&,QList<raceData*>&,QString)));
    connect(parent,SIGNAL(readBoatData(QString,bool)),this,SLOT(slot_readData(QString,bool)));

    connect(this,SIGNAL(addBoat(boat*)),parent,SLOT(slot_addBoat(boat*)));
    connect(this,SIGNAL(addRace_list(raceData*)),parent,SLOT(slot_addRace_list(raceData*)));
    connect(this,SIGNAL(addPlayer_list(Player*)),parent,SLOT(slot_addPlayer_list(Player*)));

    connect(this,SIGNAL(delBoat(boat*)),parent,SLOT(slot_delBoat(boat*)));
    connect(this,SIGNAL(delRace_list(raceData*)),parent,SLOT(slot_delRace_list(raceData*)));
    connect(this,SIGNAL(delPlayer_list(Player*)),parent,SLOT(slot_delPlayer_list(Player*)));
}

void xml_boatData::slot_writeData(QList<Player*> & player_list,QList<raceData*> & race_list,QString fname)
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

    /* managing player data */

    QListIterator<Player*> j (player_list);

    //qWarning() << "Write boatAcc - " << player_list.count() << " players to sav";

    while(j.hasNext())
    {
        Player * player= j.next();

        /*qWarning() << "Write boatAcc - " << player->getLogin() << ": "
                <<  player->getBoats()->count() << " boats to sav";*/

        group = doc.createElement(PLAYER_GROUP_NAME);
        root.appendChild(group);

        tag = doc.createElement(PLAYER_LOGIN_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(player->getLogin());
        tag.appendChild(t);

        tag = doc.createElement(PLAYER_PASS_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(player->getPass().toLatin1().toBase64());
        tag.appendChild(t);

        tag = doc.createElement(PLAYER_NAME_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(player->getName());
        tag.appendChild(t);        

        /*tag = doc.createElement(PLAYER_POLAR_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(player->getPolarName());
        tag.appendChild(t);*/

        tag = doc.createElement(PLAYER_ID_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(QString().setNum(player->getId()));
        tag.appendChild(t);

        tag = doc.createElement(PLAYER_TYPE_NAME);
        group.appendChild(tag);
        t = doc.createTextNode(QString().setNum(player->getType()));
        tag.appendChild(t);

        /* managing boat data => saving only boats listed in player */

        if(player->getType()==BOAT_VLM)
        {
            if(!player->getBoats()) continue;

            QListIterator<boatVLM*> i (*player->getBoats());

            while(i.hasNext())
            {
                boatVLM * boat = i.next();

                group = doc.createElement(BOAT_GROUP_NAME);
                root.appendChild(group);

                tag = doc.createElement(BOAT_PSEUDO_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(boat->getBoatPseudo());
                tag.appendChild(t);

                tag = doc.createElement(BOAT_IDU_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(boat->getBoatId());
                tag.appendChild(t);

                tag = doc.createElement(BOAT_IDP_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getPlayerId()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ISOWN_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getIsOwn()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ACTIVATED_NAME);
                group.appendChild(tag);
                bool status = boat->getStatus();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_LOCK_NAME);
                group.appendChild(tag);
                status = boat->getLockStatus();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_POLAR_NAME);
                group.appendChild(tag);
                QString polarName = boat->getPolarName();
                if(polarName.isEmpty()) polarName="none";
                t = doc.createTextNode(polarName);
                tag.appendChild(t);

                tag = doc.createElement(BOAT_POLAR_CHK_NAME);
                group.appendChild(tag);
                status = boat->getPolarState();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ALIAS_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(boat->getAlias());
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ALIAS_CHK_NAME);
                group.appendChild(tag);
                status = boat->getAliasState();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_BOARD_SKIN);
                group.appendChild(tag);
                t = doc.createTextNode(boat->get_boardSkin());
                tag.appendChild(t);

                tag = doc.createElement(BOAT_USE_SKIN);
                group.appendChild(tag);
                status = boat->get_useSkin();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ZOOM_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getZoom()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ESTIME_NAME);
                group.appendChild(tag);
                status = boat->getForceEstime();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_NPD);
                group.appendChild(tag);
                QString temp=boat->getNpd();
                temp=temp.replace(QString("\n"),QString("^!"));
                t = doc.createTextNode(temp);
                tag.appendChild(t);

                add_setKeys(&doc,&group,boat);

            }
        }
        else
        {
            boatReal * boat = player->getRealBoat();
            if(boat)
            {
                group = doc.createElement(BOAT_GROUP_NAME);
                root.appendChild(group);

                tag = doc.createElement(BOAT_PSEUDO_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(boat->getBoatPseudo());
                tag.appendChild(t);

                tag = doc.createElement(BOAT_IDP_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(player->getId()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ACTIVATED_NAME);
                group.appendChild(tag);
                bool status = boat->getStatus();
                t = doc.createTextNode(status?"1":"0");
                tag.appendChild(t);

                tag = doc.createElement(BOAT_POLAR_NAME);
                group.appendChild(tag);
                QString polarName = boat->getPolarName();
                if(polarName.isEmpty()) polarName="none";
                t = doc.createTextNode(polarName);
                tag.appendChild(t);

                tag = doc.createElement(BOAT_LAT_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getLat()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_LON_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getLon()));
                tag.appendChild(t);

                tag = doc.createElement(BOAT_ZOOM_NAME);
                group.appendChild(tag);
                t = doc.createTextNode(QString().setNum(boat->getZoom()));
                tag.appendChild(t);

                add_setKeys(&doc,&group,boat);

            }
        }
    }
    QDomNode node = root.firstChild();
    node=root.nextSiblingElement(RACE_GROUP_NAME);
    while(!node.isNull())
    {
        root.removeChild(node);
        node=root.nextSiblingElement(RACE_GROUP_NAME);
    }

     QListIterator<raceData*> k (race_list);
     while(k.hasNext())
     {
         raceData * race_data = k.next();

         //qWarning() << "Saving race: " << race_data->idrace << " - " << race_data->oppList;

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

         tag = doc.createElement(DISPLAY_NSZ);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->displayNSZ?1:0));
         tag.appendChild(t);

         tag = doc.createElement(LAT_NSZ);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->latNSZ));
         tag.appendChild(t);

         tag = doc.createElement(WIDTH_NSZ);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->widthNSZ));
         tag.appendChild(t);

         tag = doc.createElement(COLOR_NSZ_R);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->colorNSZ.red()));
         tag.appendChild(t);
         tag = doc.createElement(COLOR_NSZ_G);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->colorNSZ.green()));
         tag.appendChild(t);
         tag = doc.createElement(COLOR_NSZ_B);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->colorNSZ.blue()));
         tag.appendChild(t);

         tag = doc.createElement(SHOWWHAT);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->showWhat));
         tag.appendChild(t);

         tag = doc.createElement(SHOWREAL);
         group.appendChild(tag);
         t = doc.createTextNode(QString().setNum(race_data->showReal?1:0));
         tag.appendChild(t);

         tag = doc.createElement(REALFILTER);
         group.appendChild(tag);
         t = doc.createTextNode(race_data->realFilter);
         tag.appendChild(t);
     }

     QFile file(fname);
     if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
         return ;

     QTextStream out(&file);

     doc.save(out,4);

     file.close();

}

void xml_boatData::add_setKeys(QDomDocument * doc,QDomElement * group, boat *myBoat) {
    QDomElement subGroup;
    QDomElement tag;
    QDomText t;

    QList<QString>* set_keys = myBoat->get_barrierKeys();
    if(set_keys->count() > 0) {

        subGroup = doc->createElement(BOAT_GROUP_SET_NAME);
        group->appendChild(subGroup);

        QListIterator<QString> i (*set_keys);

        while(i.hasNext()) {
            tag = doc->createElement(BOAT_KEY_NAME);
            subGroup.appendChild(tag);
            t = doc->createTextNode(i.next());
            tag.appendChild(t);
        }
    }
}

void xml_boatData::slot_readData(QString fname,bool readAll)
{
    /* opening file */
    QString  errorStr;
    int errorLine;
    int errorColumn;    
    bool hasVersion = false;

    QFile file(fname);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
        return ;

    QDomDocument doc;
    if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
    {
        QMessageBox::warning(0,QObject::tr("Lecture de parametre bateau"),
                             QString("Erreur ligne %1, colonne %2:\n%3")
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

    /* check for old version */
    QDomNode node = root.firstChild();
    while(!node.isNull())
    {
        if(node.toElement().tagName() == VERSION_NAME)
        {
            QDomNode dataNode = node.firstChild();
            if(dataNode.nodeType() == QDomNode::TextNode)
            {
                hasVersion = true;
                if(dataNode.toText().data().toInt()<VERSION_NUMBER)
                {
                    QMessageBox::warning(this->main,tr("Chargement des comptes/bateaux"),
                                         tr("Ancienne version de fichier, demarrage avec une configuration vide"));
                    file.close();                    
                    return;
                }
            }
            break;
        }
    }

    if(!hasVersion)
    {
        qWarning() << "Missing version number in boatAcc.dat";
        return;
    }

    /* are we reading all ? */
    PlayerMap *pList=new PlayerMap();
    if(readAll)
    {
        /* start with players */
        readPlayer(root.firstChild(),pList);
        /* now read boats */
        readBoat(root.firstChild(),pList);
    }

    /* read race data */
    readRace(root.firstChild());
    delete pList;
}

void xml_boatData::readPlayer(QDomNode node,PlayerMap * pList)
{
    while(!node.isNull())
    {
        if(node.toElement().tagName() == PLAYER_GROUP_NAME)
        {
            QDomNode subNode= node.firstChild();
            QDomNode dataNode;
            QString login = "";
            QString pass = "";
            QString name = "";
            QString polarName = "";
            int player_id = 0;
            int player_type = BOAT_VLM;

            while(!subNode.isNull())
            {
                if(subNode.toElement().tagName() == PLAYER_LOGIN_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        login = dataNode.toText().data();
                }
                if(subNode.toElement().tagName() == PLAYER_ID_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        player_id = dataNode.toText().data().toInt();
                }
                if(subNode.toElement().tagName() == PLAYER_TYPE_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        player_type = dataNode.toText().data().toInt();
                }
                if(subNode.toElement().tagName() == PLAYER_PASS_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        pass = QByteArray::fromBase64(dataNode.toText().data().toLatin1());
                }                
                if(subNode.toElement().tagName() == PLAYER_NAME_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        name = dataNode.toText().data();
                }
                /*if(subNode.toElement().tagName() == PLAYER_POLAR_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        polarName = dataNode.toText().data();
                }*/
                subNode = subNode.nextSibling();
            }
            /* do we have enough info */
            if(!login.isEmpty() && ((!pass.isEmpty() && player_type==BOAT_VLM) || player_type==BOAT_REAL))
            {
                Player * player = new Player(login,pass,player_type,player_id,name,proj,main,parent,inet);
                //player->setPolarName(polarName);
                pList->insertMulti(player_id,player);
                emit addPlayer_list(player);
                qWarning() << "ReadPlayer: add " << login;
            }
            else
                qWarning() << "Not adding 1 player:" << login;
        }


        node = node.nextSibling();
    }
}

void xml_boatData::readBoat(QDomNode node,PlayerMap * pList)
{
    while(!node.isNull())
    {
        if(node.toElement().tagName() == BOAT_GROUP_NAME)
        {
            QDomNode subNode= node.firstChild();
            QDomNode dataNode;

            QString pseudo = "";
            int idu=-1;
            int idp=-1;
            bool activated = false;
            bool chk_polar=false;
            QString polar="";
            bool locked=false;            
            bool chk_alias=false;
            QString alias="";
            bool force_estime=false;
            double zoom=-1;
            int isOwn=0;
            double lat=0,lon=0;
            QString npd="";
            QList<QString> setKeys;
            bool useSkin;
            QString boardSkin;

            while(!subNode.isNull())
            {
                if(subNode.toElement().tagName() == BOAT_NAME_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        pseudo = dataNode.toText().data();
                }
                if(subNode.toElement().tagName() == BOAT_PSEUDO_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        pseudo = dataNode.toText().data();
                }
                if(subNode.toElement().tagName() == BOAT_IDU_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        idu = dataNode.toText().data().toInt();
                }
                if(subNode.toElement().tagName() == BOAT_IDP_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        idp = dataNode.toText().data().toInt();
                }
                if(subNode.toElement().tagName() == BOAT_ISOWN_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        isOwn = dataNode.toText().data().toInt();
                }
                if(subNode.toElement().tagName() == BOAT_ACTIVATED_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        activated = dataNode.toText().data()=="1";
                }
                if(subNode.toElement().tagName() == BOAT_POLAR_CHK_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        chk_polar = dataNode.toText().data() == "1";
                }
                if(subNode.toElement().tagName() == BOAT_POLAR_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                    {
                        polar = dataNode.toText().data();
                        if(polar=="none") polar="";
                    }
                }                
                if(subNode.toElement().tagName() == BOAT_ALIAS_CHK_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        chk_alias = dataNode.toText().data() == "1";
                }

                if(subNode.toElement().tagName() == BOAT_ALIAS_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        alias = dataNode.toText().data();
                }
                if(subNode.toElement().tagName() == BOAT_USE_SKIN)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        useSkin = dataNode.toText().data() == "1";
                }

                if(subNode.toElement().tagName() == BOAT_BOARD_SKIN)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        boardSkin = dataNode.toText().data();
                }
                if(subNode.toElement().tagName() == BOAT_LOCK_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        locked = dataNode.toText().data() == "1";
                }
                if(subNode.toElement().tagName() == BOAT_ZOOM_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        zoom = dataNode.toText().data().toDouble();
                }
                if(subNode.toElement().tagName() == BOAT_ESTIME_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        force_estime = dataNode.toText().data() == "1";
                }
                if(subNode.toElement().tagName() == BOAT_LAT_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        lat = dataNode.toText().data().toDouble();
                }
                if(subNode.toElement().tagName() == BOAT_LON_NAME)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        lon = dataNode.toText().data().toDouble();
                }
                if(subNode.toElement().tagName() == BOAT_NPD)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                    {
                        QString temp=dataNode.toText().data();
                        npd = temp.replace(QString("^!"),QString("\n"));
                    }
                }               

                if(subNode.toElement().tagName() == BOAT_GROUP_SET_NAME)
                {
                    QDomNode subSubNode= subNode.firstChild();

                    while(!subSubNode.isNull())
                    {
                        if(subSubNode.toElement().tagName() == BOAT_KEY_NAME) {
                            dataNode = subSubNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                                setKeys.append(dataNode.toText().data());
                        }
                        subSubNode = subSubNode.nextSibling();
                    }
                }


                subNode = subNode.nextSibling();
            }

            /* trying to find player idp */
            Player * player=NULL;
            if(idp!=-1 && idp!=0)
                player=pList->value(idp,NULL);

            if(player)
            {
                if(player->getType()==BOAT_VLM)
                {
                    //qWarning() << "Boat has player => create item " <<  pseudo << " state " << activated;
                    boatVLM * boat = new boatVLM(pseudo,activated, idu,idp,player,isOwn,
                                                 proj,main,parent,inet);
                    boat->setPolar(chk_polar,polar);
                    boat->setAlias(chk_alias,alias);
                    boat->set_boardSkin(boardSkin);
                    boat->set_useSkin(useSkin);
                    boat->setLockStatus(locked);
                    boat->setZoom(zoom);
                    boat->setForceEstime(force_estime);
                    boat->setNpd(npd);
                    boat->setSetKeys(setKeys);
                    emit addBoat(boat);
                    player->addBoat(boat);
                }
                else
                {
                    //qWarning() << "Creating real boat from xmlBoatReader: " << pseudo << " - " << player->getName();
                    //player->setRealBoat(new boatReal(pseudo,true,proj,main,parent));
                    // should init players variable here
                    boatReal * boat=player->getRealBoat();
                    //qWarning()<<"zoom restored"<<zoom;
                    if(boat)
                    {
                        boat->setPosition(lat,lon);
                        boat->setPolar(polar);
                        boat->setZoom(zoom);
                        boat->setSetKeys(setKeys);
                        //proj->setScaleAndCenterInMap(boat->getZoom(),boat->getLon(),boat->getLat());
                    }


                }
            }
            else
                qWarning() << "Boat has NO player " <<  pseudo << "(player= " << idp << ")";
        }
        node = node.nextSibling();
    }
}

void xml_boatData::readRace(QDomNode node)
{
    while(!node.isNull())
    {
        if(node.toElement().tagName() == RACE_GROUP_NAME)
        {
            QDomNode subNode= node.firstChild();
            QDomNode dataNode;

            QString race = "";
            QString opp_list = "";
            bool displayNSZ=false;
            double latNSZ=-60;
            double widthNSZ=2;
            int showWhat=0;
            bool showReal=false;
            QString filter="";
            QColor colorNSZ=Qt::black;

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
                if(subNode.toElement().tagName() == DISPLAY_NSZ)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        displayNSZ=(dataNode.toText().data().toInt()==1);
                }
                if(subNode.toElement().tagName() == LAT_NSZ)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        latNSZ=(dataNode.toText().data().toDouble());
                }
                if(subNode.toElement().tagName() == WIDTH_NSZ)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        widthNSZ=(dataNode.toText().data().toDouble());
                }
                if(subNode.toElement().tagName() == COLOR_NSZ_R)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        colorNSZ.setRed(dataNode.toText().data().toInt());
                }
                if(subNode.toElement().tagName() == COLOR_NSZ_G)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        colorNSZ.setGreen(dataNode.toText().data().toInt());
                }
                if(subNode.toElement().tagName() == COLOR_NSZ_B)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        colorNSZ.setBlue(dataNode.toText().data().toInt());
                }
                if(subNode.toElement().tagName() == SHOWWHAT)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        showWhat=(dataNode.toText().data().toInt());
                }

                if(subNode.toElement().tagName() == SHOWREAL)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        showReal=(dataNode.toText().data().toInt()==1);
                }

                if(subNode.toElement().tagName() == REALFILTER)
                {
                    dataNode = subNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        filter=(dataNode.toText().data());
                }

                subNode = subNode.nextSibling();
            }
            if(!race.isEmpty() /*&& !opp_list.isEmpty()*/)
            {
                /* control nb boats*/
                QStringList lst=opp_list.split(";");
                if(lst.size()>RACE_MAX_BOAT)
                {
                    QMessageBox::warning(this,tr("Parametrage des courses"),
                                         tr("Nombre maximum de concurrent depasse")+" ("+QString().setNum(RACE_MAX_BOAT)+")");
                    while(lst.size()>RACE_MAX_BOAT)
                        lst.removeLast();
                    opp_list=lst.join(";");
                }

                struct raceData * race_data = new raceData();
                //qWarning() << "Race info present => id " <<  race << " opp list " << opp_list;
                race_data->idrace=race;
                race_data->oppList=opp_list;
                race_data->colorNSZ=colorNSZ;
                race_data->displayNSZ=displayNSZ;
                race_data->latNSZ=latNSZ;
                race_data->widthNSZ=widthNSZ;
                race_data->showWhat=showWhat;
                race_data->showReal=showReal;
                race_data->realFilter=filter;
                emit addRace_list(race_data);
            }
            else
                qWarning("Incomplete race info");
        }
        node = node.nextSibling();
    }
}
