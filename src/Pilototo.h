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

#ifndef PILOTOTO_H
#define PILOTOTO_H

#include <QList>
#include <QMessageBox>


#include <QNetworkAccessManager>
#include <QNetworkReply>

class Pilototo;

#include "Pilototo_param.h"
#include "boatAccount.h"

#define PILOTOTO_STATUS_DONE    0
#define PILOTOTO_STATUS_PENDING 1
#define PILOTOTO_STATUS_NEW     2

/******************************
* Pilototo instruction
* widget + data structure
******************************/
#include "ui_instructions.h"

class Pilototo_param;

class Pilototo_instruction : public QWidget, public Ui::instruction_ui
{Q_OBJECT
    public:
        Pilototo_instruction(QWidget * main,QWidget * parent=0);

        int   getMode(void)       { return mode; }
        float getAngle(void)      { return angle; }
        float getLat(void)        { return lat; }
        float getLon(void)        { return lon; }
        float getWph(void)        { return wph; }
        bool  getLockStatus(void) { return locked; }
        int   getRef(void)        { return ref; }
        int   getStatus(void)     { return status; }
        int   getTstamp(void)     { return tstamp.toTime_t(); }
        bool  getHasChanged(void) { return hasChanged; }
        QString getPip(void);

        void setMode(int val);
        void setAngle(float val);
        void setLat(float val);
        void setLon(float val);
        void setWph(float val);
        void setLock(bool status);
        void setStatus(int val);
        void setTstamp(int val);

        void setRef(int val) { ref=val; }

        void initVal(void);
        void updateHasChanged(bool status);

    private slots:
        void delInstruction(void);
        void editInstruction(void);
        void pastePOI(void);
        void validateModif(void);
        void dateTime_changed(QDateTime);
        void maintenant(void);

    signals:
        void doDelInstruction(Pilototo_instruction*);
        void doEditInstruction(Pilototo_instruction*);
        void instructionUpdated(void);

    private:
        int mode;
        float angle;
        float lat;
        float lon;
        float wph;
        QDateTime tstamp;
        int ref;
        int status;

        bool locked;
        bool hasChanged;

        QWidget * parent;
        QWidget * pilototo;

        void updateText(void);


};

/******************************
* Pilototo écran principal
******************************/

#include "ui_Pilototo.h"

class Pilototo : public QDialog, public Ui::pilototo_ui
{Q_OBJECT
    public:
        Pilototo(QWidget * parent=0);
        void done(int);

        void updateProxy(void);

        Pilototo_param * instructionEditor;

    public slots:
        void delInstruction(Pilototo_instruction *);
        void editInstructions(void);
        void instructionUpdated(void);
        void boatUpdated(boatAccount * boat);
        void updateTime(void);
        void requestFinished (QNetworkReply*);

    private slots:
        void addInstruction(void);

    private:
        QWidget * parent;
        QList<Pilototo_instruction*> instructions_list;
        QList<Pilototo_instruction*> drawList;
        QList<int> delList;
        QMessageBox * waitBox;
        QVBoxLayout * frameLayout;

        boatAccount * boat;
        int nbInstruction;

        void updateDrawList(void);
        void updateNbInstruction(void);

        QNetworkAccessManager *inetManager;
        QString host;
        int currentRequest;
        QStringList * currentList;

        void sendPilototo(QStringList * cmdList);
};

#endif
