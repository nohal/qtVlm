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

#include <QDebug>

#include "DialogRace.h"
#include "Util.h"

#include "opponentBoat.h"
#include "mycentralwidget.h"
#include "boatVLM.h"
#include "MainWindow.h"
#include "parser.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include "Orthodromie.h"
#include <QImage>
#include <QDesktopWidget>
#include "settings.h"
#include <QDebug>
#include <QInputDialog>

#define RACE_NO_REQUEST     0
#define RACE_LIST_BOAT      1
#define RESULT_LIST_BOAT    2
#define FLAG_REQUEST        3
#define RACE_LIST_REAL      4

DialogRace::DialogRace(MainWindow * main,myCentralWidget * parent, inetConnexion * inet) :
        QDialog(parent),
        inetClient(inet)
{
    this->main=main;
    this->parent=parent;
    this->somethingChanged=false;
    inetClient::setName("RaceDialog");
    needAuth=true;
    setupUi(this);
    Util::setFontDialog(this);

    model= new QStandardItemModel(this);
    model->setColumnCount(10);
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Sel"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Rang"));
    switch (Settings::getSetting("opp_labelType",0).toInt())
    {
        case SHOW_PSEUDO:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Pseudo"));
            break;
        case SHOW_NAME:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Nom"));
            break;
        case SHOW_IDU:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Numero"));
            break;
    }
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Pavillon"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Loch 1h"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Loch 3h"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Loch 24h"));
    model->setHeaderData(8,Qt::Horizontal,QObject::tr("DNM"));
    model->setHeaderData(9,Qt::Horizontal,QObject::tr("Dist 1er"));
    model->setSortRole(Qt::UserRole);
    ranking->setModel(model);
    connect(this,SIGNAL(updateOpponent()),main,SLOT(slotUpdateOpponent()));



    modelResult= new QStandardItemModel(this);
    modelResult->setColumnCount(7);
    modelResult->setHeaderData(0,Qt::Horizontal,QObject::tr("Rang"));
    switch (Settings::getSetting("opp_labelType",0).toInt())
    {
        case SHOW_PSEUDO:
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Pseudo"));
            break;
        case SHOW_NAME:
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Nom"));
            break;
        case SHOW_IDU:
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Numero"));
            break;
    }
    modelResult->setHeaderData(2,Qt::Horizontal,QObject::tr("Pavillon"));
    modelResult->setHeaderData(3,Qt::Horizontal,QObject::tr("Loch"));
    modelResult->setHeaderData(4,Qt::Horizontal,QObject::tr("Date depart"));
    modelResult->setHeaderData(5,Qt::Horizontal,QObject::tr("Duree"));
    modelResult->setHeaderData(6,Qt::Horizontal,QObject::tr("Ecart"));
    modelResult->setSortRole(Qt::UserRole);
    arrived->setModel(modelResult);
    arrived->header()->setAlternatingRowColors(true);
    arrived->header()->setDefaultAlignment(Qt::AlignCenter|Qt::AlignVCenter);;
    ranking->header()->setAlternatingRowColors(true);
    ranking->header()->setDefaultAlignment(Qt::AlignCenter|Qt::AlignVCenter);;
    //ranking->viewOptions().decorationAlignment=Qt::AlignCenter|Qt::AlignVCenter;
    chooser_raceList->setInsertPolicy(QComboBox::InsertAlphabetically);
    inputTraceColor =new InputLineParams(1,QColor(Qt::red),1.6,  QColor(Qt::red),this,0.1,5);
    noSailZoneGroup->layout()->addWidget(inputTraceColor);
    connect(model,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(itemChanged(QStandardItem*)));
    waitBox = new QMessageBox(QMessageBox::Information,tr("Parametrage des courses"),
                              tr("Chargement des courses et des bateaux"));
    waitBox->setStandardButtons(QMessageBox::NoButton);
    connect(this->filterReal,SIGNAL(clicked()),this,SLOT(slotFilterReal()));

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(doSynch()));
}

