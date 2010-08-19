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

struct boatParam {
    QString login;
    QString name;
    QString user_id;
    bool selected;
};

struct raceParam {
    QString id;
    QString name;
    QList <boatParam*> boats;
    bool displayNSZ;
    double latNSZ;
    double widthNSZ;
    QColor colorNSZ;
};

class race_dialog : public QDialog, public Ui::race_dialog_ui, public inetClient
{
    Q_OBJECT

    public:
        race_dialog(MainWindow * main,myCentralWidget * parent, inetConnexion * inet);
        ~race_dialog();
        void done(int result);
        void initList(QList<boatAccount*> & acc_list,QList<raceData*> & race_list);
        void requestFinished (QByteArray);

    public slots:
        void chgRace(int id);
        void addBoat(void);
        void delBoat(void);
        void delAllBoat(void);
        void doSynch(void);


    signals:
        void readRace(void);
        void writeBoat(void);
        void updateOpponent(void);

    private:
        QList<boatAccount*> * acc_list;
        QList<raceData*> * race_list;

        QList<raceParam*> param_list;

        QMessageBox * waitBox;

        bool initDone;

        int numRace;

        void clear(void);
        void getNextRace();
        void mvBoat(QListWidget * from,QListWidget * to,bool withLimit);
        void mvAllBoat(QListWidget * from,QListWidget * to);

        void saveData(bool);

        /* http connection */
        int currentRace;
        QStringList currentParam;
        bool currentDisplayNSZ;
        double currentLatNSZ;
        double currentWidthNSZ;
        QColor currentColorNSZ;
        InputLineParams *inputTraceColor;

private slots:
    void on_displayNSZ_clicked();
};

#endif
