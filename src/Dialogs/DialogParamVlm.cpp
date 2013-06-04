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
#include "DialogParamVlm.h"
#include "settings.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "GshhsReader.h"
#include "Util.h"
#include "Grib.h"

DialogParamVlm::DialogParamVlm(MainWindow * main,myCentralWidget * parent) : QDialog(parent)
{
    centralWidget=parent;
    setupUi(this);
    Util::setFontDialog(this);
    connect(this,SIGNAL(resetTraceCache()),parent,SIGNAL(resetTraceCache()));
    connect(this,SIGNAL(paramVLMChanged()),main,SLOT(slotParamChanged()));
    connect(this, SIGNAL(inetUpdated()), main, SLOT(slotInetUpdated()));
    connect(this,SIGNAL(redrawGrib()),centralWidget,SIGNAL(redrawGrib()));

    /* Drawing / affichage */
    opp_labelType->addItem(tr("Pseudo"));
    opp_labelType->addItem(tr("Nom"));
    opp_labelType->addItem(tr("Numero"));
    opp_labelType->setCurrentIndex(Settings::getSetting("opp_labelType",0).toInt());

    this->chkPavillon->setCheckState(Settings::getSetting("showFlag",0,"showHideItem").toInt()==1?Qt::Checked:Qt::Unchecked);
    this->chkFusion->setChecked(Settings::getSetting("fusionStyle",0).toInt()==1);
    this->classicalBoard->setChecked(Settings::getSetting("classicalVlmBoard",0).toInt()==1);
    QString skinName=Settings::getSetting("defaultSkin",QFileInfo("img/skin_compas.png").absoluteFilePath()).toString();
    if(!QFile(skinName).exists())
        skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    this->edt_skinFile->setText(skinName);
    connect(this->btn_browseSkin,SIGNAL(clicked()),this,SLOT(doBtn_browseSkin()));


    QString mapsFolderString = Settings::getSetting("mapsFolder",appFolder.value("maps")).toString();
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
        int gdm=Settings::getSetting("gribDrawingMethod",0).toInt();
        if(gdm==0)
            this->gribAuto->setChecked(true);
        else if (gdm==1)
            this->gribMono->setChecked(true);
        else
            this->gribMulti->setChecked(true);
        int cal1=Settings::getSetting("gribBench1",-1).toInt();
        if(cal1>0)
        {
            int cal2=Settings::getSetting("gribBench2",-1).toInt();
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

    setColor(Settings::getSetting("POI_Color",QColor(Qt::black).name()).toString(),0);
    setColor(Settings::getSetting("Marque_WP_Color",QColor(Qt::red).name()).toString(),4);
    setColor(Settings::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString(),1);
    setColor(Settings::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString(),2);
    setColor(Settings::getSetting("WP_Color",QColor(Qt::darkYellow).name()).toString(),5);
    setColor(Settings::getSetting("Balise_Color",QColor(Qt::darkMagenta).name()).toString(),6);

    /* Bateau */
    initEstime();

    chk_centerOnSynch->setCheckState(Settings::getSetting("centerOnSynch","0").toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_centerOnBoatChange->setCheckState(Settings::getSetting("centerOnBoatChange","1").toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_askConfirm->setCheckState(Settings::getSetting("askConfirmation","0").toInt()==1?Qt::Checked:Qt::Unchecked);

    /*Route*/
    this->speedLossOnTackReal->setValue(Settings::getSetting("speedLossOnTackReal","100").toInt());
    this->speedLossOnTackVlm->setValue(Settings::getSetting("speedLossOnTackVlm","100").toInt());
    this->chkHideRoute->setCheckState(Settings::getSetting("autoHideRoute",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoRemove->setCheckState(Settings::getSetting("autoRemovePoiFromRoute",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoAt->setCheckState(Settings::getSetting("autoFillPoiHeading",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->routeSortByName->setChecked(Settings::getSetting("routeSortByName",1).toInt()==1);

    /* Trace */
    trace_length->setValue(Settings::getSetting("trace_length",12).toInt());
    speedReplay->setValue(Settings::getSetting("speed_replay",20).toInt());
    chk_oppTrace->setCheckState(Settings::getSetting("opp_trace","1").toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Compas */
    chk_showCompass->setCheckState(Settings::getSetting("showCompass",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_showPolar->setCheckState(Settings::getSetting("showPolar",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    if(Settings::getSetting("scalePolar",0).toInt()==1)
    {
        chk_scalePolarF->setChecked(true);
    }
    else
    {
        chk_scaleEstime->setChecked(true);
    }

     /* Grib */

    chk_askGribFolder->setCheckState(Settings::getSetting("askGribFolder",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    edt_gribFolder->setText(Settings::getSetting("edtGribFolder",appFolder.value("grib")).toString());

    chk_gribZoomOnLoad->setCheckState(Settings::getSetting("gribZoomOnLoad",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    //chk_autoGribDate->setCheckState(Settings::getSetting("autoGribDate",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    chk_externalMail->setCheckState(Settings::getSetting("sDocExternalMail",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    sailsDocPress->setCheckState(Settings::getSetting("sailsDocPress",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    forceWind->setChecked(Settings::getSetting("forceWind",0).toInt()==1);
    forcedTWS->setValue(Settings::getSetting("forcedTWS",0.0).toDouble());
    forcedTWD->setValue(Settings::getSetting("forcedTWD",0.0).toDouble());
    forceCurrents->setChecked(Settings::getSetting("forceCurrents",0).toInt()==1);
    forcedCS->setValue(Settings::getSetting("forcedCS",0.0).toDouble());
    forcedCD->setValue(Settings::getSetting("forcedCD",0.0).toDouble());

    /* GPS */
    chk_activateEmulation->setCheckState(
         Settings::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Settings::getSetting("serialName", "COM2").toString());
    spn_gpsDelay->setValue(Settings::getSetting("GPS_DELAY",30).toInt());

    /* advanced */

    chk_forceUserAgent->setCheckState(Settings::getSetting("forceUserAgent",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    userAgent->setText(Settings::getSetting("userAgent", "").toString());
    userAgent->setEnabled(Settings::getSetting("forceUserAgent",0).toInt()==1);
    defFontName->findText(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    defFontSize->setValue(8.25+Settings::getSetting("defaultFontSizeInc",0).toFloat());

    for(int i=0;i<NB_URL;i++)
        url_list->addItem(url_name[i]+": "+url_str[i]);
    url_list->setCurrentIndex(Settings::getSetting("vlm_url",0).toInt());
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

    saveWinGeometry->setCheckState(Settings::getSetting("saveMainWindowGeometry","1").toInt()==1?Qt::Checked:Qt::Unchecked);

    /*QString interpol_name[3] = { "TWSA", "selecive TWSA", "Hybride" };
    for(int i=0;i<3;i++)
        interpol_list->addItem(interpol_name[i]);
    interpol_list->setCurrentIndex(Settings::getSetting("interpolation",INTERPOL_DEFAULT).toInt());*/

}

void DialogParamVlm::initEstime(void) {
    estimeVal_dist->setValue(Settings::getSetting("estimeLen",100).toInt());
    estimeVal_time->setValue(Settings::getSetting("estimeTime",60).toInt());
    estimeVal_vac->setValue(Settings::getSetting("estimeVac",12).toInt());

    estimeVal_time->setEnabled(false);
    estimeVal_vac->setEnabled(false);
    estimeVal_dist->setEnabled(false);

    this->radioBtn_time->setEnabled(true);
    this->radioBtn_dist->setEnabled(true);
    radioBtn_vac->setEnabled(true);



    switch(Settings::getSetting("estimeType",0).toInt())
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
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(result == QDialog::Accepted)
    {
        /*drawing*/
        Settings::setSetting("opp_labelType",QString().setNum(opp_labelType->currentIndex()));
        Settings::setSetting("showFlag",this->chkPavillon->checkState()==Qt::Checked?"1":"0","showHideItem");
        Settings::setSetting("fusionStyle",this->chkFusion->isChecked()?1:0);
        Settings::setSetting("classicalVlmBoard",this->classicalBoard->isChecked()?1:0);
        QString skinName=edt_skinFile->text();
        if(!QFile(skinName).exists())
            skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
        Settings::setSetting("defaultSkin",skinName);
        int gdm=2;
        if(this->gribAuto->isChecked())
            gdm=0;
        else if (this->gribMono->isChecked())
            gdm=1;
        Settings::setSetting("gribDrawingMethod",gdm);
        Settings::setSetting("defaultFontName",this->defFontName->currentText());
        Settings::setSetting("defaultFontSizeInc",QString().setNum(this->defFontSize->value()-8.25));

        if(Settings::getSetting("mapsFolder",appFolder.value("maps")).toString() != mapsFolder->text())
        {
            QString mapDir = mapsFolder->text();
            QDir dir(mapDir);
            QDir appDir=QDir::currentPath();
            if(dir.rootPath()==appDir.rootPath())
                mapDir=appDir.relativeFilePath(mapDir);
            else
                mapDir=appDir.absoluteFilePath(mapDir);
            qWarning() << "Setting map folder to " << mapDir;
            Settings::setSetting("mapsFolder",mapDir);
            centralWidget->loadGshhs();
        }


        /* colors */

        Settings::setSetting("POI_Color",POI_color);
        Settings::setSetting("Marque_WP_Color",Marque_WP_color);
        Settings::setSetting("qtBoat_color",qtBoat_color);
        Settings::setSetting("qtBoat_sel_color",qtBoat_sel_color);
        Settings::setSetting("WP_Color",WP_color);
        Settings::setSetting("Balise_Color",Balise_color);

        /* Bateau */
        Settings::setSetting("estimeLen", QString().setNum(estimeVal_dist->value()));
        Settings::setSetting("estimeTime", QString().setNum(estimeVal_time->value()));
        Settings::setSetting("estimeVac", QString().setNum(estimeVal_vac->value()));


        if(radioBtn_time->isChecked())
            Settings::setSetting("estimeType","0");
        else if(radioBtn_vac->isChecked())
            Settings::setSetting("estimeType","1");
        else
            Settings::setSetting("estimeType","2");

        Settings::setSetting("centerOnSynch",chk_centerOnSynch->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("centerOnBoatChange",chk_centerOnBoatChange->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("askConfirmation",chk_askConfirm->checkState()==Qt::Checked?"1":"0");

        /* Trace */
        if(Settings::getSetting("trace_length",12).toInt()!=trace_length->value())
            emit resetTraceCache();
        Settings::setSetting("trace_length",QString().setNum(trace_length->value()));
        Settings::setSetting("speed_replay",QString().setNum(speedReplay->value()));
        Settings::setSetting("opp_trace",chk_oppTrace->checkState()==Qt::Checked?"1":"0");

        /* Compas */
        Settings::setSetting("showCompass",chk_showCompass->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("showPolar",chk_showPolar->checkState()==Qt::Checked?"1":"0");
        if(this->chk_scalePolarF->isChecked())
            Settings::setSetting("scalePolar","1");
        else
            Settings::setSetting("scalePolar","2");

        /* Route */

        Settings::setSetting("speedLossOnTackReal", QString().setNum(speedLossOnTackReal->value()));
        Settings::setSetting("speedLossOnTackVlm", QString().setNum(speedLossOnTackVlm->value()));
        Settings::setSetting("autoHideRoute",this->chkHideRoute->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("autoRemovePoiFromRoute",this->autoRemove->isChecked()?"1":"0");
        Settings::setSetting("autoFillPoiHeading",this->autoAt->isChecked()?"1":"0");
        Settings::setSetting("routeSortByName",this->routeSortByName->isChecked()?"1":"0");

        /* Grib */

        Settings::setSetting("askGribFolder",chk_askGribFolder->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("edtGribFolder",edt_gribFolder->text());
        Settings::setSetting("gribZoomOnLoad",chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        //Settings::setSetting("autoGribDate",chk_autoGribDate->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("sDocExternalMail",chk_externalMail->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("sailsDocPress",sailsDocPress->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("forceWind",forceWind->isChecked()?1:0);
        Settings::setSetting("forcedTWS",forcedTWS->value());
        Settings::setSetting("forcedTWD",forcedTWD->value());
        Settings::setSetting("forceCurrents",forceCurrents->isChecked()?1:0);
        Settings::setSetting("forcedCS",forcedCS->value());
        Settings::setSetting("forcedCD",forcedCD->value());
        if(centralWidget->getGrib()) centralWidget->getGrib()->forceParam();
        if(centralWidget->getGribCurrent()) centralWidget->getGribCurrent()->forceParam();

        /* advanced */
        Settings::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("serialName", serialName->text());
        Settings::setSetting("GPS_DELAY",spn_gpsDelay->value());

        Settings::setSetting("forceUserAgent",chk_forceUserAgent->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("userAgent",userAgent->text());

        int oldUrl = Settings::getSetting("vlm_url",0).toInt();
        Settings::setSetting("vlm_url",QString().setNum(url_list->currentIndex()));
        if(interpolTWSA->isChecked())
            INTERPOLATION_DEFAULT=INTERPOLATION_TWSA;
        else if(interpolSelect->isChecked())
            INTERPOLATION_DEFAULT=INTERPOLATION_SELECTIVE_TWSA;
        else
            INTERPOLATION_DEFAULT=INTERPOLATION_HYBRID;
        INTERPOLATION_DEFAULT=INTERPOLATION_HYBRID;
        Settings::setSetting("defaultInterpolation",INTERPOLATION_DEFAULT);
        //qWarning() << "old url=" << oldUrl << " new=" << url_list->currentIndex();
        if(oldUrl!=url_list->currentIndex())
            emit inetUpdated();

        Settings::setSetting("saveMainWindowGeometry",saveWinGeometry->checkState()==Qt::Checked?"1":"0");

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
    userAgent->setEnabled(newVal==Qt::Checked);
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
    chk_showCompass->setCheckState(Settings::getSetting("showCompass",0).toInt()==1?Qt::Checked:Qt::Unchecked);
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
