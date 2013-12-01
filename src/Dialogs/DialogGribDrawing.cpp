/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#include <QObject>
#include <QMessageBox>
#include <QComboBox>
#include <QDebug>
#include <QColorDialog>

#include "Terrain.h"
#include "mycentralwidget.h"
#include "DataManager.h"
#include "MapDataDrawer.h"
#include "Util.h"
#include "settings.h"
#include "ToolBar.h"
#include "MenuBar.h"

#include "DialogGribDrawing.h"

/*****************************
 * A faire:
 * => remettre le deltaDew
 * => parametre de level pour les Min/Max pression ?
 * => logique affichage des iso (utilisation de level ?)
 * => revoir les enchainements drawGeneric_DTC => 1D/2D_DTC => 1D/2D, en particulier sur l'utilisation de a struct
 * soit tt ds generic soit ds dtc spécifique, supprimer un niveau
 * => ajouter un affichage de fleche '1D' => interpolation 1D sur angle
 * => quelle interpolation utilisée pour les vagues ? 1D en force et 1D en direction ou 2D
 * => modifier le cartouche pour afficher le type de data selectionné
 */

DialogGribDrawing::DialogGribDrawing(QWidget *parent, myCentralWidget *centralWidget) : QDialog(parent) {
    this->centralWidget=centralWidget;
    this->terrain=centralWidget->getTerre();
    this->dataManager = centralWidget->get_dataManager();

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    setupUi(this);
    Util::setFontDialog(this);

    savDataMapMode = new Couple[DATA_MAX];
    savFrstArwMode = new Couple[DATA_MAX];
    savSecArwMode = new Couple[DATA_MAX];

    clear_savArray();
    connect(this,SIGNAL(hideDialog(bool)),centralWidget->get_menuBar()->acGrib_dialog,SLOT(setChecked(bool)));
    connect(this,SIGNAL(hideDialog(bool)),centralWidget->get_toolBar()->acGrib_dialog,SLOT(setChecked(bool)));
}

DialogGribDrawing::~DialogGribDrawing(void) {
    delete[] savDataMapMode;
    delete[] savFrstArwMode;
    delete[] savSecArwMode;
}

void DialogGribDrawing::showDialog(void) {
    if(init_state()) {
        show();
    }
}

