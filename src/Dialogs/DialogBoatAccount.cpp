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
#ifdef QT_V5
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <QDebug>
#include <QBuffer>
#include <QDialog>
#include <QFileDialog>

#include "DialogBoatAccount.h"

#include "xmlBoatData.h"
#include "boatVLM.h"
#include "dataDef.h"
#include "mycentralwidget.h"
#include "MainWindow.h"
#include "Util.h"
#include "settings.h"
DialogBoatAccount::DialogBoatAccount(Projection * proj, MainWindow * main, myCentralWidget * parent,inetConnexion * inet) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);

    this->proj = proj;
    this->main=main;
    this->parent=parent;
    this->inet=inet;

    /* signal / slot init */

    connect(this,SIGNAL(accountListUpdated()), main, SLOT(slotAccountListUpdated()));

    connect(this,SIGNAL(writeBoat()),parent,SLOT(slot_writeBoatData()));
    connect(this,SIGNAL(writeBoat()),main,SIGNAL(paramVLMChanged()));
    connect(this->btn_browseSkin,SIGNAL(clicked()),this,SLOT(slot_browseSkin()));
}

DialogBoatAccount::~DialogBoatAccount()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}
void DialogBoatAccount::slot_browseSkin()
{
    QString skinPath=QFileInfo(boardSkin->text()).absolutePath();
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Selectionner un skin tableau de bord VLM"), skinPath, "png (*.png)");
     if(fileName!="")
         boardSkin->setText(QFileInfo(fileName).absoluteFilePath());
}

bool DialogBoatAccount::initList( QList<boatVLM*> * boat_list, Player * player)
{
    if(!boat_list || !player)
        return false;

    this->boat_list=boat_list;

    this->setWindowTitle(tr("Liste des bateaux pour ")+player->getName());

    boat_idx=0;

    list_boat->clear();
    list_boatSit->clear();
    polarList->clear();

    boats.clear();


    QListIterator<boatVLM*> j (*boat_list);
    while(j.hasNext())
    {
        boatVLM * boat = j.next();
        boatSetup * setup=new boatSetup(boat);
        QListWidgetItem * item;

        if(boat->getIsOwn()==BOAT_OWN)
            item = new QListWidgetItem(list_boat);
        else
            item = new QListWidgetItem(list_boatSit);
        if(boat->getStatus())
        {
            item->setData(Qt::ForegroundRole,QColor(Qt::darkRed));
        }
        setBoatItemName(item,boat);
        item->setData(ROLE_IDX,boat_idx);
        item->setData(ROLE_IDU,boat->getId());
        boats.insert(boat_idx,setup);
        boat_idx++;
    }




    /* browse polar folder */
    QDir polarDir = QDir(appFolder.value("polar"));
    QStringList extStr;
    extStr.append("*.pol");
    extStr.append("*.POL");
    extStr.append("*.csv");
    extStr.append("*.CSV");
    QFileInfoList fileList=polarDir.entryInfoList(extStr,QDir::Files);

    /* default value */
    polarList->addItem(tr("<Aucun>"));

    if(!fileList.isEmpty())
    {
        QListIterator<QFileInfo> j (fileList);

        while(j.hasNext())
        {
            QFileInfo finfo = j.next();
            polarList->addItem(finfo.fileName());
        }
    }
    if(list_boat->count()>0)
    {
        list_boat->setCurrentRow(0);
        setItem(list_boat->currentItem());
        //slot_selectItem_boat(list_boat->currentItem(),NULL);
    }
    else if(list_boatSit->count()>0)
    {
        list_boatSit->setCurrentRow(0);
        setItem(list_boatSit->currentItem());
        //slot_selectItem_boat(list_boatSit->currentItem(),NULL);
    }
    return true;
}

