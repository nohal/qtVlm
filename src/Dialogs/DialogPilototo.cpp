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

#include <QTextStream>
#include <QDebug>
#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "Util.h"
#include "boat.h"
#include "boatVLM.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "POI.h"
#include "dataDef.h"

#include "DialogPilototo.h"
#include "DialogPilototoParam.h"
#include "settings.h"

DialogPilototo::DialogPilototo(MainWindow *main,myCentralWidget * parent,inetConnexion * inet):QDialog(parent), inetClient(inet)
{
    this->parent=parent;
    poiToWp=NULL;
    navModeToDo=false;
    this->move(250,100);
    setupUi(this);
    Util::setFontDialog(this);
    selectPOI_mode=1;

    needAuth=true;

    instructionEditor = new DialogPilototoParam(this);
    connect(instructionEditor,SIGNAL(doSelectPOI(DialogPilototoInstruction *,int)),
            this,SLOT(doSelectPOI(DialogPilototoInstruction *,int)));
    connect(this,SIGNAL(selectPOI(DialogPilototoInstruction *)),
            main,SLOT(slotSelectPOI(DialogPilototoInstruction *)));
    connect(main,SIGNAL(editInstructions()),
            this,SLOT(editInstructions()));
    connect(main,SIGNAL(setInstructions(boat *,QList<POI*>)),this,SLOT(setInstructions(boat *,QList<POI*>)));
    connect(main,SIGNAL(editInstructionsPOI(DialogPilototoInstruction * ,POI * )),
            this,SLOT(editInstructionsPOI(DialogPilototoInstruction * ,POI * )));
    connect(main,SIGNAL(boatHasUpdated(boat*)),this,SLOT(slot_boatUpdated(boat*)));

    btn_addInstruction->setIcon(QIcon(appFolder.value("img")+"add.png"));

    instructions_list.clear();
    drawList.clear();
    delList.clear();

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    frameLayout = new QVBoxLayout(frame);
    frameLayout->setSizeConstraint(QLayout::SetFixedSize);




    waitBox = new QMessageBox(QMessageBox::Information,
			     tr("Pilototo"),
                             tr("Chargement des instructions VLM en cours"),
                             QMessageBox::NoButton,this,Qt::SplashScreen);

    /* inet init */
    currentList=NULL;
    this->updateBoat=false;

}

void DialogPilototo::updateDrawList(void)
{
    this->btn_addInstruction->setDisabled(parent->getSelectedBoat()->getLockStatus());
    this->pushButton_3->setDisabled(parent->getSelectedBoat()->getLockStatus());
    bool added;
    /* remove all instruction from screen */
    for(int i=0;i<drawList.count();++i)
    {
	drawList[i]->hide();
	frameLayout->removeWidget(drawList[i]);
    }

    drawList.clear();

    for(int i=0;i<instructions_list.count();++i)
    {
        DialogPilototoInstruction * instr=instructions_list[i];
	added = false;
	if(instr->getHasChanged())
	{ /* date have not been validated */
	    /* search for the first not validated item */
	    bool found = false;
	    int j;
        for(j=0;j<drawList.count();++j)
	    {
		if(drawList[j]->getHasChanged())
		{
		    found=true;
		    break;
		}
	    }
	    if(!found) /* not added yet => append */
	    {
		drawList.append(instr);
	    }
	    else
	    {
		/* order not validated items by tstamp */
        for(/*not changing j*/;j<drawList.count();++j)
		{
		    if(drawList[j]->getTstamp() > instr->getTstamp())
		    {
			drawList.insert(j,instr);
			added=true;
			break;
		    }
		}
		if(!added) /* not added yet => append */
		{
		    drawList.append(instr);
		}
	    }
	}
	else
	{ /* date have been validated */
        for(int j=0;j<drawList.count();++j)
	    {
		if(drawList[j]->getHasChanged())
		{
		    /* j item is not validated => add just before */
		    drawList.insert(j,instr);
		    added=true;
		    break;
		}

		if(drawList[j]->getTstamp() > instr->getTstamp())
		{
		    /* j item is later than i */
		    drawList.insert(j,instr);
		    added=true;
		    break;
		}
	    }
	    if(!added)
	    {
		drawList.append(instr);
	    }
	}
    }

    /* we now have the list of items to be shown => adding them to grid layout */

    for(int j=0;j<drawList.count();++j)
    {
	frameLayout->addWidget(drawList[j], j, 0);
	drawList[j]->show();
    }
}

