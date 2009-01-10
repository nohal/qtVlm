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

#include <QtGui>
#include <QBuffer>

#include "boatAccount_dialog.h"

#include "xmlBoatData.h"

#define ROLE_PASS      Qt::UserRole
#define ROLE_ACTIVATED Qt::UserRole+1
#define ROLE_POLAR     Qt::UserRole+2
#define ROLE_LOCKED    Qt::UserRole+3

boatAccount_dialog::boatAccount_dialog(QList<boatAccount*> & acc_list, Projection * proj, QWidget * main, QWidget * parent) : QDialog(parent)
{
    setupUi(this);

    this->acc_list = &acc_list;
    this->proj = proj;
    this->main=main;
    this->parent=parent;

    /* load data */

    xmlData = new xml_boatData(proj,main,parent);
    xmlData->readBoatData(*this->acc_list,"boatAcc.dat");

    /* signal / slot init */

    connect(btn_accAdd, SIGNAL(clicked()), this, SLOT(slot_addBoat()));
    connect(btn_accDel, SIGNAL(clicked()), this, SLOT(slot_delBoat()));
    connect(btn_accChg, SIGNAL(clicked()), this, SLOT(slot_chgBoat()));

    connect(this,SIGNAL(vlmSync()),main,SLOT(slotVLM_Sync()));

    connect(list_boat,SIGNAL(itemClicked(QListWidgetItem *)),
    this, SLOT(slot_selectItem(QListWidgetItem *)));

    connect(this,SIGNAL(showMessage(QString)), main, SLOT(slotShowMessage(QString)));
    connect(this,SIGNAL(accountListUpdated()), main, SLOT(slotAccountListUpdated()));

    /* sync boat position */
    //emit vlmSync();
}

boatAccount_dialog::~boatAccount_dialog()
{

}



void boatAccount_dialog::initList(QList<boatAccount*> & acc_list)
{
    this->acc_list = & acc_list;

    list_boat->clear();
    polarList->clear();

    QListIterator<boatAccount*> i (acc_list);

    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        QListWidgetItem * item = new QListWidgetItem(acc->getLogin(),list_boat);
        item->setData(ROLE_PASS,acc->getPass());
        item->setData(ROLE_ACTIVATED,
                      (acc->getStatus()?Qt::Checked:Qt::Unchecked));
        item->setData(ROLE_POLAR,acc->getPolarName());
        item->setData(ROLE_LOCKED,
                      (acc->getLockStatus()?Qt::Checked:Qt::Unchecked));
    }

    blank = new QListWidgetItem("<Nouveau>",list_boat);
    list_boat->setCurrentItem(blank);
    slot_selectItem(blank);

    /* browse polar folder */
    QDir polarDir = QDir("polar");
    QStringList extStr;
    extStr.append("*.pol");
    extStr.append("*.POL");
    QFileInfoList fileList=QDir("polar").entryInfoList(extStr,QDir::Files);

    /* default value */
    polarList->addItem(tr("<Aucun>"));
    
    if(!fileList.isEmpty())
    {
        QListIterator<QFileInfo> j (fileList);
        
        while(j.hasNext())
        {
            QFileInfo finfo = j.next();
            polarList->addItem(finfo.baseName());
        }
    }
    
    
}

void boatAccount_dialog::slot_accHasChanged(void)
{
    QListWidgetItem * curItem = list_boat->currentItem();
    if(curItem != blank && (edit_login->text() != curItem->text()
            || edit_pass->text() != curItem->data(ROLE_PASS).toString()
            || enable_state->checkState() != ((Qt::CheckState)curItem->data(ROLE_ACTIVATED).toInt()))
            || polarList->currentText() != curItem->data(ROLE_PASS).toString()
            || lockChange->checkState() != ((Qt::CheckState)curItem->data(ROLE_LOCKED).toInt())
      )
        btn_accChg->setEnabled(true);
    else
        btn_accChg->setEnabled(false);
}

