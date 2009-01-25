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

#include "Util.h"
#include "Pilototo.h"

#define VLM_NO_REQUEST     -1
#define VLM_REQUEST_LOGIN  0
#define VLM_DO_REQUEST     1

Pilototo::Pilototo(QWidget * parent):QDialog(parent)
{
    setupUi(this);
    this->parent=parent;

    instructionEditor = new Pilototo_param(this);

    instructions_list.clear();

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    frameLayout = new QVBoxLayout(frame);
    frameLayout->setSizeConstraint(QLayout::SetFixedSize);
    
    waitBox = new QMessageBox(QMessageBox::Question,
                             tr("Pilototo"),
                             tr("Chargement des instructions VLM en cours"),
                             QMessageBox::Cancel,this
                             );

    /* inet init */
    inetManager = new QNetworkAccessManager(this);
    if(inetManager)
    {
        host = Util::getHost();
        connect(inetManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished (QNetworkReply*)));
        Util::paramProxy(inetManager,host);
    }

    currentRequest = VLM_NO_REQUEST;
    currentList = NULL;
}

void Pilototo::updateDrawList(void)
{
    bool added;
    /* remove all instruction from screen */
    for(int i=0;i<drawList.count();i++)
    {
        drawList[i]->hide();
        frameLayout->removeWidget(drawList[i]);
    }
    
    drawList.clear();
    
    for(int i=0;i<instructions_list.count();i++)
    {
        Pilototo_instruction * instr=instructions_list[i];
        added = false;
        if(instr->getHasChanged())
        { /* date have not been validated */
            showMessage(QString().setNum(i) + " not validated");
            /* search for the first not validated item */
            bool found = false;
            int j;
            for(j=0;j<drawList.count();j++)
            {
                if(drawList[j]->getHasChanged())
                {
                    found=true;
                    break;
                }
            }
            if(!found) /* not added yet => append */
            {
                showMessage(QString().setNum(i) + " appended as no other unvalidated item");
                drawList.append(instr);
            }
            else
            {
                showMessage(QString().setNum(i) + " searching where to insert in unvalidated item list");
                /* order not validated items by tstamp */
                for(/*not changing j*/;j<drawList.count();j++)
                {
                    if(drawList[j]->getTstamp() > instr->getTstamp())
                    {
                        drawList.insert(j,instr);
                        showMessage(QString().setNum(i) + " adding at "+QString().setNum(j));
                        added=true;
                        break;
                    }
                }
                if(!added) /* not added yet => append */
                {
                    showMessage(QString().setNum(i) + " appended as end of unvalidated list reached");
                    drawList.append(instr);
                }
            }
        }
        else
        { /* date have been validated */
            showMessage(QString().setNum(i) + " validated");
            for(int j=0;j<drawList.count();j++)
            {
                if(drawList[j]->getHasChanged())
                {
                    /* j item is not validated => add just before */
                    drawList.insert(j,instr);
                    showMessage(QString().setNum(i) + " (end of validated reached) adding at "+QString().setNum(j));
                    added=true;
                    break;
                }

                if(drawList[j]->getTstamp() > instr->getTstamp())
                {
                    /* j item is later than i */
                    drawList.insert(j,instr);
                    showMessage(QString().setNum(i) + " adding at "+QString().setNum(j));
                    added=true;
                    break;
                }
            }
            if(!added)
            {
                showMessage(QString().setNum(i) + " appended as end of list reached");
                drawList.append(instr);
            }
        }

        showMessage("Next instruction");
    }
    showMessage("Finish");

    /* we now have the list of items to be shown => adding them to grid layout */
   
    for(int j=0;j<drawList.count();j++)
    {
        frameLayout->addWidget(drawList[j], j, 0);
        drawList[j]->show();
    }
    /*frameLayout->update();
    layout()->update();
    update();*/
}

/* init pilototo editor */
void Pilototo::editInstructions(void)
{
    waitBox->exec();
}
        