/* init pilototo editor */
void DialogPilototo::editInstructions(void)
{
//    waitBox->exec();
    waitBox->show();
}

/*init piloto editor after selecting a POI*/
void DialogPilototo::editInstructionsPOI(DialogPilototoInstruction * instruction,POI * poi)
{
    show();
    if(selectPOI_mode==1)
    {
        if(poi)
        {
            instruction->setLat(poi->getLatitude());
            instruction->setLon(poi->getLongitude());
            instruction->setWph(poi->getWph());
        }
    }
    else
        instructionEditor->editInstructionPOI(instruction,poi);
}

void DialogPilototo::doSelectPOI(DialogPilototoInstruction * instruction,int type) /* 1=instruction, 2=editor */
{
    selectPOI_mode=type;
    emit selectPOI(instruction);
    QDialog::done(QDialog::Rejected);
    parent->activateWindow();
}

void DialogPilototo::slot_boatUpdated(boat * pvBoat)
{
    if(!waitBox->isVisible())
	return;
    waitBox->hide();
    boatVLM * my_boat=(boatVLM*)pvBoat;
    this->myBoat=my_boat;
    int mode;
    int ref;
    double angle=0;
    double lat=0,lon=0;
    double wph=-1;
    int pos;

    QStringList list = my_boat->getPilototo();

    titreBateau->setText(tr("Pilototo pour ") + my_boat->getBoatPseudo());

    delList.clear(); /* this is a list of int => no delete*/
    drawList.clear(); /*all item are also in instructions_list */

    while (!instructions_list.isEmpty())
	delete instructions_list.takeFirst();

    if(!my_boat->getHasPilototo())
	QMessageBox::information (this,
	    tr("Pilototo"),
            tr("La recuperation des donnees pilototo de VLM n'a pas fonctionne\nVous pouvez ajouter des instructions mais sans voir le resultat dans QtVlm"),
	    QMessageBox::Ok);

    for(int i=0;i<list.count();++i)
    {
    QString instr_txt=list.at(i);
	if(instr_txt!="none")
	{
	    QStringList instr_buf = instr_txt.split(",");
            //qWarning() << "Parsing PIL" << i << ": " << instr_txt;

	    if(instr_buf.count() == 6 || instr_buf.count() == 5)
	    {
                DialogPilototoInstruction * instr = new DialogPilototoInstruction(this,frame);
		instructions_list.append(instr);
                //instr->setRef(instr_buf.at(0).toInt());
                ref=instr_buf.at(0).toInt();
		instr->setTstamp(instr_buf.at(1).toInt());
		mode=instr_buf.at(2).toInt()-1;
                //instr->setMode(mode);
		if(mode == 0 || mode == 1)
		{
                    //instr->setAngle(instr_buf.at(3).toDouble());
                    angle=instr_buf.at(3).toDouble();
		    pos=4;
		}
		else
		{
                    //instr->setLat(instr_buf.at(3).toDouble());
                    lat=instr_buf.at(3).toDouble();
		    QStringList instr_buf2 = instr_buf.at(4).split("@");
		    if(instr_buf2.count() == 2)
		    {
                        //instr->setLon(instr_buf2.at(0).toDouble());
                        lon=instr_buf2.at(0).toDouble();
                        //instr->setWph(instr_buf2.at(1).toDouble());
                        wph=instr_buf2.at(1).toDouble();
		    }
		    else
		    {
                        //instr->setLon(instr_buf.at(4).toDouble());
                        lon=instr_buf2.at(0).toDouble();
		    }
		    pos=5;
		}
		if(instr_buf.at(pos) == "done")
                    instr->setStatus(PILOTOTO_STATUS_DONE);
		else if(instr_buf.at(pos) == "pending")
		    instr->setStatus(PILOTOTO_STATUS_PENDING);
                instr->initVal(mode,angle,lat,lon,wph,ref);
		instr->updateHasChanged(false);
	    }
	}
    }



    updateNbInstruction();

    /* init current time */
    updateTime();

    updateDrawList();
    exec();
}

