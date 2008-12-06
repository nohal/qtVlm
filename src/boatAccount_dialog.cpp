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

    QListIterator<boatAccount*> i (acc_list);

    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        QListWidgetItem * item = new QListWidgetItem(acc->getLogin(),list_boat);
        item->setData(Qt::UserRole,acc->getPass());
        item->setData(Qt::UserRole+1,
                      (acc->getStatus()?Qt::Checked:Qt::Unchecked));
    }

    blank = new QListWidgetItem("<Blank>",list_boat);
    list_boat->setCurrentItem(blank);
    slot_selectItem(blank);
}

void boatAccount_dialog::slot_accHasChanged(void)
{
    QListWidgetItem * curItem = list_boat->currentItem();
    if(curItem != blank && (edit_login->text() != curItem->text()
            || edit_pass->text() != curItem->data(Qt::UserRole).toString()
            || enable_state->checkState() != ((Qt::CheckState)curItem->data(Qt::UserRole+1).toInt())))
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
                    item->data(Qt::UserRole).toString(),
                    ((Qt::CheckState)item->data(Qt::UserRole+1).toInt())==Qt::Checked);

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
                        item->data(Qt::UserRole).toString(),
                        ((Qt::CheckState)item->data(Qt::UserRole+1).toInt())==Qt::Checked,
                         proj,main,parent);
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

void boatAccount_dialog::slot_addBoat(void)
{
     /* empty login or empty password*/
     if(edit_login->text().isEmpty() || edit_pass->text().isEmpty())
        return;

     /*entry already exists*/
     if(list_boat->findItems(edit_login->text(),Qt::MatchFixedString).isEmpty())
     {
        QListWidgetItem * item = new QListWidgetItem(edit_login->text());
        item->setData(Qt::UserRole,edit_pass->text());
        item->setData(Qt::UserRole+1,enable_state->checkState());
        list_boat->insertItem(list_boat->count()-1,item);
        list_boat->setCurrentItem(item);
        edit_login->setText("");
        edit_pass->setText("");
        enable_state->setCheckState(Qt::Unchecked);
     }
     btn_accChg->setEnabled(false);
}

void boatAccount_dialog::slot_delBoat(void)
{
     QListWidgetItem * item = list_boat->currentItem();
     if(item)
     {
        edit_login->setText("");
        edit_pass->setText("");
        enable_state->setCheckState(Qt::Unchecked);
        delete(item);
        if(list_boat->currentItem() == blank)
        {
           btn_accChg->setEnabled(false);
           btn_accDel->setEnabled(false);
        }
     }
     btn_accChg->setEnabled(false);
}

void boatAccount_dialog::slot_chgBoat(void)
{
     QListWidgetItem * item = list_boat->currentItem();
     if(item)
     {
        item->setText(edit_login->text());
        item->setData(Qt::UserRole,edit_pass->text());
        item->setData(Qt::UserRole+1,enable_state->checkState());
     }
     btn_accChg->setEnabled(false);
}

void  boatAccount_dialog::slot_selectItem( QListWidgetItem * item)
{
     if(item==blank)
     {
          edit_login->setText("");
          edit_pass->setText("");
          enable_state->setCheckState(Qt::Unchecked);
          btn_accChg->setEnabled(false);
          btn_accDel->setEnabled(false);
     }
     else
     {
         edit_login->setText(item->text());
         edit_pass->setText(item->data(Qt::UserRole).toString());
         enable_state->setCheckState((Qt::CheckState)item->data(Qt::UserRole+1).toInt());
         btn_accDel->setEnabled(true);
     }
}
