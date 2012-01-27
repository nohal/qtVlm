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

#include <QtGui>

#include "DialogParamVlm.h"
#include "settings.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "Util.h"

DialogParamVlm::DialogParamVlm(MainWindow * main,myCentralWidget * parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);

    connect(this,SIGNAL(paramVLMChanged()),main,SLOT(slotParamChanged()));
    connect(this, SIGNAL(inetUpdated()), main, SLOT(slotInetUpdated()));

    /* Drawing / affichage */
    opp_labelType->addItem(tr("Pseudo"));
    opp_labelType->addItem(tr("Nom"));
    opp_labelType->addItem(tr("Numero"));
    opp_labelType->setCurrentIndex(Settings::getSetting("opp_labelType",0).toInt());

    this->chkPavillon->setCheckState(Settings::getSetting("showFlag",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->classicalButtons->setChecked(Settings::getSetting("classicalButtons",0).toInt()==1);

    /* Colors */

    setColor(Settings::getSetting("POI_Color",QColor(Qt::black).name()).toString(),0);
    setColor(Settings::getSetting("Marque_WP_Color",QColor(Qt::red).name()).toString(),4);
    setColor(Settings::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString(),1);
    setColor(Settings::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString(),2);
    setColor(Settings::getSetting("WP_Color",QColor(Qt::darkYellow).name()).toString(),5);
    setColor(Settings::getSetting("Balise_Color",QColor(Qt::darkMagenta).name()).toString(),6);

    /* Bateau */
    estimeVal_dist->setValue(Settings::getSetting("estimeLen",100).toInt());
    estimeVal_time->setValue(Settings::getSetting("estimeTime",60).toInt());
    estimeVal_vac->setValue(Settings::getSetting("estimeVac",12).toInt());


    estimeVal_time->setEnabled(false);
    estimeVal_vac->setEnabled(false);
    estimeVal_dist->setEnabled(false);

    switch(Settings::getSetting("estimeType",0).toInt())
    {
        case 0:
            radioBtn_time->setChecked(true);
            break;
        case 1:
            radioBtn_vac->setChecked(true);
            break;
        case 2:
            radioBtn_dist->setChecked(true);
            break;
    }

    chk_centerOnSynch->setCheckState(Settings::getSetting("centerOnSynch","1").toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_centerOnBoatChange->setCheckState(Settings::getSetting("centerOnBoatChange","1").toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_askConfirm->setCheckState(Settings::getSetting("askConfirmation","0").toInt()==1?Qt::Checked:Qt::Unchecked);

    /*Route*/
    this->speedLossOnTackReal->setValue(Settings::getSetting("speedLossOnTackReal","100").toInt());
    this->speedLossOnTackVlm->setValue(Settings::getSetting("speedLossOnTackVlm","100").toInt());
    this->chkHideRoute->setCheckState(Settings::getSetting("autoHideRoute",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoRemove->setCheckState(Settings::getSetting("autoRemovePoiFromRoute",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->autoAt->setCheckState(Settings::getSetting("autoFillPoiHeading",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Trace */
    trace_length->setValue(Settings::getSetting("trace_length",12).toInt());
    speedReplay->setValue(Settings::getSetting("speed_replay",20).toInt());
    chk_oppTrace->setCheckState(Settings::getSetting("opp_trace","1").toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Compas */
    chk_showCompass->setCheckState(Settings::getSetting("showCompass",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_showPolar->setCheckState(Settings::getSetting("showPolar",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    this->radioBtn_time->setEnabled(true);
    this->radioBtn_dist->setEnabled(true);
    if(Settings::getSetting("scalePolar",0).toInt()==1)
    {
        chk_scalePolarF->setChecked(true);
        polVac->setEnabled(false);
    }
    else if(Settings::getSetting("scalePolar",0).toInt()==2)
    {
        chk_scaleEstime->setChecked(true);
        polVac->setEnabled(false);
        this->radioBtn_time->setEnabled(true);
        this->radioBtn_dist->setEnabled(false);
    }
    else
    {
        chk_scalePolarR->setChecked(true);
        polVac->setEnabled(true);
    }
    polVac->setValue(Settings::getSetting("polVac",12).toInt());

     /* Grib */

    chk_askGribFolder->setCheckState(Settings::getSetting("askGribFolder",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    edt_gribFolder->setText(Settings::getSetting("edtGribFolder","grib/").toString());

    chk_gribZoomOnLoad->setCheckState(Settings::getSetting("gribZoomOnLoad",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_autoGribDate->setCheckState(Settings::getSetting("autoGribDate",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    chk_externalMail->setCheckState(Settings::getSetting("sDocExternalMail",1).toInt()==1?Qt::Checked:Qt::Unchecked);

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

void DialogParamVlm::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /*drawing*/
        Settings::setSetting("opp_labelType",QString().setNum(opp_labelType->currentIndex()));
        Settings::setSetting("showFlag",this->chkPavillon->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("classicalButtons",this->classicalButtons->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("defaultFontName",this->defFontName->currentText());
        Settings::setSetting("defaultFontSizeInc",QString().setNum(this->defFontSize->value()-8.25));


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

        Settings::setSetting("trace_length",QString().setNum(trace_length->value()));
        Settings::setSetting("speed_replay",QString().setNum(speedReplay->value()));
        Settings::setSetting("opp_trace",chk_oppTrace->checkState()==Qt::Checked?"1":"0");

        /* Compas */
        Settings::setSetting("showCompass",chk_showCompass->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("showPolar",chk_showPolar->checkState()==Qt::Checked?"1":"0");
        if(this->chk_scalePolarR->isChecked())
            Settings::setSetting("scalePolar","0");
        else if(this->chk_scalePolarF->isChecked())
            Settings::setSetting("scalePolar","1");
        else
            Settings::setSetting("scalePolar","2");
        Settings::setSetting("polVac",QString().setNum(polVac->value()));

        /* Route */

        Settings::setSetting("speedLossOnTackReal", QString().setNum(speedLossOnTackReal->value()));
        Settings::setSetting("speedLossOnTackVlm", QString().setNum(speedLossOnTackVlm->value()));
        Settings::setSetting("autoHideRoute",this->chkHideRoute->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("autoRemovePoiFromRoute",this->autoRemove->isChecked()?"1":"0");
        Settings::setSetting("autoFillPoiHeading",this->autoAt->isChecked()?"1":"0");

        /* Grib */

        Settings::setSetting("askGribFolder",chk_askGribFolder->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("edtGribFolder",edt_gribFolder->text());
        Settings::setSetting("gribZoomOnLoad",chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("autoGribDate",chk_autoGribDate->checkState()==Qt::Checked?"1":"0");
        Settings::setSetting("sDocExternalMail",chk_externalMail->checkState()==Qt::Checked?"1":"0");

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
    }
    QDialog::done(result);
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
void DialogParamVlm::changeParam()
{
    chk_showCompass->setCheckState(Settings::getSetting("showCompass",1).toInt()==1?Qt::Checked:Qt::Unchecked);
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