void DialogPilototo::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(result==QDialog::Accepted)
    {
	/* checking if there is un validated instructions */
	bool hasUnValidated =false;
    for(int i=0;i<instructions_list.count();++i)
	{
	    if(instructions_list[i]->getHasChanged())
	    {
		hasUnValidated=true;
		break;
	    }
	}

	if(hasUnValidated)
	{
	    int rep = QMessageBox::question (this,
            tr("Instructions non Validees"),
            tr("Il reste des instructions non validees. Elles ne seront pas envoyees a VLM\nContinuer la sauvegarde?"),
	    QMessageBox::Yes | QMessageBox::No);
	    if (rep == QMessageBox::No)
		return;
	}
	/* creating list of pilototo.php requests*/
        QList<struct instruction*> * instructions = new QList<struct instruction*>;
        QJson::Serializer serializer;
        struct instruction * instr_ptr;
	/* processing del */
    for(int i=0;i<delList.count();++i)
        {
            QVariantMap cur_instruction;
            cur_instruction.insert("taskid",delList[i]);
            cur_instruction.insert("idu",myBoat->getBoatId().toInt());
            instr_ptr=new struct instruction();
            instr_ptr->script=PILOT_DEL;
            instr_ptr->param=serializer.serialize(cur_instruction);
            instructions->append(instr_ptr);
        }
	/* processing others */
    for(int i=0;i<instructions_list.count();++i)
	{
            DialogPilototoInstruction * instr=instructions_list[i];
            if(instr->getTstamp()<(int)QDateTime::currentDateTime().toUTC().toTime_t()) continue;
	    if(!instr->getHasChanged())
	    { /* only processing activated and validated instructions */
                QVariantMap cur_instruction;
                QVariantMap pip;
                int type;
                int mode=instr->getMode()+1;
                cur_instruction.insert("idu",myBoat->getBoatId().toInt());
		if(instr->getRef()!=-1) /* updating */
                {
                    type=PILOT_UPD;
                    cur_instruction.insert("taskid",instr->getRef());
                }
		else /* adding */
                {
                    type=PILOT_ADD;
                }
                cur_instruction.insert("pim",mode);
                cur_instruction.insert("tasktime",instr->getTstamp());

                switch(mode)
                {
                case 1:
                case 2:
                    cur_instruction.insert("pip",(double)instr->getAngle());
                    break;
                case 3:
                case 4:
                case 5:
                    pip.insert("targetlat",QString().sprintf("%.10f",(double)instr->getLat()));
                    pip.insert("targetlong",QString().sprintf("%.10f",(double)instr->getLon()));
                    if(instr->getWph()!=-1)
                        pip.insert("targetandhdg",QString().sprintf("%.2f",(double)instr->getWph()));
                    cur_instruction.insert("pip",pip);
                    break;
                }
                instr_ptr=new struct instruction();
                instr_ptr->script=type;
                instr_ptr->param=serializer.serialize(cur_instruction);
                instructions->append(instr_ptr);
	    }
	}
	/* ready to send */
        currentList=instructions;

        for(int i=0;i<currentList->count();++i)
            qWarning() << i << ": " << currentList->at(i)->script << " - " << currentList->at(i)->param;

        sendPilototo();
    }

    while (!instructions_list.isEmpty())
	delete instructions_list.takeFirst();

    QDialog::done(result);
}



void DialogPilototo::sendPilototo(void)
{
    if(!currentList)
    {
        qWarning("error can't send pilototo list does not exists");
        return;
    }

    if(!hasInet() || hasRequest())
    {
        qWarning("error can't send pilototo (nb instr:%d)",currentList->count());
        return;
    }

    if(currentList->isEmpty())
    {
        //qWarning() << "Piloto instruction list is empty";
        /* ask for an update of boat data*/
        delete currentList;
        currentList=NULL;
        myBoat->slot_getData(true);
    }
    else
    {
        struct instruction* data = currentList->takeFirst();
        QString scriptList[3]={"pilototo_add.php","pilototo_update.php","pilototo_delete.php" };
        clearCurrentRequest();
        //qWarning() << "Sending: " << data->script << "(" << scriptList[data->script] << ") - " << data->param;
        lastOrder="/ws/boatsetup/" + scriptList[data->script] + " | " + "parms="+data->param+"&select_idu="+QString().setNum(myBoat->getId());
        inetPost(VLM_DO_REQUEST,"/ws/boatsetup/" + scriptList[data->script],
                 "parms="+data->param+"&select_idu="+QString().setNum(myBoat->getId()));
        delete data;
    }
}

