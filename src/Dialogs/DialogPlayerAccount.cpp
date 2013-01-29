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
#ifdef QT_V5
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif
#include <QDebug>


#include "DialogPlayerAccount.h"

#include "boatVLM.h"
#include "dataDef.h"
#include "mycentralwidget.h"
#include "MainWindow.h"
#include "route.h"
#include "routage.h"
#include "boatReal.h"
#include "Util.h"
#include "settings.h"
#include <QTranslator>
#include <QCoreApplication>


DialogPlayerAccount::DialogPlayerAccount(Projection * proj, MainWindow * main,
                             myCentralWidget * parent, inetConnexion * inet) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    this->proj = proj;
    this->main=main;
    this->parent=parent;
    this->inet=inet;

    msgBox = NULL;

    /* signal / slot init */

    connect(this,SIGNAL(writeBoat()),parent,SLOT(slot_writeBoatData()));
    connect(this,SIGNAL(reloadPlayer()),parent,SLOT(slot_readBoatData()));
    connect(this,SIGNAL(addBoat(boat*)),parent,SLOT(slot_addBoat(boat*)));
    connect(this,SIGNAL(delBoat(boat*)),parent,SLOT(slot_delBoat(boat*)));
    connect(this,SIGNAL(addPlayer(Player*)),parent,SLOT(slot_addPlayer_list(Player*)));
    connect(this,SIGNAL(delPlayer(Player*)),parent,SLOT(slot_delPlayer_list(Player*)));

    connect(this,SIGNAL(playerSelected(Player*)),parent,SLOT(slot_playerSelected(Player*)));
    this->en->setChecked(Settings::getSetting("appLanguage","fr").toString()=="en");
    qWarning()<<"langage is"<<Settings::getSetting("appLanguage","fr").toString();
    if(!parent->getIsStartingUp())
        lang->hide();
    else
    {
        connect(fr,SIGNAL(toggled(bool)),this,SLOT(slot_langChanged(bool)));
        connect(en,SIGNAL(toggled(bool)),this,SLOT(slot_langChanged(bool)));
        connect(cz,SIGNAL(toggled(bool)),this,SLOT(slot_langChanged(bool)));
        connect(es,SIGNAL(toggled(bool)),this,SLOT(slot_langChanged(bool)));
    }
}

