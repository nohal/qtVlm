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

#include "paramVLM.h"
#include "Util.h"

paramVLM::paramVLM(QWidget * parent) : QDialog(parent)
{
    setupUi(this);

    /* Drawing / affichage */   
    chk_gribZoomOnLoad->setCheckState(Util::getSetting("gribZoomOnLoad",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    opp_labelType->addItem(tr("Login"));
    opp_labelType->addItem(tr("Nom"));
    opp_labelType->addItem(tr("Numéro"));
    opp_labelType->setCurrentIndex(Util::getSetting("opp_labelType",0).toInt());

    chk_autoGribDate->setCheckState(Util::getSetting("autoGribDate",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Colors */

    setColor(Util::getSetting("POI_Color",QColor(Qt::black).name()).toString(),0);
    setColor(Util::getSetting("Marque_WP_Color",QColor(Qt::red).name()).toString(),4);
    setColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString(),1);
    setColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString(),2);
    setColor(Util::getSetting("WP_Color",QColor(Qt::darkYellow).name()).toString(),5);
    setColor(Util::getSetting("Balise_Color",QColor(Qt::darkMagenta).name()).toString(),6);

    /* Estime */
    estimeVal_dist->setValue(Util::getSetting("estimeLen",100).toInt());
    estimeVal_time->setValue(Util::getSetting("estimeTime",60).toInt());
    estimeVal_vac->setValue(Util::getSetting("estimeVac",10).toInt());

    estimeVal_time->setEnabled(false);
    estimeVal_vac->setEnabled(false);
    estimeVal_dist->setEnabled(false);

    switch(Util::getSetting("estimeType",0).toInt())
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

    /* Trace */
    for(int i=5;i<=61;i+=5)
        trace_step->addItem(QString().setNum(i));
    trace_step->setCurrentIndex(Util::getSetting("trace_step",60/5-1).toInt());
    trace_length->setValue(Util::getSetting("trace_length",12).toInt());
    chk_oppTrace->setCheckState(Util::getSetting("opp_trace","1").toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Compas */
    chk_showCompass->setCheckState(Util::getSetting("showCompass",1).toInt()==1?Qt::Checked:Qt::Unchecked);

     /* Fichier repertoire */

    chk_askGribFolder->setCheckState(Util::getSetting("askGribFolder",1).toInt()==1?Qt::Checked:Qt::Unchecked);
    edt_gribFolder->setText(Util::getSetting("edtGribFolder","grib/").toString());

    /* advanced */
    chk_activateEmulation->setCheckState(
         Util::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Util::getSetting("serialName", "COM2").toString());

    chk_forceUserAgent->setCheckState(Util::getSetting("forceUserAgent",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    userAgent->setText(Util::getSetting("userAgent", "").toString());
    userAgent->setEnabled(Util::getSetting("forceUserAgent",0).toInt()==1);

    for(int i=0;i<NB_URL;i++)
        url_list->addItem(url_name[i]+": "+url_str[i]);
    url_list->setCurrentIndex(Util::getSetting("vlm_url",0).toInt());

    saveWinGeometry->setCheckState(Util::getSetting("saveMainWindowGeometry","1").toInt()==1?Qt::Checked:Qt::Unchecked);


}

void paramVLM::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /*drawing*/        
        Util::setSetting("gribZoomOnLoad",chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("opp_labelType",QString().setNum(opp_labelType->currentIndex()));
        Util::setSetting("autoGribDate",chk_autoGribDate->checkState()==Qt::Checked?"1":"0");

        /* colors */

        Util::setSetting("POI_Color",POI_color);
        Util::setSetting("Marque_WP_Color",Marque_WP_color);
        Util::setSetting("qtBoat_color",qtBoat_color);
        Util::setSetting("qtBoat_sel_color",qtBoat_sel_color);
        Util::setSetting("WP_Color",WP_color);
        Util::setSetting("Balise_Color",Balise_color);

        /* Estime */
        Util::setSetting("estimeLen", QString().setNum(estimeVal_dist->value()));
        Util::setSetting("estimeTime", QString().setNum(estimeVal_time->value()));
        Util::setSetting("estimeVac", QString().setNum(estimeVal_vac->value()));

        if(radioBtn_time->isChecked())
            Util::setSetting("estimeType","0");
        else if(radioBtn_vac->isChecked())
            Util::setSetting("estimeType","1");
        else
            Util::setSetting("estimeType","2");

        /* Trace */

        Util::setSetting("trace_step",QString().setNum(trace_step->currentIndex()));
        Util::setSetting("trace_length",QString().setNum(trace_length->value()));
        Util::setSetting("opp_trace",chk_oppTrace->checkState()==Qt::Checked?"1":"0");

        /* Compas */
        Util::setSetting("showCompass",chk_showCompass->checkState()==Qt::Checked?"1":"0");

        /* Fichier repertoire */

        Util::setSetting("askGribFolder",chk_askGribFolder->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("edtGribFolder",edt_gribFolder->text());

        /* advanced */
        Util::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("serialName", serialName->text());
        Util::setSetting("forceUserAgent",chk_forceUserAgent->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("userAgent",userAgent->text());

        int oldUrl = Util::getSetting("vlm_url",0).toInt();
        Util::setSetting("vlm_url",QString().setNum(url_list->currentIndex()));
        //qWarning() << "old url=" << oldUrl << " new=" << url_list->currentIndex();
        if(oldUrl!=url_list->currentIndex())
            emit inetUpdated();

        Util::setSetting("saveMainWindowGeometry",saveWinGeometry->checkState()==Qt::Checked?"1":"0");        

        emit paramVLMChanged();
    }
    QDialog::done(result);
}

void paramVLM::forceUserAgent_changed(int newVal)
{
    userAgent->setEnabled(newVal==Qt::Checked);
}

void paramVLM::changeColor_POI(void)
{
    changeColor(0);
}

void paramVLM::changeColor_qtBoat(void)
{
    changeColor(1);
}

void paramVLM::changeColor_qtBoat_sel(void)
{
    changeColor(2);
}

void paramVLM::changeColor_Marque_WP(void)
{
    changeColor(4);
}

void paramVLM::changeColor_WP(void)
{
    changeColor(5);
}

void paramVLM::changeColor_Balise(void)
{
    changeColor(6);
}

void paramVLM::changeColor(int type)
{
    QColor color = QColorDialog::getColor(getColor(type), this);
    if(color.isValid())
        setColor(color.name (),type);
}

QColor paramVLM::getColor(int type)
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

void paramVLM::setColor(QString color,int type)
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

void paramVLM::radioBtn_time_toggle(bool val)
{
    estimeVal_time->setEnabled(val);
}

void paramVLM::radioBtn_vac_toggle(bool val)
{
    estimeVal_vac->setEnabled(val);
}

void paramVLM::radioBtn_dist_toggle(bool val)
{
    estimeVal_dist->setEnabled(val);
}

void paramVLM::doBtn_browseGrib(void)
{
     QString dir = QFileDialog::getExistingDirectory(this, tr("Repertoire Grib"),
                                                 edt_gribFolder->text());
     if(dir!="")
         edt_gribFolder->setText(dir);
}