void DialogPilototo::requestFinished (QByteArray res)
{
    switch(getCurrentRequest())
    {
	case VLM_REQUEST_LOGIN:
            qWarning() << "Error pilototo: getting res for LOGIN!!!";
            break;
	case VLM_DO_REQUEST:
            {
                if(checkWSResult(res,"Pilototo",parent,lastOrder))
                    sendPilototo();
                else
                {
                    currentList->clear();
                    delete currentList;
                    currentList=NULL;
                }
            }
	    break;
    case 999:
    {

    }
    }
    if(this->myBoat && this->updateBoat && (this->currentList==NULL || this->currentList->isEmpty()))
    {
#if 1
        if(poiToWp!=NULL && navModeToDo)
        {
            if(poiToWp->getNavMode()==0 && this->myBoat->getPilotType()!=5) //VBVMG
            {
                QString url="/ws/boatsetup/pilot_set.php";
                QString data="parms={ \"idu\" : "+QString().setNum(myBoat->getId())+
                        ", \"pim\" : 5}&select_idu="+
                        QString().setNum(myBoat->getId());
                qWarning()<<"route sends"<<url<<data;
                navModeToDo=false;
                inetPost(999,"/ws/boatsetup/pilot_set.php",
                         "parms={ \"idu\" : "+QString().setNum(myBoat->getId())+
                         ", \"pim\" : 5}&select_idu="+
                         QString().setNum(myBoat->getId()));
                return;
            }
            else if(poiToWp->getNavMode()==1 && this->myBoat->getPilotType()!=4) //VMG
            {
                navModeToDo=false;
                inetPost(999,"/ws/boatsetup/pilot_set.php",
                         "parms={ \"idu\" : "+QString().setNum(myBoat->getId())+
                         ", \"pim\" : 4}&select_idu="+
                         QString().setNum(myBoat->getId()));
                return;
            }
            else if(poiToWp->getNavMode()==2 && this->myBoat->getPilotType()!=3) //ORTHO
            {
                navModeToDo=false;
                inetPost(999,"/ws/boatsetup/pilot_set.php",
                         "parms={ \"idu\" : "+QString().setNum(myBoat->getId())+
                         ", \"pim\" : 3}&select_idu="+
                         QString().setNum(myBoat->getId()));
                return;
            }
        }
#endif
        navModeToDo=false;
        this->updateBoat=false;
        if(poiToWp!=NULL)
        {
            poiToWp->slot_setWP();
            poiToWp=NULL;
        }
    }
}

QString DialogPilototo::getAuthLogin(bool * ok=NULL)
{
    if(myBoat)
    {
        if(ok) *ok=true;
        return myBoat->getAuthLogin();
    }
    else
    {
        if(ok) *ok=false;
        return QString();
    }
}

QString DialogPilototo::getAuthPass(bool * ok=NULL)
{
    if(myBoat)
    {
        if(ok) *ok=true;
        return myBoat->getAuthPass();
    }
    else
    {
        if(ok) *ok=false;
        return QString();
    }
}


void DialogPilototo::addInstruction(void)
{
    if(nbInstruction<=5)
    {
        DialogPilototoInstruction * instr = new DialogPilototoInstruction(this,frame);
	instructions_list.append(instr);
	instr->initVal();
	updateDrawList();
	updateNbInstruction();
    }
}

void DialogPilototo::updateTime(void)
{
    QFontMetrics fmt(curTime->font());
    QDateTime tm = QDateTime::currentDateTime().toUTC();
    QString data=tm.toString(tr("(ddd) dd/MM/yyyy hh:mm:ss"));
    curTime->setMinimumWidth(fmt.width(data)+10);
    curTime->setText(data);
}

