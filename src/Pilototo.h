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

#include "boat.h"
#include "class_list.h"

#include "inetClient.h"

#define PILOTOTO_STATUS_DONE    0
#define PILOTOTO_STATUS_PENDING 1
#define PILOTOTO_STATUS_NEW     2
#define PILOTOTO_STATUS_CHG     3

/******************************
* Pilototo instruction
* widget + data structure
******************************/
#include "ui_instructions.h"

class Pilototo_instruction : public QWidget, public Ui::instruction_ui
{Q_OBJECT
    public:
	Pilototo_instruction(QWidget * main,QWidget * parent=0);

        int   getMode(void)       { return mode_scr; }
        float getAngle(void)      { return angle_scr; }
        float getLat(void)        { return lat_scr; }
        float getLon(void)        { return lon_scr; }
        float getWph(void)        { return wph_scr; }
	bool  getLockStatus(void) { return locked; }
	int   getRef(void)        { return ref; }
        int   getStatus(void)     { return status_scr; }
        int   getTstamp(void)     { return tstamp_scr.toTime_t(); }
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

        void initVal(int mode_ini,float angle_ini,float lat_ini,float lon_ini, float wph_ini,int ref_ini);
	void initVal(void);
	void updateHasChanged(bool status);

    private slots:
	void delInstruction(void);
	void editInstruction(void);
	void pastePOI(void);
	void copyPOI(void);
	void validateModif(void);
        void cancelModif(void);
	void dateTime_changed(QDateTime);
	void maintenant(void);
        void doSelectPOI();
        void modeChanged(int index);
        void pipChanged(QString);
        void pipValidated(void);

    signals:
	void doDelInstruction(Pilototo_instruction*);
	void doEditInstruction(Pilototo_instruction*);
        void instructionUpdated(void);
        void selectPOI(Pilototo_instruction *,int);

    private:
	int mode;
	float angle;
	float lat;
	float lon;
	float wph;
        int mode_scr;
        float angle_scr;
        float lat_scr;
        float lon_scr;
        float wph_scr;
	QDateTime tstamp;
        QDateTime tstamp_scr;
	int ref;
	int status;
        int status_scr;

	bool locked;
	bool hasChanged;

	QWidget * parent;
        QWidget * pilototo;

        void updateText(bool);
        bool chkHasChanged(void);
        bool checkPIP(bool savChange,bool chgColor);


};

/******************************
* Pilototo écran principal
******************************/

#include "ui_Pilototo.h"

struct instruction {
    int script;
    QByteArray param;
};

class Pilototo : public QDialog, public Ui::pilototo_ui, public inetClient
{Q_OBJECT
    public:
        Pilototo(MainWindow *main,myCentralWidget * parent,inetConnexion * inet);
        void done(int);
	Pilototo_param * instructionEditor;

        /* inetClient */
        void requestFinished(QByteArray res);
        QString getAuthLogin(bool * ok);
        QString getAuthPass(bool * ok);

    public slots:
	void delInstruction(Pilototo_instruction *);
	void editInstructions(void);
	void editInstructionsPOI(Pilototo_instruction * instruction,POI * poi);
	void instructionUpdated(void);
        void slot_boatUpdated(boat * pvBoat);
        void updateTime(void);
        void doSelectPOI(Pilototo_instruction * instruction, int type); /* 1=instruction, 2=editor */

    signals:
        void selectPOI(Pilototo_instruction *);

    private slots:
	void addInstruction(void);

    private:
        myCentralWidget * parent;
        QList<Pilototo_instruction*> instructions_list;
	QList<Pilototo_instruction*> drawList;
	QList<int> delList;
	QMessageBox * waitBox;
	QVBoxLayout * frameLayout;

        QString lastOrder;

        int selectPOI_mode;

        boatVLM * myBoat;
	int nbInstruction;

	void updateDrawList(void);
	void updateNbInstruction(void);

        QList<struct instruction*> * currentList;

        void sendPilototo(void);
};

#endif