DialogRace::~DialogRace()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(model)
        delete model;
    if(modelResult)
        delete modelResult;
}
QString DialogRace::getAuthLogin(bool * ok)
{
    Player * cur_player=parent->getPlayer();
    if(cur_player)
        return cur_player->getAuthLogin(ok);
    else
    {
        if(ok)
            *ok=true;
        return QString();
    }
}

QString DialogRace::getAuthPass(bool * ok)
{
    Player * cur_player=parent->getPlayer();
    if(cur_player)
        return cur_player->getAuthPass(ok);
    else
    {
        if(ok)
            *ok=true;
        return QString();
    }
}
void DialogRace::initList(QList<boatVLM*> & boat_list_ptr,QList<raceData*> & race_list_ptr)
{
    this->boat_list = & boat_list_ptr;
    this->race_list = & race_list_ptr;

    numRace = -1;
    initDone = false;
    this->somethingChanged=false;

    clear();

    /* determines the list of race */
    for(int i=0;i<boat_list->size();i++)
    {
        bool found = false;

        //qWarning() << boat_list->at(i)->getBoatName() << " " << boat_list->at(i)->getStatus() << " " << boat_list->at(i)->getRaceId();

        /* bateau inactif ou pas en course */
        if(!boat_list->at(i)->getStatus() || boat_list->at(i)->getRaceId() == "0")
            continue;

        /* on cherche si la course existe deja ds la liste */
        for(int j=0;j<param_list.size();j++)
            if(param_list[j]->id == boat_list->at(i)->getRaceId())
            {
                found=true;
                break;
            }
        /* Elle n'existe pas => on cr�e un nv raceParam */
        //qWarning()<<"race found:"<<found;
        if(!found)
        {
            raceParam * ptr = new raceParam();
            ptr->id=boat_list->at(i)->getRaceId();
            ptr->name=boat_list->at(i)->getRaceName();
            ptr->boats.clear();
            ptr->latNSZ=-60;
            ptr->colorNSZ=Qt::black;
            ptr->widthNSZ=2;
            ptr->displayNSZ=false;
            ptr->showWhat=SHOW_MY_LIST;
            ptr->showReal=false;
            ptr->hasReal=false;
            ptr->realFilter.clear();
            param_list.append(ptr);
        }
    }
    /* init combo box with list of races */
    chooser_raceList->clear();

    for(int j=0;j<param_list.size();j++)
        chooser_raceList->addItem(param_list[j]->name,param_list[j]->id);
    nbRace->setText(QString().setNum(param_list.size()));
    if(!hasInet() || hasRequest())
    {
        qWarning("raceDialog bad state in inet");
        return;
    }

    /* init index of search : currentRace*/
    currentRace=-1;
    waitBox->show();
    if(param_list.isEmpty())
    {
        this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        this->button10distance->setEnabled(false);
        this->button10First->setEnabled(false);
        this->button10ranking->setEnabled(false);
        this->buttonMySelection->setEnabled(false);
        this->buttonNone->setEnabled(false);
        this->buttonReal->setEnabled(false);
        this->filterReal->setEnabled(false);
    }
    getNextRace();
}
void DialogRace::NSZToggle(bool b)
{
    param_list[numRace]->displayNSZ=b;
    param_list[numRace]->colorNSZ=inputTraceColor->getLineColor();
    param_list[numRace]->widthNSZ=inputTraceColor->getLineWidth();
}
void DialogRace::myListToggle(bool b)
{
    if(b)
        param_list[numRace]->showWhat=SHOW_MY_LIST;
}

void DialogRace::tenFirstToggle(bool b)
{
    if(b)
        param_list[numRace]->showWhat=SHOW_TEN_FIRST;
}
void DialogRace::tenFirstDistToggle(bool b)
{
    if(b)
        param_list[numRace]->showWhat=SHOW_TEN_CLOSEST_DISTANCE;
}