void DialogPilototo::delInstruction(DialogPilototoInstruction * instruction)
{
    int ref=instruction->getRef();
    if(ref!=-1)
	delList.append(ref);
    instructions_list.removeAll(instruction);

    updateDrawList();
    updateNbInstruction();

    instruction->deleteLater();
}
void DialogPilototo::setInstructions(boat * pvBoat, QList<POI *> pois)
{
    boatVLM * my_boat=(boatVLM*)pvBoat;
    this->myBoat=my_boat;
    if(pois.count()>0)
    {
        for(int n=1;n<pois.count();++n)
        {
            if(pois.at(n)->getPiloteDate()<QDateTime().currentDateTimeUtc().toTime_t()+1.5*myBoat->getVacLen())
            {
                QMessageBox::critical(0,tr("Mise a jour Pilototo"),tr("Certains ordres ont des dates trop proches ou dans le passe"));
                return;
            }
        }
    }
    QList<struct instruction*> * instructions = new QList<struct instruction*>;
    QJson::Serializer serializer;
    struct instruction * instr_ptr;
    QStringList plist = my_boat->getPilototo();
    for(int n=0;n<plist.count();++n)
    {
        QStringList i=plist.at(n).split(",");
        if(i.at(0)=="none") continue;
        QVariantMap cur_instruction;
        cur_instruction.insert("taskid",i.at(0).toInt());
        cur_instruction.insert("idu",myBoat->getBoatId().toInt());
        instr_ptr=new struct instruction();
        instr_ptr->script=PILOT_DEL;
        instr_ptr->param=serializer.serialize(cur_instruction);
        instructions->append(instr_ptr);
    }
    for(int n=1;n<pois.count();++n)
    {
        POI * poi=pois.at(n);
        if(poi->getPiloteDate()==-1) continue;
        if(poi->getPiloteDate()+30<=(int)QDateTime().currentDateTimeUtc().toTime_t()) continue;
        QVariantMap cur_instruction;
        QVariantMap pip;
        instr_ptr=new struct instruction();
        instr_ptr->script=PILOT_ADD;
        int mode=5;
        if(poi->getNavMode()==1)
            mode=4;
        else if(poi->getNavMode()==2)
            mode=3;
        cur_instruction.insert("pim",mode);
        cur_instruction.insert("tasktime",(int)poi->getPiloteDate());
        pip.insert("targetlat",QString().sprintf("%.10f",poi->getLatitude()));
        pip.insert("targetlong",QString().sprintf("%.10f",poi->getLongitude()));
        if(poi->getWph()!=-1)
            pip.insert("targetandhdg",QString().sprintf("%.2f",poi->getWph()));
        cur_instruction.insert("pip",pip);
        cur_instruction.insert("idu",myBoat->getBoatId().toInt());
        instr_ptr->param=serializer.serialize(cur_instruction);
        instructions->append(instr_ptr);
        poi->setPiloteDate(-1);
    }
    if(!pois.isEmpty())
    {
        poiToWp=pois.at(0);
        navModeToDo=true;
    }
    else
    {
        poiToWp=NULL;
        navModeToDo=false;
    }
    currentList=instructions;
    this->updateBoat=true;
    sendPilototo();
}

void DialogPilototo::updateNbInstruction(void)
{
    nbInstruction=instructions_list.count();

    if(nbInstruction==0)
	frame->hide();
    else
	frame->show();

    if(nbInstruction==5)
	btn_addInstruction->setEnabled(false);
    else
	btn_addInstruction->setEnabled(true);

    txt_nbInstructions->setText(QString().setNum(nbInstruction)+"/5 "
	    +(nbInstruction>1?tr("instructions"):tr("instruction")));
}

void DialogPilototo::instructionUpdated(void)
{
    updateDrawList();
}

/******************************
* Pilototo instruction
* widget + data structure
******************************/

DialogPilototoInstruction::DialogPilototoInstruction(QWidget * main,QWidget * parent) : QWidget(parent)
{
    setupUi(this);
    pipPalette=parent->palette();
    connect(this,SIGNAL(doEditInstruction(DialogPilototoInstruction*)),
                ((DialogPilototo*)main)->instructionEditor,SLOT(editInstruction(DialogPilototoInstruction*)));
    connect(this,SIGNAL(doDelInstruction(DialogPilototoInstruction*)),
                main,SLOT(delInstruction(DialogPilototoInstruction *)));
    connect(this,SIGNAL(instructionUpdated()),
                main,SLOT(instructionUpdated()));
    connect(this,SIGNAL(selectPOI(DialogPilototoInstruction*,int)),main,SLOT(doSelectPOI(DialogPilototoInstruction*,int)));

    mode_sel->addItem(tr("Cap constant (1)"));
    mode_sel->addItem(tr("Angle du vent (2)"));
    mode_sel->addItem(tr("Pilote ortho (3)"));
    mode_sel->addItem(tr("Meilleur VMG (4)"));
    mode_sel->addItem(tr("VBVMG (5)"));

    btn_delInstruction->setIcon(QIcon(appFolder.value("img")+"del.png"));
    btn_validate->setIcon(QIcon(appFolder.value("img")+"apply.png"));
    btn_cancel->setIcon(QIcon(appFolder.value("img")+"undo.png"));
    btn_updateTime->setIcon(QIcon(appFolder.value("img")+"clock.png"));
    btn_copy->setIcon(QIcon(appFolder.value("img")+"copy.png"));
    btn_paste->setIcon(QIcon(appFolder.value("img")+"paste.png"));


    updateHasChanged(true); /*instruction is not saved when created*/
    initVal();
}

