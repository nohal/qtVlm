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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <cmath>
#include <QMessageBox>
#include <QDebug>

#include "DialogRoutage.h"
#include "Util.h"
#include "POI.h"
#include "MainWindow.h"
#include "DialogGraphicsParams.h"
#include "mycentralwidget.h"
#include "routage.h"
#include "boatVLM.h"
#include "Player.h"
#include <QThread>
#include <QDesktopWidget>
#include "settings.h"


//-------------------------------------------------------
// ROUTAGE_Editor: Constructor for edit an existing ROUTAGE
//-------------------------------------------------------
DialogRoutage::DialogRoutage(ROUTAGE *routage,myCentralWidget *parent)
    : QDialog(parent)
{
    QString m;
    this->routage=routage;
    this->parent=parent;
    setupUi(this);
    Util::setFontDialog(this);
    this->resize(widget->width()+10,widget->height()+10);
    widget->setParent(0);
    scroll=new QScrollArea(this);
    scroll->resize(widget->size());
    scroll->setWidget(widget);
    QSize mySize=QSize(widget->size().width()+20,widget->size().height()+20);
    QSize screenSize=QApplication::desktop()->screenGeometry().size()*.8;
    if(mySize.height() > screenSize.height())
    {
        mySize.setHeight(screenSize.height());
    }
    if(mySize.width() > screenSize.width())
    {
        mySize.setWidth(screenSize.width());
    }
    this->resize(mySize);
    scroll->resize(mySize);
    m.sprintf("%d",QThread::idealThreadCount());
    m=tr("Calculer en parallele (")+m+tr(" processeurs disponibles)");
    multi->setText(m);
    if(QThread::idealThreadCount()<=1)
    {
        routage->setUseMultiThreading(false);
        multi->setEnabled(false);
    }
    multi->setChecked(routage->getUseMultiThreading());
    editName->setFocus();
    inputTraceColor =new InputLineParams(routage->getWidth(),routage->getColor(),1.6,  QColor(Qt::red),this,0.1,5);
    verticalLayout_2->addWidget( inputTraceColor);
    setWindowTitle(tr("Parametres Routage"));
    editName->setText(routage->getName());
    editDateBox->setDateTime(routage->getStartTime());
    editDateBox->setEnabled(true);
    whatIfDate->setDateTime(routage->getWhatIfDate());
    whatIfUse->setChecked(routage->getWhatIfUsed());
    whatIfWind->setValue(routage->getWhatIfWind());
    whatIfTime->setValue(routage->getWhatIfTime());
    autoZoom->setChecked(routage->getAutoZoom());
    visibleOnly->setChecked(routage->getVisibleOnly());
    this->poiPrefix->setText(routage->getPoiPrefix());
    this->startFromBoat->setChecked(routage->getRouteFromBoat());
    this->maxPortant->setValue(routage->getMaxPortant());
    this->maxPres->setValue(routage->getMaxPres());
    this->minPortant->setValue(routage->getMinPortant());
    this->minPres->setValue(routage->getMinPres());
    if(routage->getFinalEta().isNull())
        this->groupBox_eta->setHidden(true);
    else
    {
        this->groupBox_eta->setHidden(false);
        this->editDateBox_2->setDateTime(routage->getFinalEta());
    }
    int n=0;
    if(parent->getPlayer()->getType()!=BOAT_REAL)
    {
        this->speedLossOnTack->setValue(Settings::getSetting("speedLossOnTackVlm","100").toInt());
        if(parent->getBoats())
        {
            QListIterator<boatVLM*> i (*parent->getBoats());
            while(i.hasNext())
            {
                boatVLM * acc = i.next();
                if(acc->getStatus())
                {
                    if(acc->getAliasState())
                        editBoat->addItem(acc->getAlias() + "(" + acc->getBoatPseudo() + ")");
                    else
                        editBoat->addItem(acc->getBoatPseudo());
                    if(acc==routage->getBoat()) editBoat->setCurrentIndex(n);
                    n++;
                }
            }
        }
    }
    else
    {
        this->speedLossOnTack->setValue(Settings::getSetting("speedLossOnTackReal","100").toInt());
        editBoat->addItem(parent->getPlayer()->getName());
        editBoat->setEnabled(false);
    }
    n=0;
    for(int m=0;m<parent->getPois().count();m++)
    {
        POI * poi = parent->getPois().at(m);
        if(poi->getRoute()==NULL)
        {
            fromPOI->addItem(poi->getName(),m);
            if(poi==routage->getFromPOI()) fromPOI->setCurrentIndex(n);

            toPOI->addItem(poi->getName(),m);
            if(poi==routage->getToPOI()) toPOI->setCurrentIndex(n);
            n++;
        }
    }
    this->range->setValue(routage->getAngleRange());
    this->step->setValue(routage->getAngleStep());
    this->dureeLess24->setValue(routage->getTimeStepLess24());
    this->dureeMore24->setValue(routage->getTimeStepMore24());
    this->windForced->setChecked(routage->getWindIsForced());
    this->showIso->setChecked(routage->getShowIso());
    this->explo->setValue(routage->getExplo());
    this->useVac->setChecked(routage->getUseRouteModule());
    this->log->setChecked(routage->useConverge);
    this->pruneWakeAngle->setValue(routage->pruneWakeAngle);
    if(routage->getWindIsForced())
    {
        this->TWD->setValue(routage->getWindAngle());
        this->TWS->setValue(routage->getWindSpeed());
    }
    this->checkCoast->setChecked(routage->getCheckCoast());
    if(routage->isDone() || routage->getIsNewPivot())
    {
        this->speedLossOnTack->setValue(qRound(routage->getSpeedLossOnTack()*100));
        this->speedLossOnTack->setDisabled(true);
        this->editName->setDisabled(false);
        this->autoZoom->setDisabled(true);
        this->visibleOnly->setDisabled(true);
        this->editBoat->setDisabled(true);
        this->editDateBox->setDisabled(true);
        this->fromPOI->setDisabled(true);
        this->toPOI->setDisabled(true);
        this->dureeMore24->setDisabled(true);
        this->dureeLess24->setDisabled(true);
        this->range->setDisabled(true);
        this->step->setDisabled(true);
        this->explo->setDisabled(true);
        this->windForced->setDisabled(true);
        this->TWD->setDisabled(true);
        this->TWS->setDisabled(true);
        this->useVac->setDisabled(true);
        this->log->setDisabled(true);
        this->pruneWakeAngle->setDisabled(true);
        this->checkCoast->setDisabled(true);
        if(routage->isConverted())
            this->convRoute->setDisabled(true);
        this->startFromBoat->setDisabled(true);
        this->whatIfUse->setDisabled(true);
        this->whatIfDate->setDisabled(true);
        this->whatIfWind->setDisabled(true);
        this->whatIfTime->setDisabled(true);
        this->multi->setDisabled(true);
        this->maxPortant->setDisabled(true);
        this->maxPres->setDisabled(true);
        this->minPortant->setDisabled(true);
        this->minPres->setDisabled(true);
    }
    if(routage->getIsNewPivot() && !routage->isDone())
    {
        this->speedLossOnTack->setDisabled(false);
        this->autoZoom->setDisabled(false);
        this->visibleOnly->setDisabled(false);
        this->dureeMore24->setDisabled(false);
        this->dureeLess24->setDisabled(false);
        this->range->setDisabled(false);
        this->step->setDisabled(false);
        this->explo->setDisabled(false);
        this->toPOI->setDisabled(false);
        this->log->setDisabled(false);
        this->pruneWakeAngle->setDisabled(false);
        this->checkCoast->setDisabled(false);
        this->whatIfUse->setDisabled(false);
        this->whatIfDate->setDisabled(false);
        this->whatIfWind->setDisabled(false);
        this->whatIfTime->setDisabled(false);
        this->multi->setDisabled(false);
        this->maxPres->setDisabled(false);
        this->maxPortant->setDisabled(false);
        this->minPres->setDisabled(false);
        this->minPortant->setDisabled(false);
    }
}
DialogRoutage::~DialogRoutage()
{
}

