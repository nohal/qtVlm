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
    
    /* Colors */

    setColor(Util::getSetting("POI_Color",QColor(Qt::black).name()).toString(),0);
    setColor(Util::getSetting("POI_WP_Color",QColor(Qt::red).name()).toString(),4);
    setColor(Util::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString(),1);
    setColor(Util::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString(),2);
    setColor(Util::getSetting("opp_color",QColor(Qt::green).name()).toString(),3);

    /* advanced */
    chk_activateEmulation->setCheckState(
         Util::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Util::getSetting("serialName", "COM2").toString());    
    
    chk_forceUserAgent->setCheckState(Util::getSetting("forceUserAgent",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    userAgent->setText(Util::getSetting("userAgent", "").toString());
    userAgent->setEnabled(Util::getSetting("forceUserAgent",0).toInt()==1);
    
}

void paramVLM::done(int result)
{
    if(result == QDialog::Accepted)
    {
        /*drawing*/
        Util::setSetting("estimeLen", QString().setNum(estimeLen->value()));
        Util::setSetting("gribZoomOnLoad",chk_gribZoomOnLoad->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("opp_labelType",QString().setNum(opp_labelType->currentIndex()));
        /* colors */

        Util::setSetting("POI_Color",POI_color);
        Util::setSetting("POI_WP_Color",POI_WP_color);
        Util::setSetting("qtBoat_color",qtBoat_color);        
        Util::setSetting("qtBoat_sel_color",qtBoat_sel_color);
        Util::setSetting("opp_color",opp_color);

        /* advanced */
        Util::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("serialName", serialName->text());        
        Util::setSetting("forceUserAgent",chk_forceUserAgent->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("userAgent",userAgent->text());
        emit paramVLMChanged();
    }
    QDialog::done(result);
}

void paramVLM::forceUserAgent_changed(int newVal)
{
    qWarning("New val %d",newVal);
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

void paramVLM::changeColor_opp(void)
{
    changeColor(3);
}

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
        case 3:
            return QColor(opp_color);
            break;
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
        case 3:
            opp_frame->setStyleSheet(style);
            opp_color=color;
            break;
        case 4:
            POI_WP_frame->setStyleSheet(style);
            POI_WP_color=color;
            break;            
    }
}

