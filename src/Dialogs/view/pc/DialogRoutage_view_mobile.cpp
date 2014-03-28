#include <QThread>

#include "mycentralwidget.h"
#include "routage.h"
#include "boatVLM.h"
#include "Player.h"
#include "settings.h"
#include "POI.h"

#include "DialogRoutage_view_mobile.h"
#include "DialogRoutage_ctrl.h"
#include "Dialog_view_pc.h"

#include "dataDef.h"

DialogRoutage_view_mobile::DialogRoutage_view_mobile(myCentralWidget *centralWidget,DialogRoutage_ctrl * ctrl):
Dialog_view_pc(centralWidget),
DialogRoutage_view(centralWidget,ctrl)
{
    //INIT_DIALOG
    setupUi(this);
    initDialog();

    setWindowTitle(tr("Parametres Routage"));
    connect(this->Default,SIGNAL(clicked()),this,SLOT(slot_default()));
}

DialogRoutage_view_mobile::~DialogRoutage_view_mobile()
{
}
void DialogRoutage_view_mobile::initDialogState(RoutageData * routageData) {
    QString m;
    this->routageData=routageData;

    this->i_iso->setChecked(routageData->I_done);
    this->i_iso->setDisabled((!routageData->isDone || routageData->I_done));
    this->isoRoute->setDisabled(!routageData->isDone);
    if(routageData->isDone)
    {
        this->isoRoute->setMaximum(routageData->timeStepMore24);
        this->isoRoute->setValue(routageData->isoRouteValue);
    }
    m.sprintf("%d",QThread::idealThreadCount());
    m=tr("Calculer en parallele\n(")+m+tr(" processeurs disponibles)");
    multi->setText(m);
    if(QThread::idealThreadCount()<=1)
        multi->setEnabled(false);
    multi->setChecked(routageData->useMultiThreading);

    inputTraceColor =new InputLineParams(routageData->lineWidth,routageData->lineColor,1.6,  QColor(Qt::red),this,0.1,5,1,true,true);
    colorBox->layout()->addWidget(inputTraceColor);

    editName->setText(routageData->name);

    editDateBox->setDateTime(routageData->startTime);
    editDateBox->setEnabled(true);

    whatIfDate->setDateTime(routageData->whatIfDate);
    whatIfUse->setChecked(routageData->whatIfUsed);
    whatIfWind->setValue(routageData->whatIfWind);
    whatIfTime->setValue(routageData->whatIfTime);

    autoZoom->setChecked(routageData->autoZoom);
    this->zoomLevel->setValue(routageData->zoomLevel);

    visibleOnly->setChecked(routageData->visibleOnly);
    this->poiPrefix->setText(routageData->poiPrefix);
    this->startFromBoat->setChecked(routageData->routeFromBoat);
    this->maxPortant->setValue(routageData->maxPortant);
    this->maxPres->setValue(routageData->maxPres);
    this->minPortant->setValue(routageData->minPortant);
    this->minPres->setValue(routageData->minPres);
    this->maxWaveHeight->setValue(routageData->maxWaveHeight);
    if(!routageData->isDone)
        this->convRoute->setChecked(Settings::getSetting(autoConvertToRoute).toInt()==1);
    if(routageData->finalETA.isNull())
        this->groupBox_eta->setHidden(true);
    else
    {
        this->groupBox_eta->setHidden(false);
        this->editDateBox_2->setDateTime(routageData->finalETA);
    }

    this->toolBox->setCurrentIndex(0);
    editName->setFocus();

    if(routageData->boatType == BOAT_REAL) {
        this->speedLossOnTack->setValue(Settings::getSetting(speedLoss_On_TackReal).toInt());
        editBoat->addItem(routageData->boatList.at(0));
        editBoat->setEnabled(false);
    }
    else {
        this->speedLossOnTack->setValue(Settings::getSetting(speedLoss_On_TackVlm).toInt());
        for(int i=0;i<routageData->boatList.count();++i) {
            editBoat->addItem(routageData->boatList.at(i),VPtr<boat>::asQVariant(routageData->boatPtrList.at(i)));
            if(routageData->boatPtrList.at(i)==routageData->curentBoat)
                editBoat->setCurrentIndex(i);
        }
    }

    if(routageData->isDone) {
        if(routageData->fromPoi) {
            fromPOI->addItem(routageData->fromPoi->getName(),VPtr<POI>::asQVariant(routageData->fromPoi));
            fromPOI->setCurrentIndex(0);
        }
        if(routageData->toPoi) {
            toPOI->addItem(routageData->toPoi->getName(),VPtr<POI>::asQVariant(routageData->toPoi));
            toPOI->setCurrentIndex(0);
        }
    }
    else {
        for(int i=0;i<routageData->poiList.count();++i) {
            fromPOI->addItem(routageData->poiList.at(i),VPtr<POI>::asQVariant(routageData->poiPtrList.at(i)));
            if(routageData->fromPoi==routageData->poiPtrList.at(i))
                fromPOI->setCurrentIndex(i);
            toPOI->addItem(routageData->poiList.at(i),VPtr<POI>::asQVariant(routageData->poiPtrList.at(i)));
            if(routageData->toPoi==routageData->poiPtrList.at(i))
                toPOI->setCurrentIndex(i);
        }
    }

    this->range->setValue(routageData->angleRange);
    this->step->setValue(routageData->angleStep);
    this->dureeLess24->setValue(routageData->timeStepLess24);
    this->dureeMore24->setValue(routageData->timeStepMore24);
    this->showIso->setChecked(routageData->showIso);
    this->explo->setValue(routageData->explo);
    this->useVac->setChecked(routageData->useRouteModule);
    this->log->setChecked(routageData->useConverge);
    this->pruneWakeAngle->setValue(routageData->pruneWakeAngle);
    this->colorIso->setChecked(routageData->colorGrib);
    this->RoutageOrtho->setChecked(routageData->routageOrtho);
    this->showBestLive->setChecked(routageData->showBestLive);
    this->checkCoast->setChecked(routageData->checkCoast);
    this->checkLines->setChecked(routageData->checkLine);
    this->nbAlter->setValue(routageData->nbAlternative);
    this->diver->setValue(routageData->thresholdAlternative);

    if(routageData->isDone || routageData->isNewPivot)
    {
        this->speedLossOnTack->setValue(qRound(routageData->speedLossOnTack*100));
        this->speedLossOnTack->setDisabled(true);
        this->editName->setDisabled(false);
        this->autoZoom->setDisabled(true);
        this->zoomLevel->setDisabled(true);
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
        this->useVac->setDisabled(true);
        this->log->setDisabled(true);
        this->pruneWakeAngle->setDisabled(true);
        this->RoutageOrtho->setDisabled(true);
        this->showBestLive->setDisabled(true);
        this->checkCoast->setDisabled(true);
        this->checkLines->setDisabled(true);
        if(routageData->isConverted)
            this->convRoute->setDisabled(true);

        this->startFromBoat->setDisabled(true);
        this->whatIfUse->setDisabled(true);
        this->whatIfDate->setDisabled(true);
        this->whatIfWind->setDisabled(true);
        this->whatIfTime->setDisabled(true);
        this->multi->setDisabled(true);
        this->maxPortant->setDisabled(true);
        this->maxPres->setDisabled(true);
        this->maxWaveHeight->setDisabled(true);
        this->minPortant->setDisabled(true);
        this->minPres->setDisabled(true);
        this->Default->setDisabled(true);
        this->multi_routage->setDisabled(true);
    }
    if(routageData->isNewPivot && !routageData->isDone)
    {
        this->speedLossOnTack->setDisabled(false);
        this->autoZoom->setDisabled(false);
        this->zoomLevel->setDisabled(false);
        this->visibleOnly->setDisabled(false);
        this->dureeMore24->setDisabled(false);
        this->dureeLess24->setDisabled(false);
        this->range->setDisabled(false);
        this->step->setDisabled(false);
        this->explo->setDisabled(false);
        this->toPOI->setDisabled(false);
        this->log->setDisabled(false);
        this->pruneWakeAngle->setDisabled(false);
        this->RoutageOrtho->setDisabled(false);
        this->showBestLive->setDisabled(false);
        this->checkCoast->setDisabled(false);
        this->checkLines->setDisabled(false);
        this->diver->setDisabled(false);
        this->nbAlter->setDisabled(false);
        this->whatIfUse->setDisabled(false);
        this->whatIfDate->setDisabled(false);
        this->whatIfWind->setDisabled(false);
        this->whatIfTime->setDisabled(false);
        this->multi->setDisabled(false);
        this->maxPres->setDisabled(false);
        this->maxWaveHeight->setDisabled(false);
        this->maxPortant->setDisabled(false);
        this->minPres->setDisabled(false);
        this->minPortant->setDisabled(false);
        this->Default->setDisabled(false);
        this->multi_routage->setDisabled(true);
    }
    if(routageData->isDone && !routageData->arrived)
        i_iso->setEnabled(false);

    this->multi_routage->setChecked(routageData->multiRoutage );
    this->multi_nb->setValue(routageData->multiNb+1);
    this->multi_days->setValue(routageData->multiDays);
    this->multi_hours->setValue(routageData->multiHours);
    this->multi_min->setValue(routageData->multiMin);

    this->toolBox->setCurrentIndex(0);



    Util::setWidgetSize(this);
    QList<QScrollArea *> scrolls=this->findChildren<QScrollArea *>(QString(),Qt::FindChildrenRecursively);
    if (scrolls.isEmpty())
        qWarning()<<"no scrollareas... bad luck";
    else
    {
        foreach(QScrollArea *s,scrolls)
        {
            //if(s==this->scrollArea_reserved) continue;
            QScroller::grabGesture(s->viewport());
            QScroller::grabGesture(s->viewport(),QScroller::LeftMouseButtonGesture);
        }
    }
}

