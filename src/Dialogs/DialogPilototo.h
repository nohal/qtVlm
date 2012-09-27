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

class DialogPilototoInstruction : public QWidget, public Ui::instruction_ui
{Q_OBJECT
    public:
        DialogPilototoInstruction(QWidget * main,QWidget * parent=0);

        int   getMode(void)       { return mode_scr; }
        double getAngle(void)      { return angle_scr; }
        double getLat(void)        { return lat_scr; }
        double getLon(void)        { return lon_scr; }
        double getWph(void)        { return wph_scr; }
	bool  getLockStatus(void) { return locked; }
	int   getRef(void)        { return ref; }
        int   getStatus(void)     { return status_scr; }
        int   getTstamp(void)     { return tstamp_scr.toTime_t(); }
	bool  getHasChanged(void) { return hasChanged; }
        QString getPip(void);

	void setMode(int val);
        void setAngle(double val);
        void setLat(double val);
        void setLon(double val);
        void setWph(double val);
	void setLock(bool status);
	void setStatus(int val);
	void setTstamp(int val);

	void setRef(int val) { ref=val; }

        void initVal(int mode_ini,double angle_ini,double lat_ini,double lon_ini, double wph_ini,int ref_ini);
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
        void doDelInstruction(DialogPilototoInstruction*);
        void doEditInstruction(DialogPilototoInstruction*);
        void instructionUpdated(void);
        void selectPOI(DialogPilototoInstruction *,int);

    private:
	int mode;
        double angle;
        double lat;
        double lon;
        double wph;
        int mode_scr;
        double angle_scr;
        double lat_scr;
        double lon_scr;
        double wph_scr;
	QDateTime tstamp;
        QDateTime tstamp_scr;
	int ref;
	int status;
        int status_scr;

	bool locked;
	bool hasChanged;

	QWidget * parent;
        QWidget * pilototo;

        QPalette pipPalette;
        QColor pipColor;

        void updateText(bool);
        bool chkHasChanged(void);
        bool checkPIP(bool savChange,bool chgColor);
        void pickPipColor(void);


};

/******************************
* Pilototo écran principal
******************************/

#include "ui_Pilototo.h"

struct instruction {
    int script;
    QByteArray param;
};

class DialogPilototo : public QDialog, public Ui::pilototo_ui, public inetClient
{Q_OBJECT
    public:
        DialogPilototo(MainWindow *main,myCentralWidget * parent,inetConnexion * inet);
        void done(int);
        DialogPilototoParam * instructionEditor;

        /* inetClient */
        void requestFinished(QByteArray res);
        QString getAuthLogin(bool * ok);
        QString getAuthPass(bool * ok);

    public slots:
        void delInstruction(DialogPilototoInstruction *);
        void setInstructions(boat *pvBoat,QList<POI*> pois);
	void editInstructions(void);
        void editInstructionsPOI(DialogPilototoInstruction * instruction,POI * poi);
	void instructionUpdated(void);
        void slot_boatUpdated(boat * pvBoat);
        void updateTime(void);
        void doSelectPOI(DialogPilototoInstruction * instruction, int type); /* 1=instruction, 2=editor */

    signals:
        void selectPOI(DialogPilototoInstruction *);

    private slots:
	void addInstruction(void);

    private:
        myCentralWidget * parent;
        QList<DialogPilototoInstruction*> instructions_list;
        QList<DialogPilototoInstruction*> drawList;
	QList<int> delList;
	QMessageBox * waitBox;
	QVBoxLayout * frameLayout;
        QString lastOrder;

        int selectPOI_mode;

        boatVLM * myBoat;
	int nbInstruction;

    bool navModeToDo;
    void updateDrawList(void);
	void updateNbInstruction(void);

        QList<struct instruction*> * currentList;

        void sendPilototo(void);
        bool updateBoat;
        POI * poiToWp;
};

#endif