void DialogPlayerAccount::initList(QList<Player*> * player_list)
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
        if(player->getType()==BOAT_VLM)
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
    if(list_player->count()>0)
        this->lang->hide();
    updBtnAndString();
}
void DialogPlayerAccount::slot_langChanged(bool)
{
    QString la;
    if(fr->isChecked())
    {
        Settings::setSetting("appLanguage", "fr");
        la="fr";
    }
    else if (en->isChecked())
    {
        Settings::setSetting("appLanguage", "en");
        la="en";
    }
    else if (cz->isChecked())
    {
        Settings::setSetting("appLanguage", "cz");
        la="cz";
    }
    else if (es->isChecked())
    {
        Settings::setSetting("appLanguage", "es");
        la="cz";
    }

    if(la=="fr")
    {
        this->btn_playerAdd->setText("Nouveau");
        this->btn_playerChg->setText("Modifier");
        this->btn_playerUpd->setText(tr("Mise a jour"));
        this->btn_playerDel->setText("Supprimer");
        this->pl_data_name->setText("Nom");
        this->pl_data_nbboat->setText("Nb bateaux");
        this->setWindowTitle("Gestion des comptes");
        accDialog.labelBoatName->setText("Identifiant");
        accDialog.labelPass->setText("Mot de passe");
        accDialog.groupBox->setTitle("Type de bateau");
        accDialog.vlmBoat->setText("Bateau VLM");
        accDialog.realBoat->setText(tr("Bateau reel"));
        accDialog.setWindowTitle(tr("Details du compte"));
    }
    else if (la=="en")
    {
        this->btn_playerAdd->setText("New");
        this->btn_playerChg->setText("Modify");
        this->btn_playerUpd->setText("Update");
        this->btn_playerDel->setText("Delete");
        this->pl_data_name->setText("Name");
        this->pl_data_nbboat->setText("Nb boats");
        this->setWindowTitle("Accounts management");
        accDialog.labelBoatName->setText("Login");
        accDialog.labelPass->setText("Password");
        accDialog.groupBox->setTitle("Type of boat");
        accDialog.vlmBoat->setText("VLM boat");
        accDialog.realBoat->setText("Real boat");
        accDialog.setWindowTitle("Account details");
    }
    else if (la=="es")
    {
        this->btn_playerAdd->setText("Nuevo");
        this->btn_playerChg->setText("Modificar");
        this->btn_playerUpd->setText("Actualizar");
        this->btn_playerDel->setText("Borrar");
        this->pl_data_name->setText("Nombre");
        this->pl_data_nbboat->setText("Num. barcos");
        this->setWindowTitle("Gestión de cuentas");
        accDialog.labelBoatName->setText("Login");
        accDialog.labelPass->setText("Password");
        accDialog.groupBox->setTitle("Tipo de barco");
        accDialog.vlmBoat->setText("Barco VLM");
        accDialog.realBoat->setText("Barco real");
        accDialog.setWindowTitle("Detalles de la cuenta");
    }
    else if (la=="cz")
    {
        this->btn_playerAdd->setText(tr("Novy"));
        this->btn_playerChg->setText("Upravit");
        this->btn_playerUpd->setText("Aktualizovat");
        this->btn_playerDel->setText("Smazat");
        this->pl_data_name->setText(tr("Jmeno"));
        this->pl_data_nbboat->setText(tr("Pocet lodi"));
        this->setWindowTitle(tr("Sprava uctu"));
        accDialog.labelBoatName->setText(tr("Jmeno"));
        accDialog.labelPass->setText("Heslo");
        accDialog.groupBox->setTitle("Typ lodi");
        accDialog.vlmBoat->setText("VLM lod");
        accDialog.realBoat->setText(tr("Skutecna lod"));
        accDialog.setWindowTitle(tr("Detaily uctu"));
    }
    main->setRestartNeeded();
}

void DialogPlayerAccount::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(result == QDialog::Accepted)
    {
        if(list_player->count()==0)
        {
            QMessageBox::critical(parent,tr("Gestion des comptes"),
                                  tr("Aucun compte cree"));
            return;
        }

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

    }
    else
    {
//        /* cleaning all structure */
//        player_list->clear();
//        QMapIterator<int,Player*> i (players);
//        while(i.hasNext())
//        {
//            i.next();
//            Player * player=i.value();
//            if(!removeBoats(player)) continue;
//            emit delPlayer(player);
//            delete player;
//        }
//        /* reloading from disk */
//        emit reloadPlayer();
//        /* trying to find current player */
//        if(curPlayerIdp!=-1)
//        {
//            QListIterator<Player*> j (*player_list);
//            bool found=false;
//            while(j.hasNext())
//            {
//                Player * player=j.next();
//                if(player->getId()==curPlayerIdp)
//                {
//                    emit playerSelected(player);
//                    found=true;
//                }
//            }
//            if(!found)
//            {
//                QMessageBox::critical(parent,tr("Rechargement du compte courant"),
//                                      tr("Erreur lors du rechargement du compte, le compte courant a disparu, relancez qtVlm"));
//                QApplication::quit();

//            }
//        }
    }

    while(list_player->count())
    {
        QListWidgetItem * item = list_player->currentItem();
        list_player->removeItemWidget(item);
        delete item;
    }

    list_player->clear();
    //parent->emitUpdateRoute();
    QDialog::done(result);
}

void DialogPlayerAccount::updBtnAndString(void)
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

void DialogPlayerAccount::slot_addPlayer(void)
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
                player->setId(-2);
            }

            list_player->setCurrentItem(item);
            slot_selectItem_player(item);            
        }
    }
    delete data;
    updBtnAndString();
}

void DialogPlayerAccount::slot_modPlayer(void)
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
#if 1
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
    {
        qWarning() << "Modification annulée";
    }
    delete data;
}