void DialogRoutage_view_mobile::set_dialogVisibility(bool visible) {
    if(visible) {
        show();
        setWindowModality(Qt::ApplicationModal);
    }
    else
        hide();
}

void DialogRoutage_view_mobile::closeEvent(QCloseEvent * ) {
    slot_cancel();
}

void DialogRoutage_view_mobile::slot_ok() {

    qWarning() << "Slot ok";

    Settings::saveGeometry(this);

    routageData->name=editName->text();

    routageData->poiPrefix=poiPrefix->text();

    routageData->I_done=i_iso->isChecked();
    routageData->useMultiThreading=multi->isChecked();
    routageData->showIso=showIso->isChecked();
    routageData->useRouteModule=useVac->isChecked();
    routageData->useConverge=log->isChecked();
    routageData->colorGrib=colorIso->isChecked();
    routageData->routageOrtho=RoutageOrtho->isChecked();
    routageData->showBestLive=showBestLive->isChecked();
    routageData->checkCoast=checkCoast->isChecked();
    routageData->checkLine=checkLines->isChecked();

    routageData->startTime=editDateBox->dateTime();

    routageData->timeStepLess24=dureeLess24->value();
    routageData->timeStepMore24=dureeMore24->value();

    routageData->explo=explo->value();

    routageData->isoRouteValue=isoRoute->value();
    routageData->visibleOnly=visibleOnly->isChecked();
    routageData->routeFromBoat=startFromBoat->isChecked();

    routageData->whatIfDate=whatIfDate->dateTime();
    routageData->whatIfUsed=whatIfUse->isChecked();
    routageData->whatIfWind=whatIfWind->value();
    routageData->whatIfTime=whatIfTime->value();

    routageData->maxPortant=maxPortant->value();
    routageData->maxPres=maxPres->value();
    routageData->minPortant=minPortant->value();
    routageData->minPres=minPres->value();
    routageData->maxWaveHeight=maxWaveHeight->value();

    routageData->zoomLevel=zoomLevel->value();
    routageData->autoZoom=autoZoom->isChecked();

    routageData->lineWidth=inputTraceColor->getLineWidth();
    routageData->lineColor=inputTraceColor->getLineColor();

    if(editBoat->currentIndex()!=-1)
        routageData->curentBoat=VPtr<boat>::asPtr(editBoat->currentData());
    else
        routageData->curentBoat=NULL;

    if(toPOI->currentIndex()!=-1)
        routageData->toPoi=VPtr<POI>::asPtr(toPOI->currentData());
    else
        routageData->toPoi=NULL;

    if(fromPOI->currentIndex()!=-1)
        routageData->fromPoi=VPtr<POI>::asPtr(fromPOI->currentData());
    else
        routageData->fromPoi=NULL;

    routageData->angleRange=range->value();
    routageData->angleStep=step->value();
    routageData->pruneWakeAngle=pruneWakeAngle->value();
    routageData->speedLossOnTack=(double)this->speedLossOnTack->value()/100.00;

    routageData->nbAlternative=nbAlter->value();
    routageData->thresholdAlternative=diver->value();

    routageData->multiRoutage=multi_routage->isChecked();
    routageData->multiNb=multi_nb->value()-1;
    routageData->multiDays=multi_days->value();
    routageData->multiHours=multi_hours->value();
    routageData->multiMin=multi_min->value();

    routageData->I_iso=i_iso->isChecked();
    routageData->autoConvertRoute=convRoute->isChecked();

    ctrl->validateChange();
}

