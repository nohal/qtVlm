/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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
#include <QDebug>
#include <parser.h>

#include "boatVLM.h"
#include "boatReal.h"
#include "Player.h"
#include "dataDef.h"

Player::Player(QString login, QString pass,int type, int id, QString name,
               Projection * proj,MainWindow * main,
               myCentralWidget * parent,inetConnexion * inet): inetClient(inet)
{
    this->login=login;
    this->pass=pass;
    this->type=0;
    this->name=name;
    this->player_id=id;
    this->type=type;

    realBoat=NULL;

    inetClient::setName(login);

    needAuth=true;

    this->parent=parent;
    this->main=main;
    this->proj=proj;

    boats.clear();
    boatsData.clear();

    updating = false;

    if(type==BOAT_REAL)
    {
        realBoat = new boatReal(this->login,true,proj,main,parent);
    }
}

Player::~Player()
{
    /* cleaning boat list */
    QListIterator<boatVLM*> j (boats);
    while(j.hasNext())
    {
        boatVLM * boat = j.next();
        emit delBoat_list(boat);
        delete boat;
    }
    boats.clear();
}

void Player::updateData(void)
{
    updating = true;
    /* cleaning boatsData */
    for(int i=0;i<boatsData.count();i++)
        delete boatsData.at(i);
    boatsData.clear();

    /* getting profile */
    doRequest(VLM_REQUEST_PROFILE);
}

void Player::doRequest(int requestCmd)
{
    if(hasInet())
    {
        if(hasRequest() )
        {
            qWarning() << "Request already running for player" << login;
            return;
        }
        else
            qWarning() << "Doing request " << requestCmd << " for player " << login  ;

        QString page;

        switch(requestCmd)
        {
            case VLM_REQUEST_PROFILE:
                page = "/ws/playerinfo/profile.php";
                break;
            case VLM_REQUEST_FLEET:
                page = "/ws/playerinfo/fleet_private.php";
                break;
            default:
                return;
        }

        inetGet(requestCmd,page);
    }
}

void Player::requestFinished (QByteArray res_byte)
{
    QJson::Parser parser;
    bool ok;

    //qWarning() << "Res=" << res_byte;

    QVariantMap result = parser.parse (res_byte, &ok).toMap();
    if (!ok) {
        qWarning() << "Error parsing json data " << res_byte;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        updating=false;
        emit playerUpdated(false,this);
        return;
    }

    if(checkWSResult(res_byte,"Player: " + login,parent))
    {

        switch(getCurrentRequest())
        {
            case VLM_REQUEST_PROFILE:
            {
                QVariantMap profile= result["profile"].toMap();
                player_id=profile["idp"].toInt();
                name=profile["playername"].toString();

                doRequest(VLM_REQUEST_FLEET);
                break;
            }

            case VLM_REQUEST_FLEET:
            {
                QVariantMap fleet;

                for(int j=0;j<2;j++)
                {
                    fleet = result[j==0?"fleet":"fleet_boatsit"].toMap();
                    foreach(QVariant pt,fleet)
                    {
                        //qWarning() << pt.toMap();
                        QVariantMap boats_data=pt.toMap();
                        /*boatVLM * boat = new boatVLM(boats_data["boatname"].toString(),
                                                     false, boats_data["idu"].toInt(), player_id, this,j,
                                                     proj, main, parent, getInet());
                        boat->setPseudo(boats_data["boatpseudo"].toString());
                        boat->setEngaged(boats_data["engaged"].toInt());
                        boats.append(boat);*/
                        //emit addBoat_list(boat);
                        boatData * data = new boatData();
                        data->name=boats_data["boatname"].toString();
                        data->pseudo=boats_data["boatpseudo"].toString();
                        data->idu=boats_data["idu"].toInt();
                        data->engaged=boats_data["engaged"].toInt();
                        data->isOwn=j;
                        boatsData.append(data);
                    }
                }
                updating=false;
                //qWarning() << "nb boats: " <<boatsData.count();
                emit playerUpdated(true,this);
                break;
            }

        }
    }
    else
    {
        updating=false;
        emit playerUpdated(false,this);
    }
}

void Player::authFailed(void)
{
    QMessageBox::warning(0,QObject::tr("Parametre compte VLM"),
                         QString("Erreur de parametrage du compte VLM '")+
                         login+"'.\n Verifier le login et mot de passe");
    inetClient::authFailed();
    updating=false;
    emit playerUpdated(false,this);
}

void Player::inetError()
{
    qWarning() << "Inet error";
    updating=false;
    emit playerUpdated(false,this);
}