void DialogRoutage::resizeEvent ( QResizeEvent * /*event*/ )
{
    this->scroll->resize(this->size());
}
//---------------------------------------
void DialogRoutage::done(int result)
{
    if(result == QDialog::Accepted)
    {
        if (!parent->freeRoutageName((editName->text()).trimmed(),routage))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Ce nom est deja utilise, choisissez en un autre"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        routage->setUseMultiThreading(this->multi->isChecked());
        routage->setPoiPrefix(this->poiPrefix->text());
        routage->setName((editName->text()).trimmed());
        routage->setWidth(inputTraceColor->getLineWidth());
        routage->setColor(inputTraceColor->getLineColor());
        routage->setStartTime(editDateBox->dateTime());
        routage->setCheckCoast(checkCoast->isChecked());
        routage->useConverge=log->isChecked();
        routage->pruneWakeAngle=pruneWakeAngle->value();
        routage->setAutoZoom(autoZoom->isChecked());
        routage->setVisibleOnly(visibleOnly->isChecked());
        routage->setRouteFromBoat(this->startFromBoat->isChecked());
        routage->setSpeedLossOnTack((double)this->speedLossOnTack->value()/100.00);
        routage->setMaxPortant(this->maxPortant->value());
        routage->setMaxPres(this->maxPres->value());
        routage->setMinPortant(this->minPortant->value());
        routage->setMinPres(this->minPres->value());
        if(parent->getPlayer()->getType()!=BOAT_REAL)
        {
            if(parent->getBoats())
            {
                QListIterator<boatVLM*> i (*parent->getBoats());
                while(i.hasNext())
                {
                    boatVLM * acc = i.next();
                    if(acc->getBoatPseudo()==editBoat->currentText())
                    {
                        routage->setBoat(acc);
                        break;
                    }
                }
                if(this->useVac->isChecked())
                {
                    if(this->dureeLess24->value()<4*routage->getBoat()->getVacLen()/60)
                    {
                        QMessageBox::critical(0,tr("Creation d'un routage"),
                                              tr("Vous ne pouvez pas utiliser le module route<br>avec moins de 4 vacations entre les isochrones.<br>Vous devez donc desactiver cette option ou rallonger la duree"));
                        return;
                    }
                    if(this->dureeMore24->value()<4*routage->getBoat()->getVacLen()/60)
                    {
                        QMessageBox::critical(0,tr("Creation d'un routage"),
                                              tr("Vous ne pouvez pas utiliser le module route<br>avec moins de 4 vacations entre les isochrones.<br>Vous devez donc desactiver cette option ou rallonger la duree"));
                        return;
                    }
                }
            }
        }
        else
            routage->setBoat((boat *) parent->getRealBoat());
        routage->setFromPOI(NULL);
        routage->setToPOI(NULL);
        if(toPOI->currentIndex()==-1)
        {
            QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de destination est invalide")));
            return;
        }
        else
        {
            routage->setToPOI(parent->getPois().at(toPOI->itemData(toPOI->currentIndex(),Qt::UserRole).toInt()));
        }
        if(!startFromBoat->isChecked() && fromPOI->currentIndex()==-1)
        {
            QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de depart est invalide")));
            return;
        }
        else
        {
            routage->setFromPOI(parent->getPois().at(fromPOI->itemData(fromPOI->currentIndex(),Qt::UserRole).toInt()));
        }
        if(routage->getToPOI()==NULL)
        {
            QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de destination est invalide")));
            return;
        }
        if(!routage->getRouteFromBoat())
        {
            if(routage->getFromPOI()==NULL)
            {
                QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de depart est invalide")));
                return;
            }
            if(routage->getFromPOI()->getLongitude()==routage->getToPOI()->getLongitude() &&
               routage->getFromPOI()->getLatitude()==routage->getToPOI()->getLatitude())
            {
                if(!routage->getIsNewPivot() && !routage->getIsPivot())
                {
                    QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de depart et d'arrivee sont les memes, vous etes deja arrive...")));
                    return;
                }
            }
        }
        routage->setWhatIfUsed(whatIfUse->isChecked());
        routage->setWhatIfDate(whatIfDate->dateTime());
        routage->setWhatIfWind(whatIfWind->value());
        routage->setWhatIfTime(whatIfTime->value());
        routage->setWindIsForced(windForced->isChecked());
        routage->setWind(TWD->value(),TWS->value());
        routage->setAngleRange(this->range->value());
        routage->setAngleStep(this->step->value());
        routage->setTimeStepMore24(this->dureeMore24->value());
        routage->setTimeStepLess24(this->dureeLess24->value());
        routage->setExplo(this->explo->value());
        routage->setShowIso(this->showIso->isChecked());
        routage->setUseRouteModule(this->useVac->isChecked());
        if(this->convRoute->isChecked())
        {
            if(!routage->isConverted())
            {
                if(routage->isDone())
                {
                    if (!parent->freeRouteName(tr("Routage: ")+(editName->text()).trimmed(),NULL))
                    {
                        QMessageBox msgBox;
                        msgBox.setText(tr("Ce nom de route est deja utilise, veuillez changer le nom du routage"));
                        msgBox.setIcon(QMessageBox::Critical);
                        msgBox.exec();
                        return;
                    }
                    int rep=QMessageBox::Yes;
                    if(routage->getWhatIfUsed() && (routage->getWhatIfTime()!=0 || routage->getWhatIfWind()!=100))
                    {
                        rep = QMessageBox::question (0,
                                tr("Convertir en route"),
                                tr("Ce routage a ete calcule avec une hypothese modifiant les donnees du grib<br>La route ne prendra pas ce scenario en compte<br>Etes vous sur de vouloir le convertir en route?"),
                                QMessageBox::Yes | QMessageBox::No);
                    }
                    if(rep==QMessageBox::Yes)
                    {
                        routage->convertToRoute();
                    }
                }
                else
                    routage->setConverted();
            }
        }
    }
    if(result == QDialog::Rejected)
    {
    }
    QDialog::done(result);
}

//---------------------------------------

void DialogRoutage::on_windForced_toggled(bool checked)
{
   TWD->setEnabled(checked);
   TWS->setEnabled(checked);
}
void DialogRoutage::GybeTack(int i)
{
    QFont font=this->labelTackGybe->font();
    if(i==100)
        font.setBold(false);
    else
        font.setBold(true);
    this->labelTackGybe->setFont(font);
}