bool DialogGribDrawing::init_state(void) {
    qWarning() << "Init state";
    /* init combo boxes according to loaded grib files */
    if(!dataManager->isOk()) {
        QMessageBox::information(centralWidget,tr("Grib drawing"),"No valid grib loaded, nothing to choose here");
        return false;
    }

    // loading comboLists
    init_comboList(dataManager->get_dataTypes(),bgDataType);
    init_comboList(dataManager->get_arrowTypesFst(),frstArwType);
    init_comboList(dataManager->get_arrowTypesSec(),secArwType);
    init_comboList(dataManager->get_dataTypes(),labelDataType);

    // uncomment in order to clear saved level associate with data type
    //clear_savArray();

    /* init current values */
    // BG part
    int curBGData=terrain->get_colorMapMode();
    int curBGLevelType=terrain->get_colorMapLevelType();
    int curBGLevelValue=terrain->get_colorMapLevelValue();
    qWarning() << "Color mode at start " << curBGData << " / " << curBGLevelType << " / " << curBGLevelValue;
    if(curBGData==DATA_NOTDEF) {
        chkSmooth->setEnabled(false);
        bgDataAlt->setEnabled(false);        
    }
    else {
        int idx=get_comboListItem(curBGData,bgDataType);
        bgDataType->blockSignals(true);
        bgDataType->setCurrentIndex(idx);
        bgDataType->blockSignals(false);
        chkSmooth->setEnabled(true);
        bgDataAlt->setEnabled(true);
        Couple res = update_levelCb(curBGData,bgDataAlt,SAVINFO_MAPDATA,curBGLevelType,curBGLevelValue);
        if(curBGLevelType!=res.a || curBGLevelValue!=res.b) {
            terrain->setColorMapMode(curBGData,res.a,res.b);
            curBGLevelType=res.a;
            curBGLevelValue=res.b;
        }
    }

    // Smooth
    chkSmooth->setChecked(Settings::getSetting("colorMapSmooth", true).toBool());

    //First arrow
    int curData=terrain->get_frstArwMode();
    int curLevelType=terrain->get_frstArwLevelType();
    int curLevelValue=terrain->get_frstArwLevelValue();
    qWarning() << "frstArw mode at start " << curData;
    if(curData==DATA_NOTDEF) {
        chkShowBarbule->setEnabled(false);
        frstArwAlt->setEnabled(false);
    }
    else {
        int idx=get_comboListItem(curData,frstArwType);
        frstArwType->blockSignals(true);
        frstArwType->setCurrentIndex(idx);
        frstArwType->blockSignals(false);
        chkShowBarbule->setEnabled(true);
        frstArwAlt->setEnabled(true);
        Couple res=update_levelCb(curData,frstArwAlt,SAVINFO_FRSTARW,curLevelType,curLevelValue);
        if(curLevelType!=res.a || curLevelValue!=res.b)
            terrain->setFrstArwMode(curData,res.a,res.b);
    }

    // Barbule
    chkShowBarbule->setChecked(Settings::getSetting("showBarbules", true).toBool());

    // Color
    QColor color=Settings::getSetting("frstArrowColor",QColor(Qt::white)).value<QColor>();
    updateBtnColor(btn_frstArrowColor,color);

    //Second arrow
    curData=terrain->get_secArwMode();
    curLevelType=terrain->get_secArwLevelType();
    curLevelValue=terrain->get_secArwLevelValue();
    qWarning() << "secArw mode at start " << curData;
    if(curData==DATA_NOTDEF) {
        secArwAlt->setEnabled(false);
    }
    else {
        int idx=get_comboListItem(curData,secArwType);
        secArwType->blockSignals(true);
        secArwType->setCurrentIndex(idx);
        secArwType->blockSignals(false);
        secArwAlt->setEnabled(true);
        Couple res=update_levelCb(curData,secArwAlt,SAVINFO_SECARW,curLevelType,curLevelValue);
        if(curLevelType!=res.a || curLevelValue!=res.b)
            terrain->setSecArwMode(curData,res.a,res.b);
    }

    // Color
    color=Settings::getSetting("secArrowColor",QColor(Qt::black)).value<QColor>();
    updateBtnColor(btn_secArrowColor,color);

    // Labels
    curData=terrain->get_labelMode();
    curLevelType=terrain->get_labelLevelType();
    curLevelValue=terrain->get_labelLevelValue();
    qWarning() << "label mode at start " << curData;
    if(curData==DATA_NOTDEF) {
        labelAlt->setEnabled(false);
    }
    else {
        int idx=get_comboListItem(curData,labelDataType);
        labelDataType->blockSignals(true);
        labelDataType->setCurrentIndex(idx);
        labelDataType->blockSignals(false);
        labelAlt->setEnabled(true);
        Couple res=update_levelCb(curData,labelAlt,SAVINFO_LABEL,curLevelType,curLevelValue);
        if(curLevelType!=res.a || curLevelValue!=res.b)
            terrain->setLabelMode(curData,res.a,res.b);
    }
    // Color
    color=Settings::getSetting("labelColor",QColor(Qt::black)).value<QColor>();
    updateBtnColor(btn_labelColor,color);


    // IsoBar
    bool showIso=Settings::getSetting("showIsobars", false).toBool();
    chkShowIsoBar->setChecked(showIso);
    int idx = isoBarSpacing->findText(Settings::getSetting("isobarsStep", "2").toString());
    if(idx!=-1)
        isoBarSpacing->setCurrentIndex(idx);
    isoBarSpacing->setEnabled(showIso);
    chkShowIsoBarLabel->setChecked(Settings::getSetting("showIsobarsLabels", false).toBool());
    chkShowIsoBarLabel->setEnabled(showIso);
    chkShowIsoBarMinMax->setChecked(Settings::getSetting("showPressureMinMax", false).toBool());


    // IsoTherm0
    showIso=Settings::getSetting("showIsotherms0", false).toBool();
    chkShowIsoTherm->setChecked(showIso);
    idx = isoThermSpacing->findText(Settings::getSetting("isotherms0Step", "50").toString());
    if(idx!=-1)
        isoThermSpacing->setCurrentIndex(idx);
    isoThermSpacing->setEnabled(showIso);
    chkShowIsoThermLabel->setChecked(Settings::getSetting("showIsotherms0Labels", false).toBool());
    chkShowIsoThermLabel->setEnabled(showIso);

    qWarning() << "Init state .......... Done";

    return true;
}

