#include <QDateTime>

#include "DialogRouteComparator.h"
#include "settings.h"
#include "mycentralwidget.h"
#include "route.h"
#include "boat.h"
#include "Orthodromie.h"
#include "POI.h"
#include "Util.h"

DialogRouteComparator::DialogRouteComparator(myCentralWidget *parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    this->mcw=parent;
    connect(this->closeButton,SIGNAL(clicked()),this,SLOT(close()));
    model= new QStandardItemModel(this);
    model->setColumnCount(22);
    int c=0;
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Color"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Name"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Start\nDate"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("ETA"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Duration"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Ortho\nDistance"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Sailed\nDistance"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Avg BS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Max BS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Min BS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Avg TWS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Max TWS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Min TWS"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Nb Tacks\nand Gybes"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Beating\ntime"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Downwind\ntime"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Reaching\ntime"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Motor\ntime"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Night\nnavigation"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Under rain\nnavigation"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Max waves\nheight"));
    model->setHeaderData(c++,Qt::Horizontal,QObject::tr("Max combined\nwaves height"));
    model->setSortRole(Qt::UserRole);
    routesTable->header()->setAlternatingRowColors(true);
    routesTable->header()->setDefaultAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    routesTable->setWordWrap(true);
    this->routesTable->setModel(model);
    routesCombo->addItem(tr("Add a route to comparator"));
    for (int n=0;n<parent->getRouteList().size();++n)
    {
        ROUTE * route=parent->getRouteList().at(n);
        if(route->getBoat()==parent->getSelectedBoat() && !route->getPoiList().isEmpty())
        {
            if(route->get_forceComparator())
            {
                this->insertRoute(n);
                route->set_forceComparator(false);
            }
            else
            {
                QPixmap iconI(20,10);
                iconI.fill(route->getColor());
                QIcon icon(iconI);
                routesCombo->addItem(icon," "+route->getName(),n);
            }
        }
    }
    routesCombo->setCurrentIndex(0);
    connect(routesCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(slot_insertRoute(int)));
    connect(routesTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(slot_contextMenu(QPoint)));
    routesTable->setContextMenuPolicy(Qt::CustomContextMenu);
}
void DialogRouteComparator::slot_contextMenu(QPoint P)
{
    item = model->itemFromIndex(routesTable->indexAt(P));
    if(!item) return;
    QMenu *menu = new QMenu;
    int currentRouteIndex=item->data().toInt();
    menu->addAction(tr("Remove route")+" "+mcw->getRouteList().at(currentRouteIndex)->getName()+" "+tr("from comparator"), this, SLOT(slot_removeRoute()));
    menu->addAction(tr("Delete route")+" "+mcw->getRouteList().at(currentRouteIndex)->getName(), this, SLOT(slot_deleteRoute()));
    menu->exec(QCursor::pos());
    delete menu;
}
void DialogRouteComparator::slot_deleteRoute()
{
    ROUTE * route=mcw->getRouteList().at(item->data().toInt());
    if(mcw->myDeleteRoute(route))
    {
        model->removeRow(item->row());
        routesTable->clearSelection();
    }
}
void DialogRouteComparator::slot_removeRoute()
{
    ROUTE * route=mcw->getRouteList().at(item->data().toInt());
    QPixmap iconI(20,10);
    iconI.fill(route->getColor());
    QIcon icon(iconI);
    routesCombo->addItem(icon," "+route->getName(),item->data().toInt());
    model->removeRow(item->row());
    routesTable->clearSelection();
}
void DialogRouteComparator::slot_insertRoute(int i)
{
    int n=routesCombo->itemData(i).toInt();
    routesCombo->blockSignals(true);
    routesCombo->removeItem(i);
    routesCombo->setCurrentIndex(0);
    routesCombo->blockSignals(false);
    insertRoute(n);
}
void DialogRouteComparator::insertRoute(const int &n)
{
    ROUTE * route=mcw->getRouteList().at(n);
    routeStats stats=route->getStats();
    QList<QStandardItem*> items;
    int x=0;
    QPixmap iconI(20,10);
    iconI.fill(route->getColor());
    items.append(new QStandardItem());
    items[x]->setData(iconI,Qt::DecorationRole);
    items[x]->setData(route->getColor().toRgb(),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
    items.append(new QStandardItem(route->getName()));
    items[x]->setData(route->getName().toLower(),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
    items.append(new QStandardItem(route->getStartTime().toString("dd MMM-hh:mm")));
    items[x]->setData(route->getStartTime().toTime_t(),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
    if(route->getHas_eta())
    {
        items.append(new QStandardItem(QDateTime().fromTime_t(route->getEta()).toUTC().toString("dd MMM-hh:mm")));
        items[x]->setData((int)(route->getEta()),Qt::UserRole);
        items[x]->setData(n,Qt::UserRole+1);
        if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
        items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
        double days=(route->getEta()-route->getStartDate())/86400.0000;
        if(qRound(days)>days)
            days=qRound(days)-1;
        else
            days=qRound(days);
        double hours=(route->getEta()-route->getStartDate()-days*86400)/3600.0000;
        if(qRound(hours)>hours)
            hours=qRound(hours)-1;
        else
            hours=qRound(hours);
        double mins=qRound((route->getEta()-route->getStartDate()-days*86400-hours*3600)/60.0000);
        items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                       QString::number((int)mins)+" "+tr("minutes")));
        items[x]->setData((int)(route->getEta()-route->getStartDate()),Qt::UserRole);
        items[x]->setData(n,Qt::UserRole+1);
        if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
        items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
    }
    else
    {
        items.append(new QStandardItem("N/A"));
        items[x]->setData("N/A",Qt::UserRole);
        items[x]->setData(n,Qt::UserRole+1);
        if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
        items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
        items.append(new QStandardItem("N/A"));
        items[x]->setData("N/A",Qt::UserRole);
        items[x]->setData(n,Qt::UserRole+1);
        if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
        items[x++]->setTextAlignment(Qt::AlignCenter| Qt::AlignVCenter);
    }
    Orthodromie oo(route->getStartLon(),route->getStartLat(),route->getLastPoi()->getLongitude(),route->getLastPoi()->getLatitude());
    items.append(new QStandardItem(QString( "%L1" ).arg(oo.getDistance(),0,'f',2)+tr(" NM")));
    items[x]->setData(oo.getDistance(),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(stats.totalDistance,0,'f',2)+tr(" NM")));
    items[x]->setData(stats.totalDistance,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.averageBS),0,'f',2)+tr(" kts")));
    items[x]->setData(qMax(0.0,stats.averageBS),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.maxBS),0,'f',2)+tr(" kts")));
    items[x]->setData(qMax(0.0,stats.maxBS),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(stats.minBS>1000.0?0.0:stats.minBS,0,'f',2)+tr(" kts")));
    items[x]->setData(stats.minBS>1000.0?0.0:stats.minBS,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.averageTWS),0,'f',2)+tr(" kts")));
    items[x]->setData(qMax(0.0,stats.averageTWS),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.maxTWS),0,'f',2)+tr(" kts")));
    items[x]->setData(qMax(0.0,stats.maxTWS),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(stats.minTWS>1000?0.0:stats.minTWS,0,'f',2)+tr(" kts")));
    items[x]->setData(stats.minTWS>1000?0.0:stats.minTWS,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg((double)stats.nbTacksGybes,0,'f',0)));
    items[x]->setData(stats.nbTacksGybes,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    double days=stats.beatingTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    double hours=(stats.beatingTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    double mins=qRound((stats.beatingTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.beatingTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    days=stats.reachingTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    hours=(stats.reachingTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    mins=qRound((stats.reachingTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.reachingTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    days=stats.largueTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    hours=(stats.largueTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    mins=qRound((stats.largueTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.largueTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    days=stats.engineTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    hours=(stats.engineTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    mins=qRound((stats.engineTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.engineTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    days=stats.nightTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    hours=(stats.nightTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    mins=qRound((stats.nightTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.nightTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    days=stats.rainTime/86400.0000;
    if(qRound(days)>days)
        days=qRound(days)-1;
    else
        days=qRound(days);
    hours=(stats.rainTime-days*86400)/3600.0000;
    if(qRound(hours)>hours)
        hours=qRound(hours)-1;
    else
        hours=qRound(hours);
    mins=qRound((stats.rainTime-days*86400-hours*3600)/60.0000);
    items.append(new QStandardItem(QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                                   QString::number((int)mins)+" "+tr("minutes")));
    items[x]->setData(stats.rainTime,Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.maxWaveHeight),0,'f',2)+tr(" m")));
    items[x]->setData(qMax(0.0,stats.maxWaveHeight),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    items.append(new QStandardItem(QString( "%L1" ).arg(qMax(0.0,stats.combWaveHeight),0,'f',2)+tr(" m")));
    items[x]->setData(qMax(0.0,stats.combWaveHeight),Qt::UserRole);
    items[x]->setData(n,Qt::UserRole+1);
    if(x%2!=0) items[x]->setData(this->palette().alternateBase().color(),Qt::BackgroundRole);
    items[x++]->setTextAlignment(Qt::AlignRight| Qt::AlignVCenter);
    model->appendRow(items);
    for(x=0;x<model->columnCount();++x)
        routesTable->resizeColumnToContents(x);
}

DialogRouteComparator::~DialogRouteComparator()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    Settings::setSetting(this->objectName()+".positionx",this->pos().x());
    Settings::setSetting(this->objectName()+".positiony",this->pos().y());
    if (model)
        delete model;
}
