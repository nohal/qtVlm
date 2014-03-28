/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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



#include "mycentralwidget.h"
#include "routage.h"
#include "POI.h"
#include "Player.h"
#include "boat.h"
#include "boatVLM.h"
#include "boatReal.h"
#include "settings.h"
#include "Terrain.h"

#include "DialogRoutage_ctrl.h"
#include "DialogRoutage_view.h"

#include "DialogRoutage_view_pc.h"
#include "DialogRoutage_view_mobile.h"

DialogRoutage_ctrl::DialogRoutage_ctrl(myCentralWidget *centralWidget,ROUTAGE * routage,bool createMode) {
    //view = new DialogRoutage_view_pc(centralWidget,this);
    view = new DialogRoutage_view_mobile(centralWidget,this);
    this->centralWidget=centralWidget;
    this->routage=routage;
    this->createMode=createMode;

    routageData=NULL;
    build_routageData();
    view->initDialogState(routageData);
}

DialogRoutage_ctrl::~DialogRoutage_ctrl(void) {
    if(view)
        delete view;
}

void DialogRoutage_ctrl::build_routageData(void) {
    if(routageData) delete routageData;
    routageData=new RoutageData();

    routageData->name=routage->getName();

    routageData->poiPrefix=routage->getPoiPrefix();

    routageData->isDone=routage->isDone();
    routageData->I_done=routage->getI_done();
    routageData->isNewPivot=routage->getIsNewPivot();
    routageData->isConverted=routage->isConverted();
    routageData->arrived=routage->getArrived();
    routageData->useMultiThreading=routage->getUseMultiThreading();
    routageData->showIso=routage->getShowIso();
    routageData->useRouteModule=routage->getUseRouteModule();
    routageData->useConverge=routage->useConverge;
    routageData->colorGrib=routage->getColorGrib();
    routageData->routageOrtho=routage->getRoutageOrtho();
    routageData->showBestLive=routage->getShowBestLive();
    routageData->checkCoast=routage->getCheckCoast();
    routageData->checkLine=routage->getCheckLine();

    routageData->startTime=routage->getStartTime();
    routageData->finalETA=routage->getFinalEta();
    routageData->timeStepLess24=routage->getTimeStepLess24();
    routageData->timeStepMore24=routage->getTimeStepMore24();

    routageData->explo=routage->getExplo();

    routageData->isoRouteValue=routage->getIsoRouteValue();
    routageData->visibleOnly=routage->getVisibleOnly();
    routageData->routeFromBoat=routage->getRouteFromBoat();

    routageData->whatIfDate=routage->getWhatIfDate();
    routageData->whatIfUsed=routage->getWhatIfUsed();
    routageData->whatIfWind=routage->getWhatIfWind();
    routageData->whatIfTime=routage->getWhatIfTime();

    routageData->maxPortant=routage->getMaxPortant();
    routageData->maxPres=routage->getMaxPres();
    routageData->minPortant=routage->getMinPortant();
    routageData->minPres=routage->getMinPres();
    routageData->maxWaveHeight=routage->get_maxWaveHeight();

    routageData->zoomLevel=routage->getZoomLevel();
    routageData->autoZoom=routage->getAutoZoom();

    routageData->lineWidth=routage->getWidth();
    routageData->lineColor=routage->getColor();

    routageData->boatType=centralWidget->getPlayer()->getType();
    if(routageData->boatType!=BOAT_REAL) {
        if(centralWidget->getBoats()) {
            QListIterator<boatVLM*> i (*centralWidget->getBoats());
            while(i.hasNext()) {
                boatVLM * acc = i.next();
                if(acc->getStatus()) {
                    if(acc->getAliasState())
                        routageData->boatList.append(acc->getAlias() + "(" + acc->getBoatPseudo() + ")");
                    else
                        routageData->boatList.append(acc->getBoatPseudo());
                    routageData->boatPtrList.append((boat*)acc);
                }
            }
            routageData->curentBoat=(boat*)routage->getBoat();
        }
    }
    else {
        routageData->boatList.append(centralWidget->getPlayer()->getName());
        routageData->curentBoat=NULL;
    }

    if(routage->isDone()) {
        /*if(routage->getFromPOI()) {
            routageData->poiList.append(routage->getFromPOI()->getName());
            routageData->poiPtrList.append(routage->getFromPOI());
        }
        if(routage->getToPOI() && routageData->poiPtrList.at(0)!=routage->getToPOI()) {
                routageData->poiList.append(routage->getToPOI()->getName());
                routageData->poiPtrList.append(routage->getToPOI());
        }*/
    }
    else {
        for(int m=0;m<centralWidget->getPois().count();m++) {
            POI * poi = centralWidget->getPois().at(m);
            if(poi->getType()!= POI_TYPE_BALISE && poi->getRoute()==NULL) {
                routageData->poiList.append(poi->getName());
                routageData->poiPtrList.append(poi);
            }
        }
    }

    routageData->fromPoi=routage->getFromPOI();
    routageData->toPoi=routage->getToPOI();

    routageData->angleRange=routage->getAngleRange();
    routageData->angleStep=routage->getAngleStep();
    routageData->pruneWakeAngle=routage->pruneWakeAngle;
    routageData->speedLossOnTack=routage->getSpeedLossOnTack();

    routageData->nbAlternative=routage->getNbAlternative();
    routageData->thresholdAlternative=routage->getThresholdAlternative();

    routageData->multiRoutage=routage->get_multiRoutage();
    routageData->multiNb=routage->get_multiNb();
    routageData->multiDays=routage->get_multiDays();
    routageData->multiHours=routage->get_multiHours();
    routageData->multiMin=routage->get_multiMin();
}