void DialogPilototoInstruction::initVal(void)
{
    mode=mode_scr=0;
    pickPipColor();
    mode_sel->setCurrentIndex(mode);
    angle=angle_scr=0;
    lat=lat_scr=0;
    lon=lon_scr=0;
    wph=wph_scr=-1;
    ref=-1;
    status=status_scr=PILOTOTO_STATUS_NEW;
    updateHasChanged(true);
    tstamp = QDateTime::currentDateTime().toUTC();
    tstamp_scr = QDateTime::currentDateTime().toUTC();
    horodate->setTimeSpec(Qt::UTC);
    horodate->setDateTime(tstamp);
    updateText(true);
    hide();
}

void DialogPilototoInstruction::initVal(int mode_ini,double angle_ini,double lat_ini,double lon_ini, double wph_ini,int ref_ini)
{
    mode=mode_scr=mode_ini;
    mode_sel->setCurrentIndex(mode);
    angle=angle_scr=angle_ini;
    lat=lat_scr=lat_ini;
    lon=lon_scr=lon_ini;
    wph=wph_scr=wph_ini;
    ref=ref_ini;
    updateText(true);
}

#define SETVAL(VAR)       \
{                         \
    if(VAR!=val)          \
    {                     \
        VAR=val;          \
        updateHasChanged(chkHasChanged()); \
        updateText(true);     \
    }                     \
}

void DialogPilototoInstruction::setMode(int val)
{
    SETVAL(mode_scr);
    mode_sel->setCurrentIndex(mode_scr);
    pickPipColor();
}

void DialogPilototoInstruction::setAngle(double val)
{
    SETVAL(angle_scr);
}

void DialogPilototoInstruction::setLat(double val)
{
    SETVAL(lat_scr);
}

void DialogPilototoInstruction::setLon(double val)
{
    SETVAL(lon_scr);
}

void DialogPilototoInstruction::setWph(double val)
{
    SETVAL(wph_scr);
}

void DialogPilototoInstruction::modeChanged(int index)
{

    if(!checkPIP(false,false))
    {
        QString sav=instructionText->text();
        setMode(index);
        instructionText->setText(sav);
    }
    else
        setMode(index);
    /* mode changed saving new data if ok */
    checkPIP(true,true);
}

bool DialogPilototoInstruction::chkHasChanged(void)
{
    return (mode!=mode_scr || lat!=lat_scr || lon!=lon_scr || wph!=wph_scr || angle!=angle_scr || tstamp!=tstamp_scr);
}

bool DialogPilototoInstruction::checkPIP(bool savChange,bool chgColor)
{
    double lat_val,lon_val,wph_val;
    int tstamp_int;
    double angle_val=0;
    bool ok;
    bool res=false;
    //qWarning() << "New PIP=" << instructionText->text() << " " << savChange << "-" << chgColor;
    switch(mode_scr)
    {
        case 0:
        case 1: /*converting a double */
            ok=false;
            if(!instructionText->text().contains(QChar(','),Qt::CaseInsensitive))
                angle_val=instructionText->text().toDouble(&ok);
            if(ok)
            {
                if(chgColor)
                {
                    QPalette p = instructionText->palette();
                    p.setColor( QPalette::Text, QColor(Qt::black) );
                    instructionText->setPalette(p);
                }
                if(savChange)
                    setAngle(angle_val);
                res=true;
            }
            else
            {
                if(chgColor)
                {
                    QPalette p = instructionText->palette();
                    p.setColor( QPalette::Text, QColor(Qt::red) );
                    instructionText->setPalette(p);
                }
            }
            break;
        case 2:
        case 3:
        case 4:
            if(!Util::convertPOI(instructionText->text(),NULL,&lat_val,&lon_val,&wph_val,&tstamp_int,0))
            {
                if(chgColor)
                {
                    QPalette p = instructionText->palette();
                    p.setColor( QPalette::Text, QColor(Qt::red) );
                    instructionText->setPalette(p);
                }
            }
            else
            {
                if(chgColor)
                {
                    QPalette p = instructionText->palette();
                    p.setColor( QPalette::Text, QColor(Qt::black) );
                    instructionText->setPalette(p);
                }
                if(savChange)
                {
                    setLat(lat_val);
                    setLon(lon_val);
                    setWph(wph_val);
                    if(tstamp_int!=-1)
                        tstamp_scr.setTime_t(tstamp_int);

                    if(wph_val<0 || wph_val > 360)
                        setWph(-1);
                    else
                        setWph(wph_val);
                }
                res=true;
            }
            break;
    }
    return res;
}