Couple DialogGribDrawing::update_levelCb(int data,QComboBox * cb,int infoType, int levelType, int levelValue) {
    cb->blockSignals(true);
    cb->clear();

    QMap<int,QList<int>*> * levelList = dataManager->get_levelList(data);

    int idx_default=-1;
    bool exactDefault=false;
    int idx_prev=-1;
    int idx_cur=-1;
    int idx_new=-1;
    Couple defaultVal=dataManager->get_defaultLevel(data);
    Couple prevVal;
    switch(infoType) {
        case SAVINFO_MAPDATA:
            prevVal=savDataMapMode[data];
            break;
        case SAVINFO_FRSTARW:
            prevVal=savFrstArwMode[data];
            break;
        case SAVINFO_SECARW:
            prevVal=savSecArwMode[data];
            break;
    }

    qWarning() << "[update_levelCb] cur=" << levelType << "/" << levelValue
               << " - prev=" << prevVal.a << "/" << prevVal.b
               << " - default=" << defaultVal.a << "/" << defaultVal.b;

    if(levelList) {
        QMap<int,QStringList> * levelTypes=dataManager->get_levelTypes();
        QMapIterator<int,QList<int>*> j(*levelList);
        while (j.hasNext()) {
            j.next();
            QList<int>* lst = j.value();
            if(!lst) continue;

            for(int i=0;i<lst->count();++i) {
                if(!levelTypes->contains(j.key())) continue;
                QString str=levelTypes->value(j.key()).at(0);
                int lvType=j.key();
                int lvVal=lst->at(i);


                if(!levelTypes->value(j.key()).at(1).isEmpty()) {
                    // level value has a meaning
                    str += " - " + QString().setNum(lst->at(i)) + " " + levelTypes->value(j.key()).at(1);
                }
                cb->addItem(str,lvType);
                int idx=cb->count()-1;
                cb->setItemData(idx,lvVal,Qt::UserRole+1);

                /* trying to find idx */
                // curent val ?
                if(levelType!=DATA_LV_NOTDEF) {
                    if(levelType==lvType && levelValue==lvVal)
                        idx_cur=idx;
                }
                // default value
                if(lvType == defaultVal.a) {
                    if(lvVal == defaultVal.b) {
                        idx_default=idx;
                        exactDefault=true;
                    }
                    else {
                        if(!exactDefault)
                            idx_default=idx;
                    }

                }

                if(lvType == prevVal.a && lvVal == prevVal.b)
                    idx_prev=idx;

                qWarning() << "Adding item: " << str << " - " << j.key() << "," << lst->at(i);
            }
        }
    }

    if(idx_cur!=-1) {
        qWarning() << "using Cur val";
        idx_new=idx_cur;
    }
    else if(idx_prev!=-1) {
        qWarning() << "using prev val";
        idx_new=idx_prev;
    }
    else if(idx_default!=-1) {
        qWarning() << "using default";
        idx_new=idx_default;
    }

    if(idx_new==-1) {
        qWarning() << "using first level";
        idx_new=0;
    }

    qWarning() << "new idx=" << idx_new << "(size=" << cb->count() << ")";
    cb->setCurrentIndex(idx_new);
    cb->blockSignals(false);

    Couple res;
    res.a=cb->itemData(idx_new,Qt::UserRole).toInt();
    res.b=cb->itemData(idx_new,Qt::UserRole+1).toInt();

    /* setting back prev info */
    switch(infoType) {
        case SAVINFO_MAPDATA:
            savDataMapMode[data]=res;
            break;
        case SAVINFO_FRSTARW:
            savFrstArwMode[data]=res;
            break;
        case SAVINFO_SECARW:
            savSecArwMode[data]=res;
            break;
    }

    return res;
}

void DialogGribDrawing::copy_cb(QComboBox * from, QComboBox * to) {
    to->blockSignals(true);
    to->clear();
    for(int i=0;i<from->count();++i) {
        to->addItem(from->itemText(i));
        to->setItemData(i,from->itemData(i,Qt::UserRole),Qt::UserRole);
        to->setItemData(i,from->itemData(i,Qt::UserRole+1),Qt::UserRole+1);
    }
    to->setCurrentIndex(from->currentIndex());
    to->blockSignals(false);
}

int DialogGribDrawing::get_comboListItem(int data,QComboBox * cb) {
    return cb->findData(data);
}

int DialogGribDrawing::get_comboListItem(int data1,int data2,QComboBox * cb) {
    qWarning() << "get_comboListItem for: " << data1 << " - " << data2;
    bool found=false;
    int i;
    for(i=0;i<cb->count();++i) {
        qWarning() << i << ": " << cb->itemData(i,Qt::UserRole).toInt() << "," << cb->itemData(i,Qt::UserRole+1).toInt();
        if(cb->itemData(i,Qt::UserRole).toInt()==data1 && cb->itemData(i,Qt::UserRole+1).toInt()==data2) {
            found=true;
            break;
        }
    }
    if(!found)
        i=-1;
    return i;
}