void DialogBoatAccount::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /* validate last change for currentItem */
        saveItem(list_boat->currentItem());
        saveItem(list_boatSit->currentItem());
        for(int n=0;n<list_boat->count();n++)
        {
            QListWidgetItem * item = list_boat->item(n);
            for(int nBoat=0;nBoat<boat_list->count();nBoat++)
            {
                if(boat_list->at(nBoat)->getId()==item->data(ROLE_IDU).toInt())
                {
                    boatVLM * boat=boat_list->at(nBoat);
                    boat_list->removeAt(nBoat);
                    boat_list->append(boat);
                    break;
                }
            }
        }
        for(int n=0;n<list_boatSit->count();n++)
        {
            QListWidgetItem * item = list_boatSit->item(n);
            for(int nBoat=0;nBoat<boat_list->count();nBoat++)
            {
                if(boat_list->at(nBoat)->getId()==item->data(ROLE_IDU).toInt())
                {
                    boatVLM * boat=boat_list->at(nBoat);
                    boat_list->removeAt(nBoat);
                    boat_list->append(boat);
                    break;
                }
            }
        }

        while(list_boat->count())
        {
            QListWidgetItem * item = list_boat->item(0);
            list_boat->removeItemWidget(item);
            delete item;
        }
        list_boat->clear();

        while(list_boatSit->count())
        {
            QListWidgetItem * item = list_boatSit->item(0);
            list_boatSit->removeItemWidget(item);
            delete item;
        }
        list_boatSit->clear();

        /* update all boat */
        QMapIterator<int,boatSetup*> i (boats);
        while(i.hasNext())
        {
            i.next();
            /* sync changes */
            i.value()->updateBoat();
            /* clean list */
            delete i.value();
        }


        polarList->clear();
        emit accountListUpdated();
        emit writeBoat();
        //parent->emitUpdateRoute();
    }
    else
    {
        while(list_boat->count())
        {
            QListWidgetItem * item = list_boat->item(0);
            list_boat->removeItemWidget(item);
            delete item;
        }
        list_boat->clear();

        while(list_boatSit->count())
        {
            QListWidgetItem * item = list_boatSit->item(0);
            list_boatSit->removeItemWidget(item);
            delete item;
        }
        list_boatSit->clear();
    }



    QDialog::done(result);
}


void DialogBoatAccount::setBoatItemName(QListWidgetItem * item,boatVLM * boat)
{
    if(boat->getIsOwn()==BOAT_OWN)
        if(boat->getRaceName().isEmpty())
            item->setText(boat->getBoatPseudo()+" ("+tr("pas en course en ce moment")+")");
        else
            item->setText(boat->getBoatPseudo()+" ("+boat->getRaceName()+")");
    else
    {
        if(boat->getOwn().isEmpty())
        {
            if(boat->getRaceName().isEmpty())
                item->setText(boat->getBoatPseudo() +" ("+tr("pas en course en ce moment")+")");
            else
                item->setText(boat->getBoatPseudo() +" ("+boat->getRaceName()+")");
        }
        else /*for VLM0.15*/
        {
            if(boat->getRaceName().isEmpty())
                item->setText(boat->getBoatPseudo() + " " + tr("proprio")+": "+boat->getOwn()+" ("+tr("pas en course en ce moment")+")");
            else
                item->setText(boat->getBoatPseudo() + " " + tr("proprio")+": "+boat->getOwn()+" ("+boat->getRaceName()+")");
        }
    }
}

void  DialogBoatAccount::slot_selectItem_boat(QListWidgetItem * item, QListWidgetItem * old)
{
    if(!item) return ;

    /*qWarning() << "boat Old: " << old << " - new: " << item
            << " - current boat: " << list_boat->currentItem()
            << " - current boatSit: " << list_boatSit->currentItem();*/
    if(!old)
    {
        saveItem(list_boatSit->currentItem());
        list_boatSit->setCurrentItem(NULL);
        list_boatSit->clearSelection();
        list_boatSit->clearFocus();
        btn_upBoatSit->setEnabled(false);
        btn_downBoatSit->setEnabled(false);
    }
    else
    {
        saveItem(old);
    }


    setItem(item);

    int index=list_boat->row(item);
    btn_upBoat->setEnabled(index>0);
    btn_downBoat->setEnabled(index<(list_boat->count()-1));
}

void  DialogBoatAccount::slot_selectItem_boatSit(QListWidgetItem * item, QListWidgetItem * old)
{
    if(!item) return ;

    /*qWarning() << "boatSit Old: " << old << " - new: " << item
            << " - current boat: " << list_boat->currentItem()
            << " - current boatSit: " << list_boatSit->currentItem();*/

    if(!old)
    {
        saveItem(list_boat->currentItem());
        list_boat->setCurrentItem(NULL);
        list_boat->clearSelection();
        list_boat->clearFocus();
        btn_upBoat->setEnabled(false);
        btn_downBoat->setEnabled(false);
    }
    else
    {
        saveItem(old);
    }


    setItem(item);

    int index=list_boatSit->row(item);
    btn_upBoatSit->setEnabled(index>0);
    btn_downBoatSit->setEnabled(index<(list_boatSit->count()-1));

}

void DialogBoatAccount::saveItem(QListWidgetItem * item)
{
    if(!item)
        return;

    boatSetup * boat=boats.value(item->data(ROLE_IDX).toInt(),NULL);
    if(!boat)
    {
        qWarning() << "selectItem: OLD boat not found";
        return;
    }
    if(!chk_polar->isChecked() || polarList->currentIndex()==0)
    {
        boat->usePolar=false;
        boat->polar=QString();
    }
    else
    {
        qWarning() << "Setting polar to " << polarList->currentText();
        boat->usePolar=true;
        boat->polar=polarList->currentText();
    }
    boat->useAlias=chk_alias->isChecked();
    boat->alias=edit_alias->text();
    boat->activated=enable_state->checkState()==Qt::Checked;
    boat->blocked=lockChange->checkState()==Qt::Checked;
    boat->useSkin=chk_skin->isChecked();
    boat->boardSkin=boardSkin->text();
}