void DialogPilototoInstruction::pipChanged(QString)
{
    checkPIP(false,true);
}

void DialogPilototoInstruction::pipValidated(void)
{
    checkPIP(true,true);
    updateText(false);
}

void DialogPilototoInstruction::updateText(bool updateAll)
{
    QFontMetrics fmt(instructionText->font());
    QString final_txt;
    QString modeTxt,param;
    QString param_txt;

    switch(mode_scr)
    {
	case 0:
	    modeTxt="Cap";
            param=QString().setNum(angle_scr)+" °";
            param_txt=QString().setNum(angle_scr);
	    break;
	case 1:
	    modeTxt="Angle";
            param=QString().setNum(angle_scr)+" °";
            param_txt=QString().setNum(angle_scr);
	    break;
	case 2:
	    modeTxt="Ortho";
            if(wph_scr==-1)
            {
                param=QString("%1,%2")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr));
                param_txt=QString().sprintf("%.6f,%.6f",lat_scr,lon_scr);
            }
            else
            {
                param=QString("%1,%2@%3")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr))
                        .arg(wph_scr);
                param_txt=QString().sprintf("%.6f,%.6f@%.2f",lat_scr,lon_scr,wph_scr);
            }
	    break;
	case 3:
	    modeTxt="BVMG";
            if(wph_scr==-1)
            {
                param=QString("%1,%2")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr));
                param_txt=QString().sprintf("%.6f,%.6f",lat_scr,lon_scr);
            }
            else
            {
                param=QString("%1,%2@%3")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr))
                        .arg(wph_scr);
                param_txt=QString().sprintf("%.6f,%.6f@%.2f",lat_scr,lon_scr,wph_scr);
            }
	    break;
        case 4:
            modeTxt="VBVMG";
            if(wph_scr==-1)
            {
                param=QString("%1,%2")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr));
                param_txt=QString().sprintf("%.6f,%.6f",lat_scr,lon_scr);
            }
            else
            {
                param=QString("%1,%2@%3")
                        .arg(Util::pos2String(TYPE_LAT,lat_scr))
                        .arg(Util::pos2String(TYPE_LON,lon_scr))
                        .arg(wph_scr);
                param_txt=QString().sprintf("%.6f,%.6f@%.2f",lat_scr,lon_scr,wph_scr);
            }
            break;
    }
    final_txt=modeTxt+" = "+param;
    if(updateAll)
    {
        instructionText->setMinimumWidth( fmt.width(param_txt)+20 );
        instructionText->setText(param_txt);
    }
    instructionText->setToolTip(final_txt);

    switch(status_scr)
    {
	case PILOTOTO_STATUS_DONE:
	    status_txt->setText(tr("Passee"));
	    break;
	case PILOTOTO_STATUS_PENDING:
	    status_txt->setText(tr("En-cours"));
	    break;
	case PILOTOTO_STATUS_NEW:
	    status_txt->setText(tr("Nouveau"));
	    break;
        case PILOTOTO_STATUS_CHG:
            status_txt->setText(tr("Modifie"));
            break;
    }

    horodate->setDateTime(tstamp_scr);
}

void DialogPilototoInstruction::updateHasChanged(bool status)
{

    hasChanged = status;
    btn_validate->setEnabled(status);
    btn_cancel->setEnabled(status);
    if(status)
    {
        if(ref!=-1)
            status_scr=PILOTOTO_STATUS_CHG;
	btn_validate->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    }
    else
    {
        status_scr=this->status;
	btn_validate->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
    }
}

void DialogPilototoInstruction::delInstruction(void)
{
    emit doDelInstruction(this);
}

void DialogPilototoInstruction::editInstruction(void)
{
    emit doEditInstruction(this);
}