void DialogRoutage_view_mobile::slot_cancel() {
    qWarning() << "Slot cancel";
    Settings::saveGeometry(this);
    ctrl->exitDialog();
}

void DialogRoutage_view_mobile::slot_default()
{
    this->maxPres->setValue(70);
    this->maxWaveHeight->setValue(100);
    this->minPres->setValue(0);
    this->maxPortant->setValue(70);
    this->minPortant->setValue(0);
    this->step->setValue(3);
    this->dureeLess24->setValue(30);
    this->dureeMore24->setValue(60);
    this->range->setValue(180);
    this->showIso->setChecked(true);
    this->speedLossOnTack->setValue(100);
    this->useVac->setChecked(true);
    this->multi->setChecked(true);
    this->visibleOnly->setChecked(true);
    this->autoZoom->setChecked(true);
    this->zoomLevel->setValue(2);
    this->pruneWakeAngle->setValue(30);
    this->showBestLive->setChecked(true);
    this->RoutageOrtho->setChecked(true);
    this->colorIso->setChecked(false);
    this->explo->setValue(40);
    this->log->setChecked(true);
    this->whatIfUse->setChecked(false);
    this->checkCoast->setChecked(true);
    this->checkLines->setChecked(true);
    this->nbAlter->setValue(3);
    this->diver->setValue(75);
}

void DialogRoutage_view_mobile::GybeTack(int i)
{
    QFont font=this->labelTackGybe->font();
    if(i==100)
        font.setBold(false);
    else
        font.setBold(true);
    this->labelTackGybe->setFont(font);
}

