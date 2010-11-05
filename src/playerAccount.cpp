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

#include <QtGui>
#include <QDebug>


#include "playerAccount.h"

#include "boatVLM.h"
#include "dataDef.h"
#include "mycentralwidget.h"
#include "MainWindow.h"



playerAccount::playerAccount(Projection * proj, MainWindow * main,
                             myCentralWidget * parent, inetConnexion * inet) : QDialog(parent)
{
    setupUi(this);

    this->proj = proj;
    this->main=main;
    this->parent=parent;
    this->inet=inet;

    msgBox = NULL;

    /* signal / slot init */

    connect(this,SIGNAL(writeBoat()),parent,SLOT(slot_writeBoatData()));
    connect(this,SIGNAL(reloadPlayer()),parent,SLOT(slot_readBoatData()));
    connect(this,SIGNAL(addBoat(boatVLM*)),parent,SLOT(slot_addBoat_list(boatVLM*)));
    connect(this,SIGNAL(delBoat(boatVLM*)),parent,SLOT(slot_delBoat_list(boatVLM*)));
    connect(this,SIGNAL(addPlayer(Player*)),parent,SLOT(slot_addPlayer_list(Player*)));
    connect(this,SIGNAL(delPlayer(Player*)),parent,SLOT(slot_delPlayer_list(Player*)));

    connect(this,SIGNAL(playerSelected(Player*)),parent,SLOT(slot_playerSelected(Player*)));
}

void playerAccount::initList(QList<Player*> * player_list)
{
    this->player_list=player_list;

    player_idx=0;
    Player * curPlayer=parent->getPlayer();
    curPlayerIdp=curPlayer?curPlayer->getId():-1;
    QListWidgetItem * curItem=NULL;

    list_player->clear();
    players.clear();

    /* init Player list */

    QListIterator<Player*> i (*player_list);
    while(i.hasNext())
    {
        Player * player = i.next();
        players.insert(player_idx,player);
        QListWidgetItem * item = new QListWidgetItem(list_player);
        setPlayerItemName(item,player);
        item->setData(ROLE_IDX,player_idx);        
        player_idx++;
        if(curPlayerIdp != -1 && curPlayerIdp==player->getId())
            curItem=item;
        /* updating player */
        updPlayer(player);
    }

    if(curItem)
    {
        list_player->setCurrentItem(curItem);
    }
    else if(list_player->count()>0)
    {
        list_player->setCurrentRow(0);
        slot_selectItem_player(list_player->currentItem());
    }

    updBtnAndString();
}

void playerAccount::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /* sync players */
        player_list->clear();

        for(int i=0;i<list_player->count();i++)
        {
            Player * player=players.value(list_player->item(i)->data(ROLE_IDX).toInt(),NULL);
            if(player)
            {
                player_list->append(player);
            }
            else
                qWarning() << "done: player not found";
        }

        int idx=list_player->currentItem()->data(ROLE_IDX).toInt();
        emit playerSelected(players.value(idx,NULL));
        emit writeBoat();
    }
    else
    {
        /* cleaning all structure */
        player_list->clear();
        QMapIterator<int,Player*> i (players);
        while(i.hasNext())
        {
            i.next();
            Player * player=i.value();
            removeBoats(player);
            emit delPlayer(player);
            delete player;
        }
        /* reloading from disk */
        emit reloadPlayer();
        /* trying to find current player */
        if(curPlayerIdp!=-1)
        {
            QListIterator<Player*> j (*player_list);
            bool found=false;
            while(j.hasNext())
            {
                Player * player=j.next();
                if(player->getId()==curPlayerIdp)
                {
                    emit playerSelected(player);
                    found=true;
                }
            }
            if(!found)
            {
                QMessageBox::critical(parent,tr("Rechargement du compte courant"),
                                      tr("Erreur lors du rechargement du compte, le compte courant a disparu, relancez qtVlm"));
                QApplication::quit();

            }
        }
    }

    while(list_player->count())
    {
        QListWidgetItem * item = list_player->currentItem();
        list_player->removeItemWidget(item);
        delete item;
    }

    list_player->clear();

    QDialog::done(result);
}

void playerAccount::updBtnAndString(void)
{
    bool test=list_player->count()>0;

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(test);
    btn_playerUpd->setEnabled(test);
    btn_playerChg->setEnabled(test);
    btn_playerDel->setEnabled(test);
    if(!test)
    {
        pl_data_name->setText(QString());
        pl_data_id->setText(QString());
        pl_data_nbboat->setText(QString());
    }
}