void DialogPilototoInstruction::pastePOI(void)
{
    double lat,lon,wph;
    int tstamp_int;
    if(!Util::getWPClipboard(NULL,&lat,&lon,&wph,&tstamp_int))
	return;

    this->lat_scr = lat;
    this->lon_scr = lon;
    mode_scr=2; /*default mode : ortho */

    if(tstamp_int!=-1)
        tstamp_scr.setTime_t(tstamp_int);

    if(wph_scr<0 || wph_scr > 360)
        this->wph_scr=-1;
    else
        this->wph_scr=wph;
    updateHasChanged(true);
    updateText(true);
}

void DialogPilototoInstruction::copyPOI(void)
{
    if(mode_scr==2 || mode_scr==3)
        Util::setWPClipboard(lat_scr,lon_scr,wph_scr);
}

void DialogPilototoInstruction::doSelectPOI(void)
{
    emit selectPOI(this,1);
}

void DialogPilototoInstruction::dateTime_changed(QDateTime tm)
{
    if(tm!=tstamp_scr)
    {
        tstamp_scr=tm;
        updateHasChanged(chkHasChanged());
        updateText(true);
        if(status_scr!=PILOTOTO_STATUS_DONE && tm.toTime_t() < QDateTime::currentDateTime().toTime_t())
        {
            QPalette p = horodate->palette();
            p.setColor( QPalette::Text, QColor(Qt::red) );
            horodate->setPalette(p);
        }
        else
        {
            QPalette p = horodate->palette();
            p.setColor( QPalette::Text, QColor(Qt::black) );
            horodate->setPalette(p);
        }
    }
}

void DialogPilototoInstruction::setLock(bool status)
{
    locked=status;
    btn_delInstruction->setEnabled(!status);
    horodate->setEnabled(!status);
    btn_paste->setEnabled(!status);
    btn_edit->setEnabled(!status);
    if(status)
	btn_validate->setEnabled(!status);
    else
	updateHasChanged(hasChanged);
}

void DialogPilototoInstruction::validateModif(void)
{
    updateHasChanged(false);
    /* set data to scr one */
    mode=mode_scr;
    angle=angle_scr;
    lat=lat_scr;
    lon=lon_scr;
    wph=wph_scr;
    status=status_scr;
    tstamp=tstamp_scr;
    /* code to update instruction order */
    emit instructionUpdated();
}

void DialogPilototoInstruction::cancelModif(void)
{

    /* set src data to org */
    mode_scr=mode;
    angle_scr=angle;
    lat_scr=lat;
    lon_scr=lon;
    wph_scr=wph;
    status_scr=status;
    tstamp_scr=tstamp;

    updateHasChanged(false);
    updateText(true);
}

void DialogPilototoInstruction::setStatus(int val)
{
    status=status_scr=val;
}

void DialogPilototoInstruction::setTstamp(int val)
{
    tstamp.setTime_t(val);
    tstamp_scr.setTime_t(val);
    //updateHasChanged(true);
    updateText(true);
}

QString DialogPilototoInstruction::getPip(void)
{
    QString txt;

    /* using real data */
    switch(mode)
    {
	case 0:
	case 1:
	    txt=QString().setNum(angle);
	    break;
	case 2:
	case 3:
        case 4:
	    if(wph==-1)
		txt=QString("%1,%2")
			.arg(lat)
			.arg(lon);
	    else
		txt=QString("%1,%2@%3")
			.arg(lat)
			.arg(lon)
			.arg(wph);
	    break;
    }
    return txt;
}

void DialogPilototoInstruction::maintenant(void)
{
    tstamp_scr = QDateTime::currentDateTime().toUTC();
    horodate->setDateTime(tstamp_scr);
}

void DialogPilototoInstruction::pickPipColor(void)
{

    switch(mode_scr)
    {
        case 0: //Heading
            pipColor=QColor(255,255,255,255);
            break;
        case 1: //TWA
            pipColor=QColor(247,246,210,255);
            break;
        case 2: //Ortho
            pipColor=QColor(247,210,237,255);
            break;
        case 3: //BVmg
            pipColor=QColor(210,221,247,255);
            break;
        case 4: //VBVmg
            pipColor=QColor(210,247,210,255);
            break;
    }
    pipPalette.setColor(QPalette::Base, pipColor);
    instructionText->setPalette(pipPalette);
}