void boatAccount_dialog::done(int result)
{
    if(result == QDialog::Accepted)
    {
/* do we have emptied all account ?*/
        if(list_boat->count() == 1)
        {
            /* clear all account from list*/
            while(acc_list->count()!=0)
            {
                delete acc_list->last();
                acc_list->removeLast();
            }
            acc_list->clear();
        }
        else
        {
            int commonLen = qMin(list_boat->count()-1,acc_list->count());
            int i;

            for(i=0;i<commonLen;i++)
            {
                QListWidgetItem * item = list_boat->item(i);
                boatAccount * acc = acc_list->at(i);
                acc->setParam(item->text(),
                    item->data(ROLE_PASS).toString(),
                    ((Qt::CheckState)item->data(ROLE_ACTIVATED).toInt())==Qt::Checked);
                acc->setPolar(item->data(ROLE_POLAR).toString());
                acc->setLockStatus(((Qt::CheckState)item->data(ROLE_LOCKED).toInt())==Qt::Checked);
                if(acc->getStatus())
                    acc->getData();
            }

            if(list_boat->count()-1<acc_list->count())
            { /* need to remove account */
                while(acc_list->count()!=commonLen)
                {
                    delete acc_list->last();
                    acc_list->removeLast();
                }
            }

            if(list_boat->count()-1>acc_list->count())
            { /* need to create account */
                for(/*nothing*/;i<list_boat->count()-1;i++)
                {
                    QListWidgetItem * item = list_boat->item(i);
                    boatAccount * acc = new boatAccount(item->text(),
                        item->data(ROLE_PASS).toString(),
                        ((Qt::CheckState)item->data(ROLE_ACTIVATED).toInt())==Qt::Checked,
                         proj,main,parent);
                    acc->setPolar(item->data(ROLE_POLAR).toString());
                    acc->setLockStatus(((Qt::CheckState)item->data(ROLE_LOCKED).toInt())==Qt::Checked);
                    acc_list->append(acc);
                    if(acc->getStatus())
                        acc->getData();
                }
            }

        }
        emit accountListUpdated();
        xmlData->writeBoatData(*acc_list,QString("boatAcc.dat"));
    }
    QDialog::done(result);
}

/* add boat button */
void boatAccount_dialog::slot_addBoat(void)
{
     /* empty login or empty password*/
     if(edit_login->text().isEmpty() || edit_pass->text().isEmpty())
        return;

     /*entry already exists*/
     if(list_boat->findItems(edit_login->text(),Qt::MatchFixedString).isEmpty())
     {
        QListWidgetItem * item = new QListWidgetItem(edit_login->text());
        item->setData(ROLE_PASS,edit_pass->text());
        item->setData(ROLE_ACTIVATED,enable_state->checkState());
        item->setData(ROLE_POLAR,polarList->currentIndex()==0?QString():polarList->currentText());
        item->setData(ROLE_LOCKED,lockChange->checkState());
        list_boat->insertItem(list_boat->count()-1,item);
        list_boat->setCurrentItem(item);
        edit_login->setText("");
        edit_pass->setText("");
        polarList->setCurrentIndex(0);
        enable_state->setCheckState(Qt::Unchecked);
        lockChange->setCheckState(Qt::Unchecked);
     }
     btn_accChg->setEnabled(false);
}

/* del boat button */
void boatAccount_dialog::slot_delBoat(void)
{
     QListWidgetItem * item = list_boat->currentItem();
     if(item)
     {
        edit_login->setText("");
        edit_pass->setText("");
        polarList->setCurrentIndex(0);
        enable_state->setCheckState(Qt::Unchecked);
        lockChange->setCheckState(Qt::Unchecked);
        delete(item);
        if(list_boat->currentItem() == blank)
        {
           btn_accChg->setEnabled(false);
           btn_accDel->setEnabled(false);
        }
     }
     btn_accChg->setEnabled(false);
}

/* Modify button */
void boatAccount_dialog::slot_chgBoat(void)
{
     QListWidgetItem * item = list_boat->currentItem();
     if(item)
     {
        item->setText(edit_login->text());
        item->setData(ROLE_PASS,edit_pass->text());
        item->setData(ROLE_ACTIVATED,enable_state->checkState());
        item->setData(ROLE_POLAR,polarList->currentIndex()==0?QString():polarList->currentText());
        item->setData(ROLE_LOCKED,lockChange->checkState());
     }
     btn_accChg->setEnabled(false);
}

/* changing selection */
void  boatAccount_dialog::slot_selectItem( QListWidgetItem * item)
{
     if(item==blank)
     {
          edit_login->setText("");
          edit_pass->setText("");
          polarList->setCurrentIndex(0);
          enable_state->setCheckState(Qt::Unchecked);
          lockChange->setCheckState(Qt::Unchecked);
          btn_accChg->setEnabled(false);
          btn_accDel->setEnabled(false);
     }
     else
     {
         edit_login->setText(item->text());
         edit_pass->setText(item->data(ROLE_PASS).toString());
         polarList->setCurrentIndex(item->data(ROLE_POLAR).toString().isEmpty()?0:polarList->findText(item->data(ROLE_POLAR).toString()));
         enable_state->setCheckState((Qt::CheckState)item->data(ROLE_ACTIVATED).toInt());
         lockChange->setCheckState((Qt::CheckState)item->data(ROLE_LOCKED).toInt());
         btn_accDel->setEnabled(true);
     }
}