void Pilototo::boatUpdated(boatAccount * boat)
{
    if(!waitBox->isVisible())
        return;
    waitBox->hide();
    this->boat=boat;
    int mode;
    int pos;

    QStringList * list = boat->getPilototo();
    delList.clear();
    drawList.clear();
    instructions_list.clear();

    if(!boat->getHasPilototo())
        QMessageBox::information (this,
            tr("Pilototo"),
            tr("La récupération des données pilototo de VLM n'a pas fonctionné\nVous pouvez ajouter des instructions mais sans voir le resultat dans QtVlm"),
            QMessageBox::Ok);
    
    for(int i=0;i<list->count();i++)
    {
        QString instr_txt=list->at(i);
        showMessage("Processing : "+instr_txt);
        if(instr_txt!="none")
        {
            QStringList instr_buf = instr_txt.split(",");

            if(instr_buf.count() == 6 || instr_buf.count() == 5)
            {
                Pilototo_instruction * instr = new Pilototo_instruction(this,frame);
                instructions_list.append(instr);
                instr->setRef(instr_buf.at(0).toInt());
                instr->setTstamp(instr_buf.at(1).toInt());
                mode=instr_buf.at(2).toInt()-1;
                instr->setMode(mode);
                showMessage("Mode : "+QString().setNum(mode));
                if(mode == 0 || mode == 1)
                {
                    instr->setAngle(instr_buf.at(3).toFloat());
                    pos=4;
                }
                else
                {
                    instr->setLat(instr_buf.at(3).toFloat());
                    QStringList instr_buf2 = instr_buf.at(4).split("@");
                    if(instr_buf2.count() == 2)
                    {
                        instr->setLon(instr_buf2.at(0).toFloat());
                        instr->setWph(instr_buf2.at(1).toFloat());
                    }
                    else
                    {   
                        instr->setLon(instr_buf.at(4).toFloat());
                    }
                    pos=5;
                }
                if(instr_buf.at(pos) == "done")
                    instr->setStatus(PILOTOTO_STATUS_DONE);
                else if(instr_buf.at(pos) == "pending")
                    instr->setStatus(PILOTOTO_STATUS_PENDING);
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

void Pilototo::done(int result)
{
    if(result==QDialog::Accepted)
    {
        /* checking if there is un validated instructions */
        bool hasUnValidated =false;
        for(int i=0;i<instructions_list.count();i++)
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
            tr("Instructions non Validées"),
            tr("Il reste des instructions non validées. Elles ne seront pas envoyées à VLM\nContinuer la sauvegarde?"),
            QMessageBox::Yes | QMessageBox::No);
            if (rep == QMessageBox::No)
                return;
        }
        /* creating list of pilototo.php requests*/
        QStringList * requestList= new QStringList();
        /* processing del */
        for(int i=0;i<delList.count();i++)
            requestList->append("action=efface&lang=fr&taskid="+QString().setNum(delList[i]));
        /* processing others */
        for(int i=0;i<instructions_list.count();i++)
        {
            Pilototo_instruction * instr=instructions_list[i];
            if(!instr->getHasChanged())
            { /* only processing activated and validated instructions */
                QString txt;
                if(instr->getRef()!=-1) /* updating */
                    txt="action=modifie&lang=fr&taskid="+QString().setNum(instr->getRef());
                else /* adding */
                    txt="action=ajoute&lang=fr";
                txt=txt+QString("&pim=%1&pip=%2&time=%3").arg(instr->getMode()+1)
                        .arg(instr->getPip()).arg(instr->getTstamp());
                requestList->append(txt);
            }
        }
        /* ready to send */
        sendPilototo(requestList);
    }
    
    for(int i=0;i<instructions_list.count();i++)
        delete instructions_list[i];

    instructions_list.clear();

    QDialog::done(result);
}

void Pilototo::sendPilototo(QStringList * cmdList)
{
    if(inetManager && currentRequest == VLM_NO_REQUEST && cmdList->count() > 0)
    {
        currentRequest=VLM_REQUEST_LOGIN;
        currentList=cmdList;
        QString page;
        QTextStream(&page) << host
                        << "/myboat.php?"
                        << "pseudo=" << boat->getLogin()
                        << "&password=" << boat->getPass()
                        << "&lang=fr&type=login"
                        ;

        emit showMessage(QString("login for cmd pilototo param: %2 instructions").arg(currentList->count()));

        inetManager->get(QNetworkRequest(QUrl(page)));
    }
    else
    {
        emit showMessage(QString("error can't send pilototo manager=%1 current state:%2 nb instr:%3").arg((int)inetManager)
                                        .arg(currentRequest).arg(cmdList->count()));
    }
}

void Pilototo::requestFinished ( QNetworkReply* inetReply)
{
    QString page;
    QString data;
    if (inetReply->error() != QNetworkReply::NoError) {
        emit showMessage("Error doing inetGet:" + QString().setNum(inetReply->error()));
        currentRequest=VLM_NO_REQUEST;
    }
    else
    {
        switch(currentRequest)
        {
            case VLM_NO_REQUEST:
                return;
            case VLM_REQUEST_LOGIN:
                emit showMessage("Login done");
            case VLM_DO_REQUEST:
                currentRequest=VLM_DO_REQUEST;
                if(currentList->isEmpty())
                {
                    /*we have processed all cmd*/
                    currentRequest=VLM_NO_REQUEST;
                    delete currentList;
                    emit showMessage("All instructions send");
                    /* ask for an update of boat data*/
                    boat->getData();
                }
                else
                {
                    QTextStream(&page) << host
                            << "/pilototo.php";
                    data = currentList->takeFirst();

                    emit showMessage("Request: " + page + "data: " + data);
                    inetManager->post(QNetworkRequest(QUrl(page)),data.toAscii());
                }
                break;
        }
    }
}

void Pilototo::updateProxy(void)
{
    /* update connection */
    Util::paramProxy(inetManager,host);
}

void Pilototo::addInstruction(void)
{
    if(nbInstruction<=5)
    {
        Pilototo_instruction * instr = new Pilototo_instruction(this,frame);
        instructions_list.append(instr);
        instr->initVal();
        updateDrawList();
        updateNbInstruction();
    }
}

void Pilototo::updateTime(void)
{
    QFontMetrics fmt(curTime->font());
    QDateTime tm = QDateTime::currentDateTime().toUTC();
    QString data=tm.toString("(ddd) dd/MM/yyyy hh:mm:ss");
    curTime->setMinimumWidth(fmt.width(data)+10);
    curTime->setText(data);
}

void Pilototo::delInstruction(Pilototo_instruction * instruction)
{
    int ref=instruction->getRef();
    if(ref!=-1)
        delList.append(ref);
    instructions_list.removeAll(instruction);
    
    updateDrawList();
    updateNbInstruction();

    instruction->deleteLater();
}

void Pilototo::updateNbInstruction(void)
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

void Pilototo::instructionUpdated(void)
{
    updateDrawList();
}

/******************************
* Pilototo instruction
* widget + data structure
******************************/

Pilototo_instruction::Pilototo_instruction(QWidget * main,QWidget * parent) : QWidget(parent)
{
    setupUi(this);
    connect(this,SIGNAL(doEditInstruction(Pilototo_instruction*)),
                ((Pilototo*)main)->instructionEditor,SLOT(editInstruction(Pilototo_instruction*)));
    connect(this,SIGNAL(doDelInstruction(Pilototo_instruction*)),
                main,SLOT(delInstruction(Pilototo_instruction *)));
    connect(this,SIGNAL(instructionUpdated()),
                main,SLOT(instructionUpdated()));
    this->parent=parent;
    
    /*QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);*/
    layout()->setContentsMargins(0,0,0,0);
    
    updateHasChanged(true); /*instruction is not saved when created*/
    initVal();
}

void Pilototo_instruction::initVal(void)
{
    mode=0;
    angle=0;
    lat=0;
    lon=0;
    wph=-1;
    ref=-1;
    status=PILOTOTO_STATUS_NEW;
    updateHasChanged(true);
    tstamp = QDateTime::currentDateTime().toUTC();
    horodate->setTimeSpec(Qt::UTC);
    horodate->setDateTime(tstamp);
    updateText();
    hide();
}

#define SETVAL(VAR)       \
{                         \
    if(VAR!=val)          \
    {                     \
        VAR=val;          \
        updateHasChanged(true); \
        updateText();     \
    }                     \
}

void Pilototo_instruction::setMode(int val)
{
    SETVAL(mode);
}

void Pilototo_instruction::setAngle(float val)
{
    SETVAL(angle);
}

void Pilototo_instruction::setLat(float val)
{
    SETVAL(lat);
}

void Pilototo_instruction::setLon(float val)
{
    SETVAL(lon);
}

void Pilototo_instruction::setWph(float val)
{
    SETVAL(wph);
}

void Pilototo_instruction::updateText(void)
{
    QFontMetrics fmt(instructionText->font());
    QString final_txt;
    QString modeTxt,param;
    
    switch(mode)
    {
        case 0:
            modeTxt="Cap";
            param=QString().setNum(angle)+" °";
            break;
        case 1:
            modeTxt="Angle";
            param=QString().setNum(angle)+" °";
            break;
        case 2:
            modeTxt="Ortho";
            if(wph==-1)
                param=QString("%1,%2")
                        .arg(Util::pos2String(TYPE_LAT,lat))
                        .arg(Util::pos2String(TYPE_LON,lon));
            else
                param=QString("%1,%2@%3")
                        .arg(Util::pos2String(TYPE_LAT,lat))
                        .arg(Util::pos2String(TYPE_LON,lon))
                        .arg(wph);
            break;
        case 3:
            modeTxt="BVMG";
            if(wph==-1)
                param=QString("%1,%2")
                        .arg(Util::pos2String(TYPE_LAT,lat))
                        .arg(Util::pos2String(TYPE_LON,lon));
            else
                param=QString("%1,%2@%3")
                        .arg(Util::pos2String(TYPE_LAT,lat))
                        .arg(Util::pos2String(TYPE_LON,lon))
                        .arg(wph);
            break;
    }
    final_txt=modeTxt+" = "+param;
    instructionText->setMinimumWidth( fmt.width(final_txt)+20 );
    instructionText->setText(final_txt);

    switch(status)
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
    }
    
    horodate->setDateTime(tstamp);
}

void Pilototo_instruction::updateHasChanged(bool status)
{
    hasChanged = status;
    btn_validate->setEnabled(status);
    if(status)
        btn_validate->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    else
        btn_validate->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
}

void Pilototo_instruction::delInstruction(void)
{
    emit doDelInstruction(this);
}

void Pilototo_instruction::editInstruction(void)
{
    emit doEditInstruction(this);
}

void Pilototo_instruction::pastePOI(void)
{
    float lat,lon,wph;
    int tstamp_int;
    if(!Util::getWPClipboard(&lat,&lon,&wph,&tstamp_int))
        return;
    
    this->lat = lat;
    this->lon = lon;
    mode=2; /*default mode : ortho */
    
    if(tstamp_int!=-1)
        tstamp.setTime_t(tstamp_int);

    if(wph<0 || wph > 360)
        this->wph=-1;
    else
        this->wph=wph;
    updateHasChanged(true);
    updateText();
}

void Pilototo_instruction::dateTime_changed(QDateTime tm)
{
    if(tm!=tstamp)
        updateHasChanged(true);
    tstamp=tm;
}

void Pilototo_instruction::setLock(bool status)
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

void Pilototo_instruction::validateModif(void)
{
    updateHasChanged(false);
    /* code to update instruction order */
    emit instructionUpdated();
}

void Pilototo_instruction::setStatus(int val)
{
    SETVAL(status);
}

void Pilototo_instruction::setTstamp(int val)
{
    tstamp.setTime_t(val);
    updateHasChanged(true);
    updateText();
}

QString Pilototo_instruction::getPip(void)
{
    QString txt;
    
    switch(mode)
    {
        case 0:
        case 1:
            txt=QString().setNum(angle);
            break;
        case 2:
        case 3:
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

void Pilototo_instruction::maintenant(void)
{
    tstamp = QDateTime::currentDateTime().toUTC();
    horodate->setDateTime(tstamp);
}