void DialogRace::tenFirstRankToggle(bool b)
{
    if(b)
        param_list[numRace]->showWhat=SHOW_TEN_CLOSEST_RANKING;
}
void DialogRace::noneToggle(bool b)
{
    if(b)
        param_list[numRace]->showWhat=SHOW_NONE;
}
void DialogRace::showRealToggle(bool)
{
    param_list[numRace]->showReal=buttonReal->isChecked();
    this->filterReal->setEnabled(buttonReal->isChecked());
}

void DialogRace::itemChanged(QStandardItem * item)
{
    model->blockSignals(true);
    boatParam *boatPtr=reinterpret_cast<struct boatParam *>(qvariant_cast<void*>(item->data(Qt::UserRole+1)));
    if(item->checkState()==Qt::Checked )
    {
        if(nbSelected>=RACE_MAX_BOAT)
        {
            QMessageBox::warning(this,tr("Parametrage des courses"),
                                  tr("Nombre maximum de concurrent depasse")+" ("
                                  +QString().setNum(RACE_MAX_BOAT)+")");
            item->setCheckState(Qt::Unchecked);
        }
        else
        {
            boatPtr->selected=true;
//            QFont myfont=item->font();
//            myfont.setBold(true);
//            for(int n=1;n<model->columnCount();n++)
//                model->item(item->row(),n)->setData(myfont,Qt::FontRole);

            nbSelected++;
        }
    }
    else
    {
        boatPtr->selected=false;
//        QFont myfont=item->font();
//        myfont.setBold(false);
//        for(int n=1;n<model->columnCount();n++)
//            model->item(item->row(),n)->setData(myfont,Qt::FontRole);
        nbSelected--;
    }
    this->boatSelect->setText(QString().setNum(nbSelected)+"/"+QString().setNum(RACE_MAX_BOAT));
    item->setData(item->checkState(),Qt::UserRole);
    model->blockSignals(false);
}

void DialogRace::getNextRace()
{
    currentRace++;
    if(currentRace>=param_list.size())
    {
        /* finished */
        initDone = true;
        numRace=-1;
        chgRace(0);
        waitBox->hide();
        this->exec();
        return;
    }
    waitBox->show();
    /* let's find this race in the param from boatAcc.dat*/
    currentParam.clear();

    currentShowWhat=SHOW_MY_LIST;
    currentDisplayNSZ=false;
    currentLatNSZ=-60;
    currentColorNSZ=Qt::black;
    currentWidthNSZ=2;
    currentShowReal=false;
    currentHasReal=false;
    currentFilterReal.clear();

    for(int i=0;i<race_list->size();i++)
        if(race_list->at(i)->idrace==param_list[currentRace]->id)
        {
            currentParam=race_list->at(i)->oppList.split(";");
            currentDisplayNSZ=race_list->at(i)->displayNSZ;
            currentLatNSZ=race_list->at(i)->latNSZ;
            currentWidthNSZ=race_list->at(i)->widthNSZ;
            currentColorNSZ=race_list->at(i)->colorNSZ;
            currentShowWhat=race_list->at(i)->showWhat;
            currentShowReal=race_list->at(i)->showReal;
            currentFilterReal=race_list->at(i)->realFilter;
            currentHasReal=race_list->at(i)->hasReal;
            break;
        }

    QString page;
    QTextStream(&page)
            << "/ws/raceinfo/ranking.php?idr="
            << param_list[currentRace]->id;
    clearCurrentRequest();
    inetGet(RACE_LIST_BOAT,page);
}