void DialogRoutage_ctrl::validateChange(void) {
    if (!centralWidget->freeRoutageName(routageData->name.trimmed(),routage))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Ce nom est deja utilise, choisissez en un autre"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    if(centralWidget->getPlayer()->getType()!=BOAT_REAL) {
        if(centralWidget->getBoats()) {
            if(!routageData->curentBoat) {
                QMessageBox::critical(0,tr("Creation d'un routage"),
                                      tr("Pas de bateau selectionne, pas de routage possible"));
                return;
            }

            if(routageData->useRouteModule) {
                if(routageData->timeStepLess24<4*routage->getBoat()->getVacLen()/60) {
                    QMessageBox::critical(0,tr("Creation d'un routage"),
                                          tr("Vous ne pouvez pas utiliser le module route<br>avec moins de 4 vacations entre les isochrones.<br>Vous devez donc desactiver cette option ou rallonger la duree"));
                    return;
                }
                if(routageData->timeStepMore24<4*routage->getBoat()->getVacLen()/60) {
                    QMessageBox::critical(0,tr("Creation d'un routage"),
                                          tr("Vous ne pouvez pas utiliser le module route<br>avec moins de 4 vacations entre les isochrones.<br>Vous devez donc desactiver cette option ou rallonger la duree"));
                    return;
                }
            }
        }
    }

    if(!routage->isDone()) {
        if(!routageData->toPoi) {
            QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de destination est invalide")));
            return;
        }

        if(!routageData->routeFromBoat && !routageData->fromPoi) {
            QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de depart est invalide")));
            return;
        }

        if(!routageData->routeFromBoat) {
            if(routageData->fromPoi->getLongitude()==routageData->toPoi->getLongitude() &&
                    routageData->fromPoi->getLatitude()==routageData->toPoi->getLatitude()) {
                if(!routage->getIsNewPivot() && !routage->getIsPivot()) {
                    QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le POI de depart et d'arrivee sont les memes, vous etes deja arrive...")));
                    return;
                }
            }
        }
        else {
            if(routageData->multiRoutage) {
                QMessageBox::critical(0,QString(QObject::tr("Routage")),QString(QObject::tr("Le routage ne peut pas partir du bateau si la fonction<br>multi-routage est utilisee")));
                return;
            }
        }
    }

    bool converted=false;
    if(routageData->autoConvertRoute || routageData->multiRoutage) {
        if(!routage->isConverted()) {
            if (!centralWidget->freeRouteName(routageData->name.trimmed(),NULL)) {
                QMessageBox msgBox;
                msgBox.setText(tr("Ce nom de route est deja utilise, veuillez changer le nom du routage"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                return;
            }
            if(routage->isDone()) {
                int rep=QMessageBox::Yes;
                if(routageData->whatIfUsed && (routageData->whatIfTime!=0 || routageData->whatIfWind!=100))
                    rep = QMessageBox::question (0,
                                                 tr("Convertir en route"),
                                                 tr("Ce routage a ete calcule avec une hypothese modifiant les donnees du grib<br>La route ne prendra pas ce scenario en compte<br>Etes vous sur de vouloir le convertir en route?"),
                                                 QMessageBox::Yes | QMessageBox::No);
                if(rep==QMessageBox::Yes)
                    connect(this,SIGNAL(destroyed()),routage,SLOT(convertToRoute()));
            }
            else
                converted=true;
        }
    }
    else
    {
        if(routageData->I_iso && routageData->timeStepLess24 != routageData->timeStepMore24)
        {
            QMessageBox::critical(0,tr("Isochrones inverses"),
                                  tr("Pour l'instant, on ne peut pas calculer les isochrones inverses<br>si la duree des isochrones est variable"));
            return;
        }
    }

    routage->setUseMultiThreading(routageData->useMultiThreading);
    routage->setPoiPrefix(routageData->poiPrefix);
    routage->setName(routageData->name.trimmed());
    routage->setWidth(routageData->lineWidth);
    routage->setColor(routageData->lineColor);
    QDateTime dd=routageData->startTime;
    if(routage->getBoat()->get_boatType()==BOAT_VLM)
    {
        time_t ddd=dd.toTime_t();
        ddd=floor((double)ddd/(double)routage->getBoat()->getVacLen())*routage->getBoat()->getVacLen();
        dd=dd.fromTime_t(ddd);
    }
    routage->setStartTime(dd);
    routage->setCheckCoast(routageData->checkCoast);
    routage->setCheckLine(routageData->checkLine);
    bool reCalculateAlternative=false;

    if(routage->isDone() && (routage->getNbAlternative()!=routageData->nbAlternative||
                             routage->getThresholdAlternative()!=routageData->thresholdAlternative))
        reCalculateAlternative=true;

    routage->setNbAlternative(routageData->nbAlternative);
    routage->setThresholdAlternative(routageData->thresholdAlternative);
    routage->useConverge=routageData->useConverge;
    routage->pruneWakeAngle=routageData->pruneWakeAngle;
    routage->setColorGrib(routageData->colorGrib);
    routage->setRoutageOrtho(routageData->routageOrtho);
    routage->setShowBestLive(routageData->showBestLive);
    routage->setAutoZoom(routageData->autoZoom);
    routage->setZoomLevel(routageData->zoomLevel);
    routage->setVisibleOnly(routageData->visibleOnly);
    routage->setRouteFromBoat(routageData->routeFromBoat);
    routage->setSpeedLossOnTack(routageData->speedLossOnTack);
    routage->setMaxPortant(routageData->maxPortant);
    routage->setMaxPres(routageData->maxPres);
    routage->set_maxWaveHeight(routageData->maxWaveHeight);
    routage->setMinPortant(routageData->minPortant);
    routage->setMinPres(routageData->minPres);
    routage->set_multiRoutage(routageData->multiRoutage);
    routage->set_multiNb(routageData->multiNb);
    routage->set_multiDays(routageData->multiDays);
    routage->set_multiHours(routageData->multiHours);
    routage->set_multiMin(routageData->multiMin);

    if(centralWidget->getPlayer()->getType()!=BOAT_REAL)
        routage->setBoat(routageData->curentBoat);
    else
        routage->setBoat((boat *) centralWidget->getRealBoat());

    if(!routage->isDone()) {
        routage->setFromPOI(routageData->fromPoi);
        routage->setToPOI(routageData->toPoi);
    }

    routage->setWhatIfUsed(routageData->whatIfUsed);
    routage->setWhatIfDate(routageData->whatIfDate);
    routage->setWhatIfWind(routageData->whatIfWind);
    routage->setWhatIfTime(routageData->whatIfTime);
    routage->setAngleRange(routageData->angleRange);
    routage->setAngleStep(routageData->angleStep);
    routage->setTimeStepMore24(routageData->timeStepMore24);
    routage->setTimeStepLess24(routageData->timeStepLess24);
    routage->setExplo(routageData->explo);
    routage->setShowIso(routageData->showIso);
    routage->setUseRouteModule(routageData->useRouteModule);
    if(!routage->isDone())
        Settings::setSetting(autoConvertToRoute,routageData->autoConvertRoute?1:0);

    if(routageData->autoConvertRoute || routageData->multiRoutage) {
        if(converted)
            routage->setConverted();
    }
    else {
        routage->setIsoRouteValue(routageData->isoRouteValue);
        routage->setI_iso(routageData->I_iso);
    }

    if(!routage->isDone())
        routage->setIsoRouteValue(routage->getTimeStepMore24());
    else if(reCalculateAlternative)
        routage->calculateAlternative();

    if(routage->isDone())
    {
        if(!routage->getColorGrib() && centralWidget->get_terrain()->getRoutageGrib()==routage)
            centralWidget->get_terrain()->setRoutageGrib(NULL);
        else if(routage->getColorGrib() && centralWidget->get_terrain()->getRoutageGrib()!=routage)
            centralWidget->get_terrain()->setRoutageGrib(routage);
    }

    if(createMode || !routage->isConverted() || routage->getIsNewPivot())
    {
        centralWidget->update_menuRoutage();
        QApplication::processEvents();
        if(createMode || routage->getIsNewPivot())
            routage->calculate();
        else if(routage->getI_iso() && !routage->getI_done() && !routage->isConverted())
            routage->calculateInverse();
        else if(routage->getI_iso() && routage->getI_done() && !routage->isConverted())
            routage->showIsoRoute();
    }

    if(view) view->set_dialogVisibility(false);
    deleteLater();
}

void DialogRoutage_ctrl::exitDialog(void) {
    if(createMode || routage->getIsNewPivot())
    {
        bool b=routage->getIsNewPivot();
        delete routage;
        centralWidget->removeRoutage(routage);
        routage=NULL;
        if(b)
            centralWidget->update_menuRoutage();
    }
    if(view) view->set_dialogVisibility(false);
    deleteLater();
}

void DialogRoutage_ctrl::launchDialog(void) {
    if(view) view->set_dialogVisibility(true);
}

void DialogRoutage_ctrl::dialogRoutage(myCentralWidget *centralWidget,ROUTAGE * routage,POI * endPoi,bool createMode) {
    if(!centralWidget || !routage) return;

    if(endPoi)
        routage->setToPOI(endPoi);

    if(QThread::idealThreadCount()<=1)
        routage->setUseMultiThreading(false);

    DialogRoutage_ctrl * ctrl=new DialogRoutage_ctrl(centralWidget,routage,createMode);
    ctrl->launchDialog();
}
