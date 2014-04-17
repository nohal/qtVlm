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
#ifdef QT_V5
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>
#include <QtCore/QThread>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QColorDialog>
#else
#include <QWidget>
#include <QMessageBox>
#include <QThread>
#include <QFileDialog>
#include <QColorDialog>
#endif
#include <QDebug>
#ifdef QT_V5
#include <QUiLoader>
#else
#include <QtUiTools/QUiLoader>
#endif
#include "DialogParamVlm.h"
#include "settings.h"
#include "MainWindow.h"
#include "DataManager.h"
#include "mycentralwidget.h"
#include "GshhsReader.h"
#include "Util.h"
#include "Player.h"
#include "BoardInterface.h"
#include "inetConnexion.h"
#include <QScreen>

DialogParamVlm::DialogParamVlm(MainWindow * main,myCentralWidget * parent) : QDialog(parent)
{
    centralWidget=parent;
    setupUi(this);
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    connect(this,SIGNAL(resetTraceCache()),parent,SIGNAL(resetTraceCache()));
    connect(this,SIGNAL(paramVLMChanged()),main,SLOT(slotParamChanged()));
    connect(this, SIGNAL(inetUpdated()), main, SLOT(slotInetUpdated()));
    connect(this,SIGNAL(redrawGrib()),centralWidget,SIGNAL(redrawGrib()));

    connect(radioBtn_vac,SIGNAL(toggled(bool)),this,SLOT(radioBtn_vac_toggle(bool)));
    connect(radioBtn_time,SIGNAL(toggled(bool)),this,SLOT(radioBtn_time_toggle(bool)));
    connect(radioBtn_dist,SIGNAL(toggled(bool)),this,SLOT(radioBtn_dist_toggle(bool)));
    connect(pushButton,SIGNAL(clicked()),this,SLOT(changeColor_POI()));
    connect(pushButton_2,SIGNAL(clicked()),this,SLOT(changeColor_qtBoat()));
    connect(pushButton_3,SIGNAL(clicked()),this,SLOT(changeColor_WP()));
    connect(pushButton_4,SIGNAL(clicked()),this,SLOT(changeColor_qtBoat_sel()));
    connect(pushButton_5,SIGNAL(clicked()),this,SLOT(changeColor_Marque_WP()));
    connect(pushButton_6,SIGNAL(clicked()),this,SLOT(changeColor_Balise()));
    connect(buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
    connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
    connect(btn_browseGrib,SIGNAL(clicked()),this,SLOT(doBtn_browseGrib()));

    connect(pushButton_7,SIGNAL(clicked()),this,SLOT(slot_chgMapFolder()));
    connect(this->resetDialogPosition,SIGNAL(clicked()),this,SLOT(slot_resetDialogPosition()));
    /* Drawing / affichage */
    displayCoords->addItem( tr("dddegmm'ss\""), "dddegmm'ss");
    displayCoords->addItem( tr("dddegmm,mm'"), "dddegmm,mm'");
    displayCoords->addItem( tr("dd,dddeg"), "dd,dddeg");
    QString tunit;
    int ind;
    tunit = Settings::getSetting(unitsPosition).toString();
    ind = (tunit=="") ? 0 : displayCoords->findData(tunit);
    displayCoords->setCurrentIndex(ind);
    cb_oppLabelType->addItem(tr("Pseudo"));
    cb_oppLabelType->addItem(tr("Nom"));
    cb_oppLabelType->addItem(tr("Numero"));
    cb_oppLabelType->setCurrentIndex(Settings::getSetting(opp_labelType).toInt());
    this->chkPavillon->setCheckState(Settings::getSetting(showFlag).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->chkFusion->setChecked(Settings::getSetting(fusionStyle).toInt()==1);
#ifndef QT_V5
    chkFusion->hide();
#endif
    // QDir dir(QApplication::applicationDirPath());
    // QStringList files=dir.entryList(QDir::Files);
    //QPluginLoader pgl;
    comboBoard->addItem(tr("Classical VLM board"),0);
    //qWarning()<<"start loading plugins";
    if(parent->getPlayer() && parent->getPlayer()->getType()==BOAT_VLM)
    {
        QDir        dir (Util::currentPath() + "/boards");
        QStringList files = dir.entryList (QStringList ("*.ui"));
        QUiLoader   loader;
        foreach(const QString &filename,files)
        {
           QFile    uiFile (dir.filePath (filename));
           uiFile.open (QFile::ReadOnly);
           QWidget* tst = loader.load (&uiFile);
           uiFile.close();
           if ((tst != NULL) && (qobject_cast<BoardInterface*> (tst) != NULL))
           {
              comboBoard->addItem(tst->windowTitle(), dir.filePath (filename));
              delete tst;
           } else {
               qWarning() << "Could not load board definition"
                          << filename;
    #ifdef QT_V5
               qWarning() << loader.errorString();
    #endif
           }
        }
    }
    QString currentBoard=Settings::getSetting(vlmBoardType).toString();
    // foreach(const QString &filename,files)
    // {
    //     QFileInfo info(filename);
    //     pgl.setFileName(info.baseName());
    //     pgl.load();
    //     if(!pgl.isLoaded())
    //     {
    //         //qWarning()<<"unable to load 1"<<pgl.fileName()<<pgl.errorString();
    //         continue;
    //     }
    //     BoardInterface * plugin=qobject_cast<BoardInterface*>(pgl.instance());
    //     if(!plugin || comboBoard->findText(plugin->getName())!=-1)
    //     {
    //         //qWarning()<<"unable to load 2"<<pgl.fileName()<<pgl.errorString();
    //         pgl.unload();
    //         continue;
    //     }
    //     this->comboBoard->addItem(plugin->getName(),pgl.fileName());
    //     if(currentBoard!=pgl.fileName())
    //         pgl.unload();
    // }
    // qWarning()<<"load plugins finished";
    if(comboBoard->findData(currentBoard)!=-1)
        comboBoard->setCurrentIndex(comboBoard->findData(currentBoard));
    else
        comboBoard->setCurrentIndex(0);

    this->newBoardShadow->setChecked(Settings::getSetting(newBoard_Shadow).toInt()==1);
    QString skinName=Settings::getSetting(defaultSkin).toString();
    if(!QFile(skinName).exists())
        skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    this->edt_skinFile->setText(skinName);
    connect(this->btn_browseSkin,SIGNAL(clicked()),this,SLOT(doBtn_browseSkin()));


    QString mapsFolderString = Settings::getSetting(mapsFolderName).toString();
    mapsFolder->setText(mapsFolderString);
    mapsFolder->setToolTip(mapsFolderString);
    if(QThread::idealThreadCount()<=1)
    {
        this->gribMono->setChecked(true);
        this->gribMono->setEnabled(false);
        this->gribMulti->setEnabled(false);
        this->gribAuto->setEnabled(false);
    }
    else
    {
        int gdm=Settings::getSetting(gribDrawingMethod).toInt();
        if(gdm==0)
            this->gribAuto->setChecked(true);
        else if (gdm==1)
            this->gribMono->setChecked(true);
        else
            this->gribMulti->setChecked(true);
        int cal1=Settings::getSetting(gribBench1).toInt();
        if(cal1>0)
        {
            int cal2=Settings::getSetting(gribBench2).toInt();
            QString tt=tr("If checked, let qtVlm choose the fastest way for your computer")+"<br>"+
                    tr("to display the Grib.")+"<br>";
            tt+=tr("The result of the benchmark gives ")+QString().setNum(cal1)+tr(" ms for multithread against")+"<br>"+
                    QString().setNum(cal2)+tr(" ms for monothread.")+"<br>";
            if(cal1<cal2)
                tt+=tr("Therefore the automatic choice will be multithread");
            else
                tt+=tr("Therefore the automatic choice will be monothread");
            gribAuto->setToolTip(tt.replace(" ","&nbsp;"));
        }
    }


    /* Colors */

    setColor(Settings::getSetting(POIColor).toString(),0);
    setColor(Settings::getSetting(MarqueWPColor).toString(),4);
    setColor(Settings::getSetting(qtBoatColor).toString(),1);
    setColor(Settings::getSetting(qtBoatSelColor).toString(),2);
    setColor(Settings::getSetting(WPColor).toString(),5);
    setColor(Settings::getSetting(BaliseColor).toString(),6);

    /* Bateau */
    initEstime();

    chk_centerOnSynch->setCheckState(Settings::getSetting(centerOnSynch).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_centerOnBoatChange->setCheckState(Settings::getSetting(centerOnBoatChange).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_askConfirm->setCheckState(Settings::getSetting(askConfirmation).toInt()==1?Qt::Checked:Qt::Unchecked);

    /*Route*/
    this->speedLossOnTackReal->setValue(Settings::getSetting(speedLoss_On_TackReal).toInt());
    this->speedLossOnTackVlm->setValue(Settings::getSetting(speedLoss_On_TackVlm).toInt());
    this->chkHideRoute->setCheckState(Settings::getSetting(autoHideRoute).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoRemove->setCheckState(Settings::getSetting(autoRemovePoiFromRoute).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoAt->setCheckState(Settings::getSetting(autoFillPoiHeading).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->routeSortByName->setChecked(Settings::getSetting(routeSort_ByName).toInt()==1);

    /* Trace */
    trace_length->setValue(Settings::getSetting(traceLength).toInt());
    speedReplay->setValue(Settings::getSetting(speed_replay).toInt());
    chk_oppTrace->setCheckState(Settings::getSetting(opp_trace).toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Compas */
    chk_showCompass->setCheckState(Settings::getSetting(showCompass).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_showPolar->setCheckState(Settings::getSetting(showPolar).toInt()==1?Qt::Checked:Qt::Unchecked);

    if(Settings::getSetting(scalePolar).toInt()==1)
    {
        chk_scalePolarF->setChecked(true);
    }
    else
    {
        chk_scaleEstime->setChecked(true);
    }
     /* Grib */

    chk_askGribFolder->setCheckState(Settings::getSetting(askGribFolder).toInt()==1?Qt::Checked:Qt::Unchecked);
    edt_gribFolder->setText(Settings::getSetting(edtGribFolder).toString());

    chk_gribZoomOnLoad->setCheckState(Settings::getSetting(gribZoomOnLoad).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_gribDelete->setChecked(Settings::getSetting(gribDelete).toInt()==1);
    chk_externalMail->setCheckState(Settings::getSetting(sDocExternalMail).toInt()==1?Qt::Checked:Qt::Unchecked);
    sailsDocPress->setCheckState(Settings::getSetting(sailsDoc_press).toInt()==1?Qt::Checked:Qt::Unchecked);
    forceWind->setChecked(Settings::getSetting(force_Wind).toInt()==1);
    forcedTWS->setValue(Settings::getSetting(forced_TWS).toDouble());
    forcedTWD->setValue(Settings::getSetting(forced_TWD).toDouble());
    forceCurrents->setChecked(Settings::getSetting(force_Currents).toInt()==1);
    forcedCS->setValue(Settings::getSetting(forced_CS).toDouble());
    forcedCD->setValue(Settings::getSetting(forced_CD).toDouble());

    /* GPS */
    chk_activateEmulation->setCheckState(
         Settings::getSetting(gpsEmulEnable).toString()=="1"?Qt::Checked:Qt::Unchecked);
    this->enableGesture->setCheckState(
         Settings::getSetting(enable_Gesture).toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Settings::getSetting(gpsEmulSerialName).toString());
    spn_gpsDelay->setValue(Settings::getSetting(gpsEmulDelay).toInt());
    /* advanced */

    chk_forceUserAgent->setChecked(Settings::getSetting(forceUserAgent).toInt()==1?true:false);
    txt_userAgent->setText(Settings::getSetting(userAgent).toString());
    txt_userAgent->setEnabled(Settings::getSetting(forceUserAgent).toInt()==1);
    defFontName->findText(Settings::getSetting(defaultFontName).toString());
    defFontSize->setValue(8.0+Settings::getSetting(defaultFontSizeInc).toDouble());
    for(int i=0;i<NB_URL;i++)
        url_list->addItem(url_name[i]+": "+url_str[i]);
    url_list->setCurrentIndex(Settings::getSetting(vlm_url).toInt());
    switch(::INTERPOLATION_DEFAULT)
    {
        case INTERPOLATION_TWSA:
            this->interpolTWSA->setChecked(true);
            break;
        case INTERPOLATION_HYBRID:
            this->interpolHyb->setChecked(true);
            break;
        case INTERPOLATION_SELECTIVE_TWSA:
            this->interpolSelect->setChecked(true);
            break;
    }
    interpolBox->hide();

//#ifndef __QTVLM_WITH_TEST
//    urlGroup->setVisible(false);
//#endif

    saveWinGeometry->setCheckState(Settings::getSetting(saveMainWindowGeometry).toInt()==1?Qt::Checked:Qt::Unchecked);
    Util::setFontDialog(this);
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
}
void DialogParamVlm::slot_screenResize()
{
}

void DialogParamVlm::initEstime(void) {
    estimeVal_dist->setValue(Settings::getSetting(estimeLen).toInt());
    estimeVal_time->setValue(Settings::getSetting(estimeTime).toInt());
    estimeVal_vac->setValue(Settings::getSetting(estimeVac).toInt());

    estimeVal_time->setEnabled(false);
    estimeVal_vac->setEnabled(false);
    estimeVal_dist->setEnabled(false);

    this->radioBtn_time->setEnabled(true);
    this->radioBtn_dist->setEnabled(true);
    radioBtn_vac->setEnabled(true);



    switch(Settings::getSetting(estimeType).toInt())
    {
    case 0:
        radioBtn_time->setChecked(true);
        estimeVal_time->setEnabled(true);
        break;
    case 1:
        radioBtn_vac->setChecked(true);
        estimeVal_vac->setEnabled(true);
        break;
    case 2:
        radioBtn_dist->setChecked(true);
        estimeVal_dist->setEnabled(true);
        break;
    }


}

void DialogParamVlm::done(int result)
{
    Settings::saveGeometry(this);
    if(result == QDialog::Accepted)
    {
        /*drawing*/
        Settings::setSetting(unitsPosition,  displayCoords->itemData(displayCoords->currentIndex()).toString());
        Settings::setSetting(opp_labelType,QString().setNum(cb_oppLabelType->currentIndex()));
        Settings::setSetting(showFlag,this->chkPavillon->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(fusionStyle,this->chkFusion->isChecked()?1:0);
        QString previousBoardSetting=Settings::getSetting(vlmBoardType).toString();
        QString newBoardSetting=comboBoard->itemData(comboBoard->currentIndex()).toString();
        Settings::setSetting(newBoard_Shadow,this->newBoardShadow->isChecked()?1:0);
        QString skinName=edt_skinFile->text();
        if(!QFile(skinName).exists())
            skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
        Settings::setSetting(defaultSkin,skinName);
        if(previousBoardSetting!=newBoardSetting)
        {
            Settings::setSetting(vlmBoardType,newBoardSetting);
            centralWidget->getMainWindow()->loadBoard();
        }
        int gdm=2;
        if(this->gribAuto->isChecked())
            gdm=0;
        else if (this->gribMono->isChecked())
            gdm=1;
        Settings::setSetting(gribDrawingMethod,gdm);
        Settings::setSetting(defaultFontName,this->defFontName->currentText());
        Settings::setSetting(defaultFontSizeInc,QString().setNum(this->defFontSize->value()-8.0));

        if(Settings::getSetting(mapsFolderName).toString() != mapsFolder->text())
        {
            QString mapDir = mapsFolder->text();
            QDir dir(mapDir);
            QDir appDir=Util::currentPath();
            if(dir.rootPath()==appDir.rootPath())
                mapDir=appDir.relativeFilePath(mapDir);
            else
                mapDir=appDir.absoluteFilePath(mapDir);
            qWarning() << "Setting map folder to " << mapDir;
            Settings::setSetting(mapsFolderName,mapDir);
            centralWidget->loadGshhs();
        }
        /* colors */

        Settings::setSetting(POIColor,POI_color);
        Settings::setSetting(MarqueWPColor,Marque_WP_color);
        Settings::setSetting(qtBoatColor,qtBoat_color);
        Settings::setSetting(qtBoatSelColor,qtBoat_sel_color);
        Settings::setSetting(WPColor,WP_color);
        Settings::setSetting(BaliseColor,Balise_color);

        /* Bateau */
        Settings::setSetting(estimeLen, QString().setNum(estimeVal_dist->value()));
        Settings::setSetting(estimeTime, QString().setNum(estimeVal_time->value()));
        Settings::setSetting(estimeVac, QString().setNum(estimeVal_vac->value()));


        if(radioBtn_time->isChecked())
            Settings::setSetting(estimeType,0);
        else if(radioBtn_vac->isChecked())
            Settings::setSetting(estimeType,1);
        else
            Settings::setSetting(estimeType,2);

        Settings::setSetting(centerOnSynch,chk_centerOnSynch->checkState()==Qt::Checked?1:0);
        Settings::setSetting(centerOnBoatChange,chk_centerOnBoatChange->checkState()==Qt::Checked?1:0);
        Settings::setSetting(askConfirmation,chk_askConfirm->checkState()==Qt::Checked?1:0);

        /* Trace */
        if(Settings::getSetting(traceLength).toInt()!=trace_length->value())
            emit resetTraceCache();
        Settings::setSetting(traceLength,QString().setNum(trace_length->value()));
        Settings::setSetting(speed_replay,QString().setNum(speedReplay->value()));
        Settings::setSetting(opp_trace,chk_oppTrace->checkState()==Qt::Checked?1:0);

        /* Compas */
        Settings::setSetting(showCompass,chk_showCompass->checkState()==Qt::Checked?1:0);
        Settings::setSetting(showPolar,chk_showPolar->checkState()==Qt::Checked?1:0);
        if(this->chk_scalePolarF->isChecked())
            Settings::setSetting(scalePolar,1);
        else
            Settings::setSetting(scalePolar,2);
        /* Route */

        Settings::setSetting(speedLoss_On_TackReal, QString().setNum(speedLossOnTackReal->value()));
        Settings::setSetting(speedLoss_On_TackVlm, QString().setNum(speedLossOnTackVlm->value()));
        Settings::setSetting(autoHideRoute,this->chkHideRoute->checkState()==Qt::Checked?1:0);
        Settings::setSetting(autoRemovePoiFromRoute,this->autoRemove->isChecked()?1:0);
        Settings::setSetting(autoFillPoiHeading,this->autoAt->isChecked()?1:0);
        Settings::setSetting(routeSort_ByName,this->routeSortByName->isChecked()?1:0);

        /* Grib */

        Settings::setSetting(askGribFolder,chk_askGribFolder->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(edtGribFolder,edt_gribFolder->text());
        Settings::setSetting(gribZoomOnLoad,chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(gribDelete,chk_gribDelete->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(sDocExternalMail,chk_externalMail->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(sailsDoc_press,sailsDocPress->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(force_Wind,forceWind->isChecked()?1:0);
        Settings::setSetting(forced_TWS,forcedTWS->value());
        Settings::setSetting(forced_TWD,forcedTWD->value());
        Settings::setSetting(force_Currents,forceCurrents->isChecked()?1:0);
        Settings::setSetting(forced_CS,forcedCS->value());
        Settings::setSetting(forced_CD,forcedCD->value());
        DataManager * dataManager=centralWidget->get_dataManager();
        if(dataManager) dataManager->load_forcedParam();

        /* advanced */
        Settings::setSetting(gpsEmulEnable,chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(enable_Gesture,this->enableGesture->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting(gpsEmulSerialName, serialName->text());
        Settings::setSetting(gpsEmulDelay,spn_gpsDelay->value());
        Settings::setSetting(forceUserAgent,chk_forceUserAgent->isChecked()?1:0);
        Settings::setSetting(userAgent,txt_userAgent->text());

        int oldUrl = Settings::getSetting(vlm_url).toInt();
        Settings::setSetting(vlm_url,QString().setNum(url_list->currentIndex()));
        if(interpolTWSA->isChecked())
            INTERPOLATION_DEFAULT=INTERPOLATION_TWSA;
        else if(interpolSelect->isChecked())
            INTERPOLATION_DEFAULT=INTERPOLATION_SELECTIVE_TWSA;
        else
            INTERPOLATION_DEFAULT=INTERPOLATION_HYBRID;
        INTERPOLATION_DEFAULT=INTERPOLATION_HYBRID;
        Settings::setSetting(defaultInterpolation,INTERPOLATION_DEFAULT);
        //qWarning() << "old url=" << oldUrl << " new=" << url_list->currentIndex();
        if(oldUrl!=url_list->currentIndex())
            emit inetUpdated();

        Settings::setSetting(saveMainWindowGeometry,saveWinGeometry->checkState()==Qt::Checked?"1":"0");
        emit paramVLMChanged();
        emit redrawGrib();
    }
    QDialog::done(result);
}

void DialogParamVlm::slot_chgMapFolder(void) {
    QString dir = mapsFolder->text();
    bool exitLoop = false;

    while (!exitLoop) {

        dir = QFileDialog::getExistingDirectory(centralWidget, tr("Select maps folder"),
                                                    dir,
                                                    QFileDialog::ShowDirsOnly);
        if(dir.isEmpty()) {
            exitLoop=true;
        }
        else if(dir != mapsFolder->text()) {
            GshhsReader * gshhsReaderPv = new GshhsReader((dir+"/gshhs").toLatin1().data());

            int polyVersion = gshhsReaderPv->getPolyVersion();
            if(polyVersion == -1) {
                QMessageBox::warning(this,tr("Changing maps folder"),tr("Selected folder doesn't contain maps"));
            }
            else if(polyVersion!=220) {
                QMessageBox::warning(this,tr("Changing maps folder"),tr("Selected folder contains maps with wrong version"));
            }
            else {
                exitLoop=true;
                mapsFolder->setText(dir);
                mapsFolder->setToolTip(dir);
            }
            delete gshhsReaderPv;
        }
        else
            exitLoop=true;
    }
}

void DialogParamVlm::forceUserAgent_changed(int newVal)
{
    txt_userAgent->setEnabled(newVal==Qt::Checked);
}

void DialogParamVlm::changeColor_POI(void)
{
    changeColor(0);
}

void DialogParamVlm::changeColor_qtBoat(void)
{
    changeColor(1);
}

void DialogParamVlm::changeColor_qtBoat_sel(void)
{
    changeColor(2);
}

void DialogParamVlm::changeColor_Marque_WP(void)
{
    changeColor(4);
}

void DialogParamVlm::changeColor_WP(void)
{
    changeColor(5);
}

void DialogParamVlm::changeColor_Balise(void)
{
    changeColor(6);
}

void DialogParamVlm::changeColor(int type)
{
    QColor color = QColorDialog::getColor(getColor(type), this);
    if(color.isValid())
        setColor(color.name (),type);
}

QColor DialogParamVlm::getColor(int type)
{
    switch(type)
    {
        case 0:
            return QColor(POI_color);
            break;
        case 1:
            return QColor(qtBoat_color);
            break;
        case 2:
            return QColor(qtBoat_sel_color);
            break;
/*        case 3:
            return QColor(opp_color);
            break;*/
        case 4:
            return QColor(Marque_WP_color);
            break;
        case 5:
            return QColor(WP_color);
            break;
        case 6:
            return QColor(Balise_color);
            break;
    }
    return Qt::white;
}

void DialogParamVlm::setColor(QString color,int type)
{
    QString style ="background-color: "+color;
    switch(type)
    {
        case 0:
            POI_frame->setStyleSheet(style);
            POI_color=color;
            break;
        case 1:
            qtBoat_frame->setStyleSheet(style);
            qtBoat_color=color;
            break;
        case 2:
            qtBoat_sel_frame->setStyleSheet(style);
            qtBoat_sel_color=color;
            break;
        case 4:
            Marque_WP_frame->setStyleSheet(style);
            Marque_WP_color=color;
            break;
        case 5:
            WP_frame->setStyleSheet(style);
            WP_color=color;
            break;
        case 6:
            Balise_frame->setStyleSheet(style);
            Balise_color=color;
            break;
    }
}

void DialogParamVlm::radioBtn_time_toggle(bool val)
{
    estimeVal_time->setEnabled(val);
}

void DialogParamVlm::radioBtn_vac_toggle(bool val)
{
    estimeVal_vac->setEnabled(val);
}

void DialogParamVlm::radioBtn_dist_toggle(bool val)
{
    estimeVal_dist->setEnabled(val);
}

void DialogParamVlm::doBtn_browseGrib(void)
{
     QString dir = QFileDialog::getExistingDirectory(this, tr("Repertoire Grib"),
                                                 edt_gribFolder->text());
     if(dir!="")
         edt_gribFolder->setText(dir);
}
void DialogParamVlm::doBtn_browseSkin(void)
{
    QString skinPath=QFileInfo(edt_skinFile->text()).absolutePath();
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Selectionner un skin tableau de bord VLM"), skinPath, "png (*.png)");
    if(fileName!="")
         edt_skinFile->setText(QFileInfo(fileName).absoluteFilePath());
}

void DialogParamVlm::slot_changeParam()
{
    initEstime();
    bool real=centralWidget->getPlayer()->getType()!=BOAT_VLM;
    chk_showCompass->setCheckState(Settings::getSetting(showCompass).toInt()==1?Qt::Checked:Qt::Unchecked);
    concurrent_box->setVisible(!real);
    newBoardShadow->setVisible(!real);
    label_skin->setVisible(!real);
    edt_skinFile->setVisible(!real);
    btn_browseSkin->setVisible(!real);
    label_4->setVisible(!real);
    pushButton_2->setVisible(!real);
    label_6->setVisible(!real);
    pushButton_4->setVisible(!real);
    chk_centerOnSynch->setVisible(!real);
    chk_centerOnBoatChange->setVisible(!real);
    chk_askConfirm->setVisible(!real);
    autoRemove->setVisible(!real);
    chkHideRoute->setVisible(!real);
    label_21->setVisible(!real);
    speedLossOnTackVlm->setVisible(!real);
    label_22->setVisible(!real);
    chk_oppTrace->setVisible(!real);
}

void DialogParamVlm::on_chk_scaleEstime_toggled(bool checked)
{
    if (checked)
    {
        this->radioBtn_time->setEnabled(false);
        this->radioBtn_dist->setEnabled(false);
        this->radioBtn_vac->setChecked(true);
    }
    else
    {
        this->radioBtn_time->setEnabled(true);
        this->radioBtn_dist->setEnabled(true);
    }
}
void DialogParamVlm::slot_resetDialogPosition()
{
    QStringList allKeys=Settings::getAllKeys("DialogGeometry");
    for(int n=0;n<allKeys.size();++n)
    {
        if(allKeys.at(n).right(7)==".height" || allKeys.at(n).right(6)==".width" || allKeys.at(n).right(10)==".positionx" || allKeys.at(n).right(10)==".positiony" )
            Settings::removeSetting(allKeys.at(n),"DialogGeometry");
    }
    Util::setFontDialog(this);
}