void DialogRace::requestFinished (QByteArray res_byte)
{
    struct boatParam * ptr;

    QJson::Parser parser;
    bool ok;
    QVariantMap result;
    if(getCurrentRequest()!=FLAG_REQUEST)
    {
        result = parser.parse (res_byte, &ok).toMap();
        if (!ok) {
            qWarning() << "Error parsing json data " << res_byte;
            qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        }

        if(!checkWSResult(res_byte,"raceDialog->RACE_LIST_BOAT",main))
        {
            getNextRace();
            return;
        }
    }
    switch(getCurrentRequest())
    {
        case RACE_LIST_BOAT:
        {
            QVariantMap ranking = result["ranking"].toMap();
            QMapIterator<QString,QVariant> it(ranking);
            QString tt;
            Orthodromie orth(0,0,0,0);
            while (it.hasNext())
            {
                QVariantMap data=it.next().value().toMap();
                if(data["rank"].toInt()==1)
                    orth.setStartPoint(data["longitude"].toFloat(),data["latitude"].toFloat());
                ptr = new boatParam();
                ptr->pseudo=data["boatpseudo"].toString();
                ptr->name=data["boatname"].toString();
                ptr->user_id=data["idusers"].toString();
                ptr->selected=currentParam.contains(ptr->user_id);
                ptr->rank=data["rank"].toString();
                ptr->last1h=tt.sprintf("%.2f",data["last1h"].toFloat());
                ptr->last3h=tt.sprintf("%.2f",data["last3h"].toFloat());
                ptr->last24h=tt.sprintf("%.2f",data["last24h"].toFloat());
                ptr->dnm=data["nwp"].toString()+"->"+tt.sprintf("%10.2f",data["dnm"].toFloat());
                if(data["deptime"].toString()=="-1")
                    ptr->statusVLM=tr("Au mouillage");
                else
                    ptr->statusVLM=data["status"].toString();
                ptr->longitude=data["longitude"].toFloat();
                ptr->latitude=data["latitude"].toFloat();
                ptr->fromFirst=tt.sprintf("%.2f",orth.getDistance());
                ptr->pavillon=data["country"].toString();
                param_list[currentRace]->boats.append(ptr);
            }
            for(int ii=0;ii<param_list[currentRace]->boats.count();ii++)
            {
                orth.setEndPoint(param_list[currentRace]->boats[ii]->longitude,param_list[currentRace]->boats[ii]->latitude);
                param_list[currentRace]->boats[ii]->fromFirst=tt.sprintf("%.2f",orth.getDistance());
            }
            param_list[currentRace]->displayNSZ=currentDisplayNSZ;
            param_list[currentRace]->latNSZ=currentLatNSZ;
            param_list[currentRace]->widthNSZ=currentWidthNSZ;
            param_list[currentRace]->colorNSZ=currentColorNSZ;
            param_list[currentRace]->showWhat=currentShowWhat;
            param_list[currentRace]->showReal=currentShowReal;
            param_list[currentRace]->realFilter=currentFilterReal;
            param_list[currentRace]->hasReal=currentHasReal;
            QString page;
            QTextStream(&page)
                    << "/ws/raceinfo/results.php?idr="
                    << param_list[currentRace]->id;
            clearCurrentRequest();
            inetGet(RESULT_LIST_BOAT,page);
            break;
        }
        case RESULT_LIST_BOAT:
        {
            QVariantMap ranking = result["results"].toMap();
            QMapIterator<QString,QVariant> it(ranking);
            QString tt;
            int firstTime=0;
            while (it.hasNext())
            {
                QVariantMap data=it.next().value().toMap();
                if(data["rank"].toInt()==1)
                    firstTime=data["duration"].toInt();
                ptr = new boatParam();
                ptr->pseudo=data["boatpseudo"].toString();
                ptr->name=data["boatname"].toString();
                ptr->user_id=data["idusers"].toString();
                ptr->rank=data["rank"].toString();
                ptr->last1h=tt.sprintf("%.2f",data["loch"].toFloat());
                ptr->last3h=data["duration"].toString();
                ptr->last24h=data["deptime"].toString();
                ptr->pavillon=data["country"].toString();
                param_list[currentRace]->arrived.append(ptr);
            }
            for(int ii=0;ii<param_list[currentRace]->arrived.count();ii++)
                param_list[currentRace]->arrived[ii]->fromFirst=QString().setNum(qAbs(param_list[currentRace]->arrived.at(ii)->last3h.toInt()-firstTime));
            jj=0;
            QString page;
            QTextStream(&page)
                    << "/ws/raceinfo/reals.php?idr="
                    << param_list[currentRace]->id;
            clearCurrentRequest();
            inetGet(RACE_LIST_REAL,page);
            break;
        }
        case RACE_LIST_REAL:
        {
            if(result["nb_boats"].toInt()==0)
            {
                param_list[currentRace]->showReal=false;
                param_list[currentRace]->hasReal=false;
                param_list[currentRace]->realFilter.clear();
            }
            else
            {
                param_list[currentRace]->hasReal=true;
            }
            getMissingFlags();
            break;
        }
        case FLAG_REQUEST:
        {
            QImage img;
            if(img.loadFromData(res_byte))
            {
                img.save(appFolder.value("flags")+imgFileName);
                //qWarning()<<"saving flag"<<imgFileName;
            }
            ++jj;
            getMissingFlags();
            break;
        }
    }
}
void DialogRace::getMissingFlags()
{
    QFile file;
    for (;jj<param_list[currentRace]->boats.count();jj++)
    {
        imgFileName=param_list[currentRace]->boats.at(jj)->pavillon+".png";
        file.setFileName(appFolder.value("flags")+imgFileName);
        if(!file.exists())
        {
            //qWarning()<<"requesting flag"<<imgFileName;
            QString page;
            QTextStream(&page)
                    << "/cache/flags/"
                    <<imgFileName;
            clearCurrentRequest();
            inetGet(FLAG_REQUEST,page);
            return;
        }
    }
    getNextRace();
}
void DialogRace::clear(void)
{
    for(int i=0;i<param_list.size();i++)
    {
        for(int j=0;j<param_list[i]->boats.size();j++)
            delete param_list[i]->boats[j];
        param_list[i]->boats.clear();
        delete param_list[i];
    }
    param_list.clear();
}

