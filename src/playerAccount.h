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

#ifndef PLAYERACCOUNT_H
#define PLAYERACCOUNT_H

#include <QDialog>
#include <QMessageBox>

#include "ui_playerAccount.h"
#include "ui_paramAccount.h"

#include "class_list.h"

struct player_data {
    QString login;
    QString pass;
    int type;
};

class paramAccount : public QDialog, public Ui::paramAccount
{ Q_OBJECT
    public:
        paramAccount(QWidget * parent=NULL);
        bool initDialog(player_data * data);

    public slots:
        void slot_typeChanged(int);
};

class playerAccount : public QDialog, public Ui::playerAccount
{ Q_OBJECT
    public:
        playerAccount(Projection * proj, MainWindow * main,
                  myCentralWidget * parent, inetConnexion * inet);

        void done(int result);
        void initList(QList<Player*> * player_list);
        void doUpdate(Player * player);

    public slots:
        void slot_addPlayer(void);
        void slot_delPlayer(void);
        void slot_modPlayer(void);
        void slot_updPlayer(void);

        void slot_selectItem_player(QListWidgetItem *);
        void slot_selectAndValidateItem(QListWidgetItem* item);
        void slot_updFinished(bool res, Player * player);

    signals:
        void addPlayer(Player*);
        void delPlayer(Player*);
        void addBoat(boatVLM*);
        void delBoat(boatVLM*);
        void playerSelected(Player*);
        void writeBoat(void);
        void reloadPlayer();

    private:
        QList<Player*> * player_list;

        QMap<int,Player*> players;
        int player_idx;
        int curPlayerIdp;
        Projection * proj;
        MainWindow * main;
        myCentralWidget * parent;
        inetConnexion * inet;

        QMessageBox * msgBox;

        paramAccount accDialog;

        void removeBoats(Player * player);
        void setPlayerItemName(QListWidgetItem * item,Player * player);
        void updPlayer(Player * player);
        void updBtnAndString(void);
};

#endif // PLAYERACCOUNT_H