void DialogPlayerAccount::slot_delPlayer(void)
{
    int idx=list_player->currentItem()->data(ROLE_IDX).toInt();
    Player * player=players.value(idx,NULL);
    if(!player)
    {
        qWarning() << "Can't find player for DEL";
        return;
    }
    QList<ROUTE*> routes=parent->getRouteList();
    QList<ROUTAGE*> routages=parent->getRoutageList();
    QList<int> ids;
    if(player->getType()==BOAT_VLM)
    {
        QList<boatVLM*>  *boats=player->getBoats();
        for(int nn=0;nn<boats->count();nn++)
            ids.append(boats->at(nn)->getId());
    }
    else
        ids.append(player->getRealBoat()->getId());
    for(int nn=0;nn<ids.count();nn++)
    {
        for(int mm=0;mm<routes.count();mm++)
        {
            if(routes.at(mm)->getBoat()->getId()==ids.at(nn))
            {
                QMessageBox::critical(main,tr("Suppression de compte"),
                                      tr("Ce compte possede un bateau utilise dans une route.<br>Impossible de supprimer ce compte."));
                return;
            }
        }
        for(int mm=0;mm<routages.count();mm++)
        {
            if(routages.at(mm)->getBoat()->getId()==ids.at(nn))
            {
                QMessageBox::critical(main,tr("Suppression de compte"),
                                      tr("Ce compte possede un bateau utilise dans un routage.<br>Impossible de supprimer ce compte."));
                return;
            }
        }
    }

    if(QMessageBox::question(main,tr("Suppression de compte"),
                             tr("La suppression est definitive<br>Voulez-vous VRAIMENT supprimer le compte ")+player->getLogin(),QMessageBox::Yes|QMessageBox::No,
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

void DialogPlayerAccount::removeBoats(Player * player)
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

void DialogPlayerAccount::slot_updPlayer(void)
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

void DialogPlayerAccount::updPlayer(Player * player)
{
    /* showing msg box */
    if(msgBox)
        delete msgBox;
    msgBox= new QMessageBox(QMessageBox::Information,tr("Mise a jour de compte"),
                            tr("Mise a jour du compte en cours pour ")+player->getLogin(),
                            QMessageBox::NoButton,this,Qt::SplashScreen);
    msgBox->show();
    connect(player,SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updFinished(bool,Player*)));
    player->updateData();
}

void DialogPlayerAccount::slot_updFinished(bool res_ok, Player * player)
{
    disconnect(player,SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updFinished(bool,Player*)));

    //qWarning() << "Update finished for " << player->getName();

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
        //qWarning() << "Player found in player list idx=" << idx;
        //qWarning() << "CurrentRow=" << list_player->currentRow();
        for(int i=0;i<list_player->count();i++)
        {
            QListWidgetItem * item = list_player->item(i);
            if(item->data(ROLE_IDX).toInt()==idx)
            {
                setPlayerItemName(item,player);
                if(i==list_player->currentRow())
                    slot_selectItem_player(item);
            }
        }
    }

    /*else
        qWarning() << "Player " << player->getLogin() << " - " << player->getName() << " not found";*/
    //qWarning() << "Updt finished: nb " << player->getBoats()->count();
}

void DialogPlayerAccount::doUpdate(Player * player)
{
    qWarning() << "Maj player: " << player->getLogin();

    QListIterator<boatVLM*> jj (*player->getBoats());
    while(jj.hasNext())
    {
        boatVLM * boat = jj.next();
        if(!player->getFleetList().contains(boat->getId()))
        {
/* cas du bateau supprime du boatsit*/
            emit delBoat(boat);
            player->getBoats()->removeAll(boat);
            delete(boat);
        }
    }
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
            boatVLM * boat=new boatVLM(data->pseudo,data->engaged!=0,data->idu,player->getId(),player,data->isOwn,
                                       proj,main,parent,inet);
            boat->updateData(data);
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

void  DialogPlayerAccount::slot_selectItem_player( QListWidgetItem * item)
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
        //qWarning() << "We have a VLM boat";
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
        //qWarning() << "We have a REAL boat";
        pl_data_name->setText(QString());
        pl_data_id->setText(QString());
        pl_data_nbboat->setText(QString());
    }


    /* change btn state */
    btn_playerUpd->setEnabled(true);
    btn_playerDel->setEnabled(true);
    btn_playerChg->setEnabled(true);
}