void DialogRace::done(int result)
{
    if(result == QDialog::Accepted)
    {
        param_list[numRace]->colorNSZ=inputTraceColor->getLineColor();
        param_list[numRace]->widthNSZ=inputTraceColor->getLineWidth();
        saveData(true); /* really saving, not only applying*/
    }
    else if(somethingChanged)
    {
        emit readRace();
        emit updateOpponent();
    }

    QDialog::done(result);
}

void DialogRace::doSynch(void) /* apply only */
{
    somethingChanged=true;
    int savNum=numRace;
    saveData(false);
    chgRace(savNum);
}

void DialogRace::saveData(bool save)
{
    QStringList boats;
    struct raceData * ptr;
    /* removing all races */
    for(int i=0;i<race_list->size();i++)
        delete race_list->at(i);
    race_list->clear();


    /* saving races */
    for(int i=0;i<param_list.size();i++)
    {
        boats.clear();
        for(int j=0;j<param_list[i]->boats.count();j++)
        {
            if(!param_list[i]->boats[j]->selected) continue;
            boats.append(param_list[i]->boats[j]->user_id);
        }
        ptr = new raceData();
        ptr->idrace=param_list[i]->id;
        if(!boats.isEmpty())
            ptr->oppList=boats.join(";");
        else
            ptr->oppList=boats.join(";");
        ptr->colorNSZ=param_list[i]->colorNSZ;
        ptr->widthNSZ=param_list[i]->widthNSZ;
        ptr->displayNSZ=param_list[i]->displayNSZ;
        ptr->latNSZ=param_list[i]->latNSZ;
        ptr->showWhat=param_list[i]->showWhat;
        ptr->showReal=param_list[i]->showReal;
        ptr->realFilter=param_list[i]->realFilter;
        ptr->hasReal=param_list[i]->hasReal;
        race_list->append(ptr);
    }

    if(save)
    {
        //qWarning() << "saving races: " << race_list->size();
        emit writeBoat();
    }

    emit updateOpponent();
}