void DialogGribDrawing::init_comboList(QMap<int,QStringList> * map,QComboBox * cb) {
    cb->blockSignals(true);
    cb->clear();
    QMapIterator<int,QStringList> i(*map);
    while (i.hasNext()) {
        i.next();
        if(i.key()==DATA_NOTDEF)
            cb->addItem(i.value().first(),i.key());
        else if(dataManager->hasDataType(i.key()))
            cb->addItem(i.value().first(),i.key());
    }
    cb->blockSignals(false);
}

void DialogGribDrawing::slot_finished() {
    hide();
    emit hideDialog(false);
}

void DialogGribDrawing::slot_bgDataType(int idx) {
    int newType = bgDataType->itemData(idx).toInt();

    qWarning() << "New type: " << newType;

    if(newType==DATA_NOTDEF) {
        bgDataAlt->blockSignals(true);
        bgDataAlt->setCurrentIndex(0);
        bgDataAlt->blockSignals(false);
        chkSmooth->setEnabled(false);
        bgDataAlt->setEnabled(false);
        terrain->setColorMapMode(DATA_NOTDEF,DATA_LV_NOTDEF,0);

    }
    else {
        Couple res=update_levelCb(newType,bgDataAlt,SAVINFO_MAPDATA);
        chkSmooth->setEnabled(true);
        bgDataAlt->setEnabled(true);
        terrain->setColorMapMode(newType,res.a,res.b);
    }
}

void DialogGribDrawing::slot_bgDataAlt(int idx) {
    int newLevelType=bgDataAlt->itemData(idx,Qt::UserRole).toInt();
    int newLevelValue=bgDataAlt->itemData(idx,Qt::UserRole+1).toInt();
    int curDataType=terrain->get_colorMapMode();
    qWarning() << "Cb alt for data changed: " << newLevelType << ", " << newLevelValue;
    /* saving info to prev array */
    savDataMapMode[curDataType]=Couple(newLevelType,newLevelValue);
    terrain->setColorMapMode(curDataType,newLevelType,newLevelValue);
}

void DialogGribDrawing::slot_frstArwType(int idx) {
    int newType = frstArwType->itemData(idx).toInt();
    qWarning() << "New type (frst arw): " << newType;    
    if(newType==DATA_NOTDEF) {        
        frstArwAlt->blockSignals(true);
        frstArwAlt->setCurrentIndex(0);
        frstArwAlt->blockSignals(false);
        chkShowBarbule->setEnabled(false);
        frstArwAlt->setEnabled(false);
        terrain->setFrstArwMode(DATA_NOTDEF,DATA_LV_NOTDEF,0);
    }
    else {
        Couple res=update_levelCb(newType,frstArwAlt,SAVINFO_FRSTARW);
        chkShowBarbule->setEnabled(true);
        frstArwAlt->setEnabled(true);
        terrain->setFrstArwMode(newType,res.a,res.b);
    }
}

void DialogGribDrawing::slot_frstArwAlt(int idx) {
    int newLevelType=frstArwAlt->itemData(idx,Qt::UserRole).toInt();
    int newLevelValue=frstArwAlt->itemData(idx,Qt::UserRole+1).toInt();
    int curDataMode=terrain->get_frstArwMode();
    qWarning() << "Cb alt for frstArw changed: " << newLevelType << ", " << newLevelValue;
    /* saving info in prev array */
    savFrstArwMode[curDataMode]=Couple(newLevelType,newLevelValue);
    terrain->setFrstArwMode(curDataMode,newLevelType,newLevelValue);
}

void DialogGribDrawing::slot_secArwType(int idx) {
    int newType = secArwType->itemData(idx).toInt();
    qWarning() << "New type (sec arw): " << newType;

    if(newType==DATA_NOTDEF) {
        secArwAlt->blockSignals(true);
        secArwAlt->setCurrentIndex(0);
        secArwAlt->blockSignals(false);
        secArwAlt->setEnabled(false);
        terrain->setSecArwMode(DATA_NOTDEF,DATA_LV_NOTDEF,0);
    }
    else {
        Couple res=update_levelCb(newType,secArwAlt,SAVINFO_SECARW);
        secArwAlt->setEnabled(true);
        terrain->setSecArwMode(newType,res.a,res.b);
    }
}

void DialogGribDrawing::slot_secArwAlt(int idx) {
    int newLevelType=secArwAlt->itemData(idx,Qt::UserRole).toInt();
    int newLevelValue=secArwAlt->itemData(idx,Qt::UserRole+1).toInt();
    int curDataMode=terrain->get_secArwMode();
    qWarning() << "Cb alt for secArw changed: " << newLevelType << ", " << newLevelValue;
    /* saving info in prev array */
    savSecArwMode[curDataMode]=Couple(newLevelType,newLevelValue);
    terrain->setSecArwMode(curDataMode,newLevelType,newLevelValue);
}

