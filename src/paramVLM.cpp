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
    estimeLen->setValue(Util::getSetting("estimeLen",100).toInt());
    chk_gribZoomOnLoad->setCheckState(Util::getSetting("gribZoomOnLoad",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    opp_labelType->addItem(tr("Login"));
    opp_labelType->addItem(tr("Nom"));
    opp_labelType->addItem(tr("Numéro"));
    opp_labelType->setCurrentIndex(Util::getSetting("opp_labelType",0).toInt());

    chk_manualClipping->setCheckState(Util::getSetting("manualClipping",0).toInt()==1?Qt::Checked:Qt::Unchecked);

    /* Colors */

    setColor(Util::getSetting("POI_Color",QColor(Qt::black).name()).toString(),0);
    setColor(Util::getSetting("POI_WP_Color",QColor(Qt::red).name()).toString(),4);
    setColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString(),1);
    setColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString(),2);
    //setColor(Util::getSetting("opp_color",QColor(Qt::green).name()).toString(),3);

    /* Trace */
    for(int i=5;i<=61;i+=5)
        trace_step->addItem(QString().setNum(i));
    trace_step->setCurrentIndex(Util::getSetting("trace_step",60/5-1).toInt());
    trace_length->setValue(Util::getSetting("trace_length",12).toInt());
    chk_oppTrace->setCheckState(Util::getSetting("opp_trace","1").toInt()==1?Qt::Checked:Qt::Unchecked);

    /* advanced */
    chk_activateEmulation->setCheckState(
         Util::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Util::getSetting("serialName", "COM2").toString());

    chk_forceUserAgent->setCheckState(Util::getSetting("forceUserAgent",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    userAgent->setText(Util::getSetting("userAgent", "").toString());
    userAgent->setEnabled(Util::getSetting("forceUserAgent",0).toInt()==1);

    /*
    url_list->addItem(tr("Standard: ")+"www.virtual-loup-de-mer.org");
    url_list->addItem(tr("Secours: ")+"virtual-loup-de-mer.org");
    url_list->addItem(tr("Test: ")+"testing.virtual-loup-de-mer.org");
    */
    for(int i=0;i<NB_URL;i++)
        url_list->addItem(url_name[i]+": "+url_str[i]);
    url_list->setCurrentIndex(Util::getSetting("vlm_url",0).toInt());
}

void paramVLM::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /*drawing*/
        Util::setSetting("estimeLen", QString().setNum(estimeLen->value()));
        Util::setSetting("gribZoomOnLoad",chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("opp_labelType",QString().setNum(opp_labelType->currentIndex()));
        Util::setSetting("manualClipping",chk_manualClipping->checkState()==Qt::Checked?"1":"0");

        /* colors */

        Util::setSetting("POI_Color",POI_color);
        Util::setSetting("POI_WP_Color",POI_WP_color);
        Util::setSetting("qtBoat_color",qtBoat_color);
        Util::setSetting("qtBoat_sel_color",qtBoat_sel_color);
        //Util::setSetting("opp_color",opp_color);

        /* Trace */

        Util::setSetting("trace_step",QString().setNum(trace_step->currentIndex()));
        Util::setSetting("trace_length",QString().setNum(trace_length->value()));
        Util::setSetting("opp_trace",chk_oppTrace->checkState()==Qt::Checked?"1":"0");

        /* advanced */
        Util::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("serialName", serialName->text());
        Util::setSetting("forceUserAgent",chk_forceUserAgent->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("userAgent",userAgent->text());

        int oldUrl = Util::getSetting("vlm_url",0).toInt();
        Util::setSetting("vlm_url",QString().setNum(url_list->currentIndex()));
        qWarning() << "old url=" << oldUrl << " new=" << url_list->currentIndex();
        if(oldUrl!=url_list->currentIndex())
            emit inetUpdated();

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

/*void paramVLM::changeColor_opp(void)
{
    changeColor(3);
}*/

void paramVLM::changeColor_POI_WP(void)
{
    changeColor(4);
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
            return QColor(POI_WP_color);
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
/*        case 3:
            opp_frame->setStyleSheet(style);
            opp_color=color;
            break;*/
        case 4:
            POI_WP_frame->setStyleSheet(style);
            POI_WP_color=color;
            break;
    }
}