void DialogRace::chgRace(int id)
{
    if(!initDone)
        return;
    //int debug=0;
    if(numRace!=-1)
    {
        /* changement du select des boat */
        param_list[numRace]->colorNSZ=inputTraceColor->getLineColor();
        param_list[numRace]->widthNSZ=inputTraceColor->getLineWidth();
        param_list[numRace]->displayNSZ=displayNSZ->isChecked();
        if(nsNSZ->currentText()=="N")
            param_list[numRace]->latNSZ=latNSZ->value();
        else
            param_list[numRace]->latNSZ=-latNSZ->value();
        if(this->buttonMySelection->isChecked())
            param_list[numRace]->showWhat=SHOW_MY_LIST;
        if(this->button10First->isChecked())
            param_list[numRace]->showWhat=SHOW_TEN_FIRST;
        if(this->button10distance->isChecked())
            param_list[numRace]->showWhat=SHOW_TEN_CLOSEST_DISTANCE;
        if(this->button10ranking->isChecked())
            param_list[numRace]->showWhat=SHOW_TEN_CLOSEST_RANKING;
        if(this->buttonNone->isChecked())
            param_list[numRace]->showWhat=SHOW_NONE;
        param_list[numRace]->showReal=this->buttonReal->isChecked();
    }

    /* find race data */
    if(id < 0 || id >= chooser_raceList->count())
    {
        //qWarning() << "chgRace: Bad id :" << id;
        return;
    }

    QString idRace = chooser_raceList->itemData(id).toString();

    for(numRace=0;numRace<param_list.size();numRace++)
        if(param_list[numRace]->id == idRace)
            break;
    if(numRace==param_list.size())
    {
        qWarning() << "chgRace: id not found";
        return;
    }

    model->blockSignals(true);
    int labelType=Settings::getSetting("opp_labelType",0).toInt();
    switch (labelType)
    {
        case SHOW_PSEUDO:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Pseudo"));
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Pseudo"));
            break;
        case SHOW_NAME:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Nom"));
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Nom"));
            break;
        case SHOW_IDU:
            model->setHeaderData(2,Qt::Horizontal,QObject::tr("Numero"));
            modelResult->setHeaderData(1,Qt::Horizontal,QObject::tr("Numero"));
            break;
    }
    model->removeRows(0,model->rowCount());
    nbSelected=0;
    QString str;
//    QFont myFont=ranking->font();
//    myFont.setBold(true);
//    QFontMetrics fm(myFont);
    for(int i=0;i<param_list[numRace]->boats.size();i++)
    {
        QList<QStandardItem*> items;
        items.append(new QStandardItem());
        items[0]->setData(QVariant(QMetaType::VoidStar, &param_list[numRace]->boats[i]),Qt::UserRole+1);
        items[0]->setData(Qt::Unchecked, Qt::CheckStateRole);
        items[0]->setCheckable(true);
//        items[0]->setData(fm.height(),Qt::SizeHintRole);
        if(param_list[numRace]->boats[i]->selected && nbSelected<=RACE_MAX_BOAT)
        {
            items[0]->setCheckState(Qt::Checked);
            nbSelected++;
        }
        items[0]->setData(items[0]->checkState(),Qt::UserRole);
        if(main->isBoat(param_list[numRace]->boats[i]->user_id))
            items[0]->setEnabled(false);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->rank));
        items[1]->setData(param_list[numRace]->boats[i]->rank.toInt(),Qt::UserRole);
        switch(labelType)
        {
            case SHOW_PSEUDO:
                str=param_list[numRace]->boats[i]->pseudo;
                break;
            case SHOW_NAME:
                str=param_list[numRace]->boats[i]->name;
                break;
            case SHOW_IDU:
                str=param_list[numRace]->boats[i]->user_id;
                break;
        }

        items.append(new QStandardItem(str));
        items[2]->setData(str.toLower(),Qt::UserRole);
        QImage flag;
        if(flag.load(appFolder.value("flags")+param_list[numRace]->boats[i]->pavillon+".png"))
        {
            items.append(new QStandardItem());
            items[3]->setData(QPixmap::fromImage(flag),Qt::DecorationRole);
        }
        else
            items.append(new QStandardItem(param_list[numRace]->boats[i]->pavillon));
        items[3]->setData(param_list[numRace]->boats[i]->pavillon.toLower(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->statusVLM));
        items[4]->setData(param_list[numRace]->boats[i]->statusVLM.toLower(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->last1h));
        items[5]->setData(param_list[numRace]->boats[i]->last1h.toFloat(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->last3h));
        items[6]->setData(param_list[numRace]->boats[i]->last3h.toFloat(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->last24h));
        items[7]->setData(param_list[numRace]->boats[i]->last24h.toFloat(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->dnm));
        items[8]->setData(param_list[numRace]->boats[i]->dnm,Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->boats[i]->fromFirst));
        items[9]->setData(param_list[numRace]->boats[i]->fromFirst.toFloat(),Qt::UserRole);
        if(!items[0]->isEnabled())
        {
            for(int it=0;it<items.count();it++)
                items[it]->setData(Qt::red,Qt::ForegroundRole);
        }