void DialogBoatAccount::setItem(QListWidgetItem * item)
{
    if(!item)
        return;
    boatSetup * boat=boats.value(item->data(ROLE_IDX).toInt(),NULL);
    if(!boat)
    {
        qWarning() << "selectItem: NEW boat not found";
        return;
    }

    QString polarStr=boat->polar;
    chk_polar->setChecked(boat->usePolar && !polarStr.isEmpty());
    polarList->setCurrentIndex(polarStr.isEmpty()?0:polarList->findText(polarStr));

    chk_alias->setChecked(boat->useAlias);
    edit_alias->setText(boat->alias);

    disconnect (enable_state,SIGNAL(toggled(bool)),this,SLOT(slot_enableChanged(bool)));
    enable_state->setCheckState(boat->activated?Qt::Checked:Qt::Unchecked);
    currentItem=item;
    connect (enable_state,SIGNAL(toggled(bool)),this,SLOT(slot_enableChanged(bool)));
    lockChange->setCheckState(boat->blocked?Qt::Checked:Qt::Unchecked);
    chk_skin->setChecked(boat->useSkin);
    boardSkin->setText(boat->boardSkin);

    if(boat->boat)
    {
        boatVLM * curBoat=boat->boat;
        pseudo->setText(curBoat->getName());
        boat_id->setText(QString().setNum(curBoat->getId()));        
    }
    else
    {
        pseudo->setText(QString());
        boat_id->setText(QString());
    }
}
void DialogBoatAccount::slot_enableChanged(bool b)
{
    if(b)
        this->currentItem->setData(Qt::ForegroundRole,QColor(Qt::darkRed));
    else
        this->currentItem->setData(Qt::ForegroundRole,QColor(Qt::black));
}

void  DialogBoatAccount::chkAlias_changed(int state)
{
    edit_alias->setEnabled(state==Qt::Checked);
}

void  DialogBoatAccount::chkPolar_changed(int state)
{
    if(state!=Qt::Checked)
        polarList->setCurrentIndex(0);
}

void DialogBoatAccount::slot_boatUp(void)
{
    boatUp(list_boat);
}

void DialogBoatAccount::slot_boatDown(void)
{
    boatDown(list_boat);
}

void DialogBoatAccount::slot_boatSitUp(void)
{
    boatUp(list_boatSit);
}

void DialogBoatAccount::slot_boatSitDown(void)
{
    boatDown(list_boatSit);
}

void DialogBoatAccount::boatUp(QListWidget * list)
{
    int index = list->currentRow();
    if(index==0)
        return;
    QListWidgetItem * item = list->takeItem(index);
    index--;
    list->insertItem(index,item);
    list->setCurrentRow(index);
}

void DialogBoatAccount::boatDown(QListWidget * list)
{
    int index = list->currentRow();
    if(index==list->count()-1)
        return;
    QListWidgetItem * item = list->takeItem(index);
    index++;
    list->insertItem(index,item);
    list->setCurrentRow(index);
}

/******************************************************************/

boatSetup::boatSetup(void)
{
    useAlias=false;
    alias=QString();
    usePolar=false;
    polar=QString();
    activated=false;
    blocked=false;
    useSkin=false;
}

boatSetup::boatSetup(boatVLM * boat)
{
    this->boat=boat;
    useAlias=boat->getAliasState();
    alias=boat->getAlias();
    usePolar=boat->getPolarState();
    polar=boat->getPolarName();
    activated=boat->getStatus();
    blocked=boat->getLockStatus();
    useSkin=boat->get_useSkin();
    boardSkin=boat->get_boardSkin();
    if(boardSkin.isEmpty() || !(QFile(boardSkin).exists()))
    {
        boardSkin=Settings::getSetting("defaultSkin",QFileInfo("img/skin_compas.png").absoluteFilePath()).toString();
        if(!QFile(boardSkin).exists())
            boardSkin=QFileInfo("img/skin_compas.png").absoluteFilePath();
    }
}

void boatSetup::updateBoat(void)
{
    boat->setAlias(useAlias,alias);
    boat->setPolar(usePolar,polar);
    boat->setLockStatus(blocked);
    boat->setStatus(activated);
    boat->set_useSkin(useSkin);
    boat->set_boardSkin(boardSkin);
}
