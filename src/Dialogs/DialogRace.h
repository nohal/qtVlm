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



#ifndef RACE_DIALOG_H
#define RACE_DIALOG_H

#include <QDialog>
#include <QMessageBox>

#include "ui_race_dialog.h"

#include "class_list.h"

#include "inetClient.h"
#include <QStandardItemModel>
#include <dataDef.h>



class DialogRace : public QDialog, public Ui::race_dialog_ui, public inetClient
{
    Q_OBJECT

    public:
        DialogRace(MainWindow * main,myCentralWidget * parent, inetConnexion * inet);
        ~DialogRace();
        void done(int result);
        void initList(QList<boatVLM*> & boat_list,QList<raceData*> & race_list);
        void requestFinished (QByteArray);
        QString getAuthLogin(bool * ok=NULL);
        QString getAuthPass(bool * ok=NULL);

    public slots:
        void chgRace(int id);
        void doSynch(void);
        void itemChanged(QStandardItem *);
        void myListToggle(bool b);
        void tenFirstToggle(bool b);
        void tenFirstDistToggle(bool b);
        void tenFirstRankToggle(bool b);
        void noneToggle(bool b);
        void showRealToggle(bool b);
        void NSZToggle(bool b);
        void slotFilterReal();

    signals:
        void readRace(void);
        void writeBoat(void);
        void updateOpponent(void);

    private:
        QList<boatVLM*> * boat_list;
        QList<raceData*> * race_list;

        QList<raceParam*> param_list;

        QMessageBox * waitBox;

        bool initDone;

        int numRace;

        void clear(void);
        void getNextRace();

        void saveData(bool);

        /* http connection */
        int currentRace;
        QStringList currentParam;
        bool currentDisplayNSZ;
        double currentLatNSZ;
        double currentWidthNSZ;
        bool currentShowReal;
        bool currentHasReal;
        QString currentFilterReal;
        QColor currentColorNSZ;
        int currentShowWhat;
        InputLineParams *inputTraceColor;
        int showWhat;
        MainWindow *main;
        myCentralWidget *parent;
        QStandardItemModel * model;
        QStandardItemModel * modelResult;
        int nbSelected;
        QString imgFileName;
        int jj;
        void getMissingFlags();
        bool somethingChanged;

private slots:
    void on_displayNSZ_clicked();
};

#endif