void playerAccount::slot_addPlayer(void)
{
    player_data * data = new player_data();
    data->login=QString();
    data->pass=QString();
    data->type=BOAT_VLM;

    if(accDialog.initDialog(data))
    {
        if(!data->login.isEmpty() && ((!data->pass.isEmpty() && data->type==BOAT_VLM) || data->type==BOAT_REAL))
        {
            Player * player = new Player(data->login,data->pass,data->type,-1,QString(),proj,main,parent,inet);

            QListWidgetItem * item = new QListWidgetItem(data->login,list_player);

            item->setData(ROLE_IDX,player_idx);
            players.insert(player_idx,player);
            player_idx++;
            emit addPlayer(player);

            qWarning() << "Create player: " << player->getLogin() << "," << player->getPass() << "," << player->getType();

            if(data->type == BOAT_VLM)
                updPlayer(player);
            else
            {
                player->setName(data->login);            
                setPlayerItemName(item,player);
            }

            list_player->setCurrentItem(item);
            slot_selectItem_player(item);            
        }
    }
    delete data;
    updBtnAndString();
}

void playerAccount::slot_modPlayer(void)
{
    Player * player=players.value(list_player->currentItem()->data(ROLE_IDX).toInt(),NULL);
    if(!player)
    {
        qWarning() << "Can't find player for modification";
        return;
    }
    player_data * data = new player_data();
    data->login=player->getLogin();
    data->pass=player->getPass();
#ifdef __QTVLM_WITH_TEST
    data->type=player->getType();
#else
    data->type=BOAT_VLM;
#endif

    if(accDialog.initDialog(data))
    {
        if((data->login!=player->getLogin() && !data->login.isEmpty())
            || (data->pass!=player->getPass() && !data->login.isEmpty())
            )
            {
            player->setParam(data->login,data->pass);
            setPlayerItemName(list_player->currentItem(),player);
            player->setType(data->type);
            if(data->type==BOAT_VLM)
                updPlayer(player);
            else
                player->setName(data->login);
        }
    }
    else
        qWarning() << "Modification annulée";
    delete data;

}

void playerAccount::slot_delPlayer(void)
{
    int idx=list_player->currentItem()->data(ROLE_IDX).toInt();
    Player * player=players.value(idx,NULL);
    if(!player)
    {
        qWarning() << "Can't find player for DEL";
        return;
    }
    if(QMessageBox::question(main,tr("Suppression de compte"),
                             tr("Voulez-vous supprimer le compte ")+player->getLogin(),QMessageBox::Yes|QMessageBox::No,
                             QMessageBox::Yes)==QMessageBox::Yes)
    {
        removeBoats(player);        
        QListWidgetItem * item =list_player->currentItem();
        list_player->removeItemWidget(item);
        delete(item);
        players.remove(idx);
        emit delPlayer(player);
        delete(player);
        qWarning() << "Player deleted";
    }
    updBtnAndString();
}

void playerAccount::removeBoats(Player * player)
{
    if(!player->getBoats()) return;

    QListIterator<boatVLM*> i (*player->getBoats());
    while(i.hasNext())
    {
        boatVLM * boat = i.next();
        qWarning() << "Trying to remove boat: " << boat->getName();
        player->getBoats()->removeAll(boat);
        emit delBoat(boat);        
        delete(boat);
    }
}

void playerAccount::slot_updPlayer(void)
{
    int idx=list_player->currentItem()->data(ROLE_IDX).toInt();
    Player * player=players.value(idx,NULL);
    if(!player)
    {
        qWarning() << "Can't find player for UPD";
        return;
    }

    updPlayer(player);
}

void playerAccount::updPlayer(Player * player)
{
    /* showing msg box */
    if(msgBox)
        delete msgBox;
    msgBox= new QMessageBox(QMessageBox::Information,tr("Mise à jour de compte"),
                            tr("Mise en cours du compte ")+player->getLogin(),
                            QMessageBox::NoButton,this,Qt::SplashScreen);
    msgBox->show();
    connect(player,SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updFinished(bool,Player*)));
    player->updateData();
}

void playerAccount::slot_updFinished(bool res_ok, Player * player)
{
#warning voir la gestion des erreurs de update
    disconnect(player,SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updFinished(bool,Player*)));

    if(msgBox)
    {
        delete msgBox;
        msgBox=NULL;
    }

    if(!res_ok)
    {
        qWarning() << "Erreur de MaJ player";
        return;
    }

    doUpdate(player);

    /* updating player displayed */
    int idx=players.key(player,-1);
    if(idx!=-1)
    {
        //qWarning() << "Player found in player list";
        for(int i=0;i<list_player->count();i++)
        {
            QListWidgetItem * item = list_player->item(i);
            if(item->data(ROLE_IDX).toInt()==idx)
            {
                setPlayerItemName(item,player);
                slot_selectItem_player(item);
            }
        }
    }

    /*else
        qWarning() << "Player " << player->getLogin() << " - " << player->getName() << " not found";*/
    //qWarning() << "Updt finished: nb " << player->getBoats()->count();
}