//        if(items[0]->checkState()==Qt::Checked)
//        {
//            QFont font=items[1]->font();
//            font.setBold(true);
//            for(int it=0;it<items.count();it++)
//                items[it]->setData(font,Qt::FontRole);
//        }
        model->appendRow(items);

    }
    for (int c=0;c<model->columnCount();c++)
    {
        for (int r=0;r<model->rowCount();r++)
        {
            model->item(r,c)->setEditable(false);
            if(c>4)
                model->item(r,c)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                model->item(r,c)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        }
        ranking->resizeColumnToContents(c);
    }
    model->blockSignals(false);
    ranking->sortByColumn(1,Qt::AscendingOrder);
    modelResult->blockSignals(true);
    modelResult->removeRows(0,modelResult->rowCount());
    QTime myTime;
    QDateTime myDate;
    for(int i=0;i<param_list[numRace]->arrived.size();i++)
    {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(param_list[numRace]->arrived[i]->rank));
        items[0]->setData(param_list[numRace]->arrived[i]->rank.toInt(),Qt::UserRole);
        switch(labelType)
        {
            case SHOW_PSEUDO:
                str=param_list[numRace]->arrived[i]->pseudo;
                break;
            case SHOW_NAME:
                str=param_list[numRace]->arrived[i]->name;
                break;
            case SHOW_IDU:
                str=param_list[numRace]->arrived[i]->user_id;
                break;
        }

        items.append(new QStandardItem(str));
        items[1]->setData(str.toLower(),Qt::UserRole);
        QImage flag;
        if(flag.load(appFolder.value("flags")+param_list[numRace]->arrived[i]->pavillon+".png"))
        {
            items.append(new QStandardItem());
            items[2]->setData(QPixmap::fromImage(flag),Qt::DecorationRole);
        }
        else
            items.append(new QStandardItem(param_list[numRace]->arrived[i]->pavillon));
        items[2]->setData(param_list[numRace]->arrived[i]->pavillon.toLower(),Qt::UserRole);
        items.append(new QStandardItem(param_list[numRace]->arrived[i]->last1h));
        items[3]->setData(param_list[numRace]->arrived[i]->last1h.toFloat(),Qt::UserRole);
        myDate=myDate.fromTime_t(param_list[numRace]->arrived[i]->last24h.toInt()).toUTC();
        items.append(new QStandardItem(myDate.toString(tr("dd MMM yyyy hh:mm:ss"))));
        items[4]->setData(param_list[numRace]->arrived[i]->last3h.toInt(),Qt::UserRole);
        myTime.setHMS(0,0,0,0);
        int temp=param_list[numRace]->arrived[i]->last3h.toInt();
        int jour=0;
        while (temp>=24*60*60)
        {
            temp=temp-24*60*60;
            jour++;
        }
        myTime=myTime.addSecs(temp);
        items.append(new QStandardItem(QString().setNum(jour)+" jours "+myTime.toString("hh'h'mm'min'ss'secs'")));
        items[5]->setData(param_list[numRace]->arrived[i]->last3h.toInt(),Qt::UserRole);
        myTime.setHMS(0,0,0,0);
        temp=param_list[numRace]->arrived[i]->fromFirst.toInt();
        jour=0;
        while (temp>=24*60*60)
        {
            temp=temp-24*60*60;
            jour++;
        }
        myTime=myTime.addSecs(temp);
        items.append(new QStandardItem(QString().setNum(jour)+" jours "+myTime.toString("hh'h'mm'min'ss'secs'")));
        items[6]->setData(param_list[numRace]->arrived[i]->fromFirst.toInt(),Qt::UserRole);
        modelResult->appendRow(items);
    }
    for (int c=0;c<modelResult->columnCount();c++)
    {
        for (int r=0;r<modelResult->rowCount();r++)
        {
            modelResult->item(r,c)->setEditable(false);
            if(c==3 || c==5 || c==6)
                modelResult->item(r,c)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                modelResult->item(r,c)->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
        }
        arrived->resizeColumnToContents(c);
    }
    modelResult->blockSignals(false);
    arrived->sortByColumn(0,Qt::AscendingOrder);
    this->boatSelect->setText(QString().setNum(nbSelected)+"/"+QString().setNum(RACE_MAX_BOAT));
    this->boatRacing->setText(QString().setNum(param_list[numRace]->boats.size()));
    displayNSZ->setChecked(param_list[numRace]->displayNSZ);
    latNSZ->setValue(qAbs(param_list[numRace]->latNSZ));
    if(param_list[numRace]->latNSZ<0)
        nsNSZ->setCurrentIndex(1);
    else
        nsNSZ->setCurrentIndex(0);
    if(param_list[numRace]->showWhat==SHOW_MY_LIST)
        this->buttonMySelection->setChecked(true);
    if(param_list[numRace]->showWhat==SHOW_TEN_FIRST)
        this->button10First->setChecked(true);
    if(param_list[numRace]->showWhat==SHOW_TEN_CLOSEST_DISTANCE)
        this->button10distance->setChecked(true);
    if(param_list[numRace]->showWhat==SHOW_TEN_CLOSEST_RANKING)
        this->button10ranking->setChecked(true);
    this->buttonReal->setChecked(param_list[numRace]->showReal);
    this->buttonReal->setEnabled(param_list[numRace]->hasReal);
    this->filterReal->setEnabled(param_list[numRace]->hasReal);

    noSailZoneGroup->layout()->removeWidget(inputTraceColor);
    delete inputTraceColor;
    inputTraceColor =new InputLineParams(param_list[numRace]->widthNSZ,param_list[numRace]->colorNSZ,2,  QColor(Qt::black),this,0.1,5);
    noSailZoneGroup->layout()->addWidget( inputTraceColor);
    latNSZ->setEnabled(param_list[numRace]->displayNSZ);
    nsNSZ->setEnabled(param_list[numRace]->displayNSZ);
    inputTraceColor->setEnabled(param_list[numRace]->displayNSZ);
    this->tab->setFocus();
    this->boatTotal->setText(QString().setNum(model->rowCount()+modelResult->rowCount()));
    this->boatArrived->setText(QString().setNum(modelResult->rowCount()));
}

void DialogRace::on_displayNSZ_clicked()
{
    latNSZ->setEnabled(displayNSZ->isChecked());
    nsNSZ->setEnabled(displayNSZ->isChecked());
    inputTraceColor->setEnabled(displayNSZ->isChecked());
}
void DialogRace::slotFilterReal()
{
    bool ok;
    QString filter = QInputDialog::getText(this, tr("Filtrer les reels"),
                                           tr("Ids (separes par ';':"),QLineEdit::Normal, param_list[numRace]->realFilter,&ok);
    if(!ok) return;
    param_list[numRace]->realFilter=filter;
}