void DialogPlayerAccount::slot_selectAndValidateItem(QListWidgetItem* item)
{
    Player * player=players.value(item->data(ROLE_IDX).toInt(),NULL);
    if(!player)
    {
        qWarning() << "selectItem: player not found: " << item->data(ROLE_IDX).toInt();
        return;
    }
    QDialog::accept();
}

void DialogPlayerAccount::setPlayerItemName(QListWidgetItem * item,Player * player)
{
    if(player->getType()==BOAT_VLM)
        item->setText(player->getName() + " - " + player->getLogin());
    else
        item->setText(player->getName());
}

/************************************************************************/

DialogParamAccount::DialogParamAccount(QWidget * parent): QDialog(parent)
{
    setupUi(this);
    edit_login->setText(QString());
    edit_pass->setText(QString());
    //this->retranslateUi(this);
}

bool DialogParamAccount::initDialog(player_data * data)
{
    if(!data)
        return false;
    edit_login->setText(data->login);
    edit_pass->setText(data->pass);
    connect(vlmBoat,SIGNAL(toggled(bool)),this,SLOT(slot_typeChanged(bool)));
    connect(this->edit_login,SIGNAL(textChanged(QString)),this,SLOT(slot_loginPassChanged(QString)));
    connect(this->edit_pass,SIGNAL(textChanged(QString)),this,SLOT(slot_loginPassChanged(QString)));
#if 1
    vlmBoat->setChecked(data->type==BOAT_VLM);
    edit_pass->setEnabled(data->type==BOAT_VLM);
    edit_pass->setHidden(realBoat->isChecked());
    QString s1="Identifiant";
    QString s2="Nom du bateau";
    if(Settings::getSetting("appLanguage", "en").toString()=="en")
    {
        s1="Login";
        s2="Boat name";
    }
    else if(Settings::getSetting("appLanguage", "en").toString()=="cz")
    {
        s1="Jmeno";
        s2="Lod jmeno";
    }
    else if(Settings::getSetting("appLanguage", "en").toString()=="es")
    {
        s1="Login";
        s2="Nombre del barco";
    }
    labelBoatName->setText(vlmBoat->isChecked()?s1:s2);
    labelPass->setHidden(realBoat->isChecked());
    slot_loginPassChanged(QString());
#endif

    if(exec()==QDialog::Accepted)
    {
        data->login=edit_login->text();
        data->pass=edit_pass->text();
#if 1
        data->type=vlmBoat->isChecked()?BOAT_VLM:BOAT_REAL;
#else
        data->type=BOAT_VLM;
#endif
        return true;
    }
    return false;
}

void DialogParamAccount::slot_typeChanged(bool)
{
    edit_pass->setEnabled(vlmBoat->isChecked());
    QString s1="Identifiant";
    QString s2="Nom du bateau";
    if(Settings::getSetting("appLanguage", "en").toString()=="en")
    {
        s1="Login";
        s2="Boat name";
    }
    else if(Settings::getSetting("appLanguage", "en").toString()=="cz")
    {
        s1="Jmeno";
        s2="Lod jmeno";
    }
    else if(Settings::getSetting("appLanguage", "en").toString()=="es")
    {
        s1="Login";
        s2="Nombre del barco";
    }
    labelBoatName->setText(vlmBoat->isChecked()?s1:s2);
    edit_pass->setHidden(realBoat->isChecked());
    labelPass->setHidden(realBoat->isChecked());
    slot_loginPassChanged(QString());
}
void DialogParamAccount::slot_loginPassChanged(QString)
{
    if(vlmBoat->isChecked())
        this->buttonBox->button(QDialogButtonBox::Ok)->setDisabled((edit_login->text().isEmpty() || edit_pass->text().isEmpty()));
    else
        this->buttonBox->button(QDialogButtonBox::Ok)->setDisabled((edit_login->text().isEmpty()));
}