void playerAccount::doUpdate(Player * player)
{
    qWarning() << "Maj player: " << player->getLogin();

    /* comparing old and new boats */
    QListIterator<boatData*> i (player->getBoatsData());

    while(i.hasNext())
    {
        bool found=false;
        boatData * data = i.next();
        /* searching in current list*/

        QListIterator<boatVLM*> j (*player->getBoats());
        while(j.hasNext())
        {
            boatVLM * boat = j.next();
            if(boat->getId() == data->idu && boat->getPlayerId() == player->getId())
            {
                found=true;
                //qWarning() << "updating boat";
                boat->updateData(data);
                break;
            }
        }

        if(!found)
        {
            /* not found => create boat */
            //qWarning() << "creating boat: " << data->name << " " << data->engaged;
            boatVLM * boat=new boatVLM(data->name,data->engaged!=0,data->idu,player->getId(),player,data->isOwn,
                                       proj,main,parent,inet);
            emit addBoat(boat);
            player->getBoats()->append(boat);
            //qWarning() << "after create: nb " << player->getBoats()->count();
        }
    }

    /* checking for deletion */
    QListIterator<boatVLM*> j (*player->getBoats());
    while(j.hasNext())
    {
        bool found=false;
        boatVLM * boat = j.next();
        QListIterator<boatData*> i (player->getBoatsData());
        while(i.hasNext())
        {
            boatData * data = i.next();
            if(boat->getId() == data->idu && boat->getPlayerId() == player->getId())
            {
                found=true;
                break;
            }
        }
        if(!found)
        {
            //qWarning() << "Need to remove a boat";
            /* need to delete */
            emit delBoat(boat);
            player->getBoats()->removeAll(boat);
            delete(boat);
            //qWarning() << "after remove: nb " << player->getBoats()->count();
        }
    }
}

void  playerAccount::slot_selectItem_player( QListWidgetItem * item)
{
    //qWarning() << "Selecting item: " << item << " (idx=" << item->data(ROLE_IDX).toInt() + ")";
    Player * player=players.value(item->data(ROLE_IDX).toInt(),NULL);
    if(!player)
    {
        qWarning() << "selectItem: player not found: " << item->data(ROLE_IDX).toInt();
        return;
    }
    //qWarning() << "Selecting " << player->getName() << " type=" << player->getType();
    /* display player data */
    if(player->getType()==BOAT_VLM)
    {
        if(player->getName().isEmpty())
            pl_data_name->setText(tr("NO NAME ?"));
        else
            pl_data_name->setText(player->getName());

        pl_data_id->setText("id:"+QString().setNum(player->getId()));

        int nb=player->getBoats()->count();
        QString str;
        if(nb==0)
            str=tr("Pas de bateau");
        else
        {
            if(nb==1)
                str=tr("1 bateau");
            else
                str=QString().setNum(nb)+" "+tr("bateaux");
        }
        pl_data_nbboat->setText(str);
    }
    else
    {
        pl_data_name->setText(QString());
        pl_data_id->setText(QString());
        pl_data_nbboat->setText(QString());
    }


    /* change btn state */
    btn_playerUpd->setEnabled(true);
    btn_playerDel->setEnabled(true);
    btn_playerChg->setEnabled(true);
}

void playerAccount::slot_selectAndValidateItem(QListWidgetItem* item)
{
    Player * player=players.value(item->data(ROLE_IDX).toInt(),NULL);
    if(!player)
    {
        qWarning() << "selectItem: player not found: " << item->data(ROLE_IDX).toInt();
        return;
    }
    QDialog::accept();
}

void playerAccount::setPlayerItemName(QListWidgetItem * item,Player * player)
{
    if(player->getType()==BOAT_VLM)
        item->setText(player->getName() + " - " + player->getLogin());
    else
        item->setText(player->getName());
}

/************************************************************************/

paramAccount::paramAccount(QWidget * parent): QDialog(parent)
{
    setupUi(this);
    edit_login->setText(QString());
    edit_pass->setText(QString());

#ifdef __QTVLM_WITH_TEST
    type_layout->setEnabled(true);
    type_cb->show();
    type_label->show();
    type_cb->setCurrentIndex(0);
#else
    type_cb->hide();
    type_label->hide();
    type_layout->setEnabled(false);
#endif
}

bool paramAccount::initDialog(player_data * data)
{
    if(!data)
        return false;
    edit_login->setText(data->login);
    edit_pass->setText(data->pass);
#ifdef __QTVLM_WITH_TEST
    type_cb->setCurrentIndex(data->type);
    edit_pass->setEnabled(data->type==BOAT_VLM);
#endif

    if(exec()==QDialog::Accepted)
    {
        data->login=edit_login->text();
        data->pass=edit_pass->text();
#ifdef __QTVLM_WITH_TEST
        data->type=type_cb->currentIndex();
#else
        data->type=BOAT_VLM;
#endif
        return true;
    }
    return false;
}

void paramAccount::slot_typeChanged(int)
{
    edit_pass->setEnabled(type_cb->currentIndex()==BOAT_VLM);
}