void DialogGribDrawing::slot_labelDataType(int idx) {
    int newType = labelDataType->itemData(idx).toInt();
    qWarning() << "New type (label): " << newType;

    if(newType==DATA_NOTDEF) {
        labelAlt->blockSignals(true);
        labelAlt->setCurrentIndex(0);
        labelAlt->blockSignals(false);
        labelAlt->setEnabled(false);
        terrain->setLabelMode(DATA_NOTDEF,DATA_LV_NOTDEF,0);
    }
    else {
        Couple res=update_levelCb(newType,labelAlt,SAVINFO_LABEL);
        labelAlt->setEnabled(true);
        terrain->setLabelMode(newType,res.a,res.b);
    }
}

void DialogGribDrawing::slot_labelAlt(int idx) {
    int newLevelType=labelAlt->itemData(idx,Qt::UserRole).toInt();
    int newLevelValue=labelAlt->itemData(idx,Qt::UserRole+1).toInt();
    int curDataMode=terrain->get_labelMode();
    qWarning() << "Cb alt for label changed: " << newLevelType << ", " << newLevelValue;
    /* saving info in prev array */
    savLabelMode[curDataMode]=Couple(newLevelType,newLevelValue);
    terrain->setLabelMode(curDataMode,newLevelType,newLevelValue);
}

/*
void DialogGribDrawing::slot_showTemp(bool st) {
    terrain->show_temperatureLabels(st);
}

void DialogGribDrawing::slot_tempAlt(int idx) {

}
*/

void DialogGribDrawing::slot_smooth(bool st) {
    terrain->setColorMapSmooth(st);
}

void DialogGribDrawing::slot_showBarbule(bool st) {
    terrain->setBarbules(st);
}

void DialogGribDrawing::slot_frstArrowColor(void) {
    chg_color(btn_frstArrowColor,"frstArrowColor",QColor(Qt::white));
}

void DialogGribDrawing::slot_secArrowColor(void) {
    chg_color(btn_secArrowColor,"secArrowColor",QColor(Qt::black));
}

void DialogGribDrawing::slot_labelColor(void) {
    chg_color(btn_labelColor,"labelColor",QColor(Qt::black));
}

void DialogGribDrawing::chg_color(QPushButton * btn,QString setting,QColor defaultColor) {
    QColor color =Settings::getSetting(setting,defaultColor).value<QColor>();
    color = QColorDialog::getColor(color, this);

    if(color.isValid()) {
        Settings::setSetting(setting,color);
        updateBtnColor(btn,color);
    }
    terrain->redrawGrib();
}

void DialogGribDrawing::updateBtnColor(QPushButton * btn,QColor color) {
    QString styleSheet=QString().sprintf("background-color: rgb(%d, %d, %d);",color.red(),color.green(),color.blue());
    btn->setStyleSheet(styleSheet);
}

void DialogGribDrawing::slot_showIsoBar(bool st) {
    terrain->setDrawIsobars(st);
    isoBarSpacing->setEnabled(st);
    chkShowIsoBarLabel->setEnabled(st);
}

void DialogGribDrawing::slot_showIsoTherm(bool st) {
    terrain->setDrawIsotherms0(st);
    isoThermSpacing->setEnabled(st);
    chkShowIsoThermLabel->setEnabled(st);
}

void DialogGribDrawing::slot_isoBarSpacing(int val) {
    terrain->setIsobarsStep(isoBarSpacing->itemText(val).toDouble());
}

void DialogGribDrawing::slot_isoThermSpacing(int val) {
    terrain->setIsotherms0Step(isoThermSpacing->itemText(val).toDouble());
}

void DialogGribDrawing::slot_isoBarShowLabel(bool st) {
    terrain->setDrawIsobarsLabels(st);
}

void DialogGribDrawing::slot_isoThermShowLabel(bool st) {
    terrain->setDrawIsotherms0Labels(st);
}

void DialogGribDrawing::slot_isoBarShowMinMax(bool st) {
    terrain->setPressureMinMax(st);
}

void DialogGribDrawing::clear_savArray(void) {
    for(int i=0;i<DATA_MAX;++i) {
        savDataMapMode[i].a=DATA_LV_NOTDEF;
        savDataMapMode[i].b=0;

        savFrstArwMode[i].a=DATA_LV_NOTDEF;
        savFrstArwMode[i].b=0;

        savSecArwMode[i].a=DATA_LV_NOTDEF;
        savSecArwMode[i].b=0;
    }
}
