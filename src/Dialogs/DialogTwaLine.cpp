#include "DialogTwaLine.h"
#include "mycentralwidget.h"
#include "Grib.h"
#include "boatVLM.h"
#include "vlmLine.h"
#include "Util.h"
#include "Polar.h"
#include <QDebug>
#include "settings.h"
#include "GshhsReader.h"

DialogTwaLine::DialogTwaLine(QPointF start, myCentralWidget *parent, MainWindow *main) : QDialog(parent)
{
    this->parent=parent;
    this->start=start;
    this->grib=parent->getGrib();
    this->myBoat=parent->getSelectedBoat();

    color=Settings::getSetting("traceLineColor", Qt::yellow).value<QColor>();
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(Settings::getSetting("traceLineWidth", 2.0).toDouble());
    this->line=new vlmLine(parent->getProj(),parent->getScene(),Z_VALUE_ROUTE);
    line->setLinePen(pen);
    this->setWindowFlags(Qt::Tool);
    setupUi(this);
    Util::setFontDialog(this);
    this->tabWidget->setCurrentIndex(0);
    this->doubleSpinBox->setFocus();
    this->doubleSpinBox->selectAll();
    this->main=main;
    this->position=this->pos();
    if(myBoat->getType()!=BOAT_VLM)
    {
        this->startVac->setDisabled(true);
        this->startVac->setChecked(false);
        this->startGrib->setChecked(true);
    }
    else
    {
        this->startVac->setDisabled(false);
    }
    connect(parent,SIGNAL(twaDelPoi(POI *)),this,SLOT(slot_delPOI_list(POI *)));
    this->SyncWarn->hide();
    traceIt();
}
DialogTwaLine::~DialogTwaLine()
{
}
void DialogTwaLine::slot_delPOI_list(POI * poi)
{
    list.removeAll(poi);
}
void DialogTwaLine::setStart(QPointF start)
{
    this->start=start;
    if(line!=NULL)
    {
        delete line;
        line=NULL;
    }
    QListIterator<POI*> i (list);
    while(i.hasNext())
    {
        POI * poi=i.next();
        list.removeOne(poi);
        if(poi->isPartOfTwa())
        {
            parent->slot_delPOI_list(poi);
            delete poi;
        }
    }
    this->line=new vlmLine(parent->getProj(),parent->getScene(),Z_VALUE_ROUTE);
    line->setLinePen(pen);
    this->tabWidget->setCurrentIndex(0);
    this->doubleSpinBox->setFocus();
    this->doubleSpinBox->selectAll();
    this->move(position);
    traceIt();
}

void DialogTwaLine::closeEvent(QCloseEvent */*event => unused*/)
{
    if(!this->checkBox_2->isChecked())
    {
        delete line;
        line=NULL;
    }
    if(!this->checkBox->isChecked())
    {
        QListIterator<POI*> i (list);
        while(i.hasNext())
        {
            POI * poi=i.next();
            list.removeOne(poi);
            if(poi->isPartOfTwa())
            {
                parent->slot_delPOI_list(poi);
                delete poi;
            }
        }
    }
    this->position=this->pos();
    QDialog::close();
}

void DialogTwaLine::traceIt()
{
    line->deleteAll();
    QListIterator<POI*> i (list);
    while(i.hasNext())
    {
        POI * poi=i.next();
        list.removeOne(poi);
        if(poi->isPartOfTwa())
        {
            parent->slot_delPOI_list(poi);
            delete poi;
        }
    }
    this->myBoat=parent->getSelectedBoat();
    if(myBoat==NULL) return;
    if(!grib->isOk()) return;
    if(!myBoat->getPolarData()) return;
    time_t eta;
    if(this->startGrib->isChecked() || myBoat->getType()!=BOAT_VLM)
        eta=grib->getCurrentDate();
    else
        eta=((boatVLM*)myBoat)->getPrevVac()+myBoat->getVacLen();
    time_t lastSync=((boatVLM*)myBoat)->getPrevVac();
    time_t timeNow=QDateTime::currentDateTimeUtc().toTime_t();
    if ( timeNow-lastSync > myBoat->getVacLen() )
    {
        this->SyncWarn->setText("<font color='red'>! Sync > 1 Vac !</font>");
        this->SyncWarn->show();
    }
    else
        this->SyncWarn->hide();
    nbVac[0]=this->spinBox->value();
    nbVac[1]=this->spinBox_2->value();
    nbVac[2]=this->spinBox_3->value();
    nbVac[3]=this->spinBox_4->value();
    nbVac[4]=this->spinBox_5->value();
    twa[0]=this->doubleSpinBox->value();
    twa[1]=this->doubleSpinBox_2->value();
    twa[2]=this->doubleSpinBox_3->value();
    twa[3]=this->doubleSpinBox_4->value();
    twa[4]=this->doubleSpinBox_5->value();
    int vacLen=myBoat->getVacLen();
    vlmPoint current(start.x(),start.y());
    line->addVlmPoint(current);
    double wind_speed,wind_angle,cap;
    double lon,lat;
    time_t maxDate=grib->getMaxDate();
    bool crossing=false;
    //int i1,j1,i2,j2;
    GshhsReader *map=parent->get_gshhsReader();
    int mapQuality=map->getQuality();
    for (int page=0;page<5;page++)
    {
        if (nbVac[page]==0) continue;
        for(int i=1;i<=nbVac[page];i++)
        {
            if(!grib->getInterpolatedValue_byDates(current.lon, current.lat,
                eta,&wind_speed,&wind_angle,INTERPOLATION_DEFAULT) || eta>maxDate)
                break;
            wind_angle=radToDeg(wind_angle);
            cap=A360(wind_angle+twa[page]);
            float newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,twa[page]);
            float distanceParcourue=newSpeed*vacLen/3600.00;
            Util::getCoordFromDistanceAngle(current.lat, current.lon, distanceParcourue, cap,&lat,&lon);
            if(!crossing && mapQuality>=3)
            {
                double I1,J1,I2,J2;
                parent->getProj()->map2screenFloat(current.lon,current.lat,&I1,&J1);
                parent->getProj()->map2screenFloat(lon,lat,&I2,&J2);
                crossing=map->crossing(QLineF(I1,J1,I2,J2),QLineF(current.lon,current.lat,lon,lat));
            }
            current.lon=lon;
            current.lat=lat;
            line->addVlmPoint(current);
            eta=eta+vacLen;
        }
        if(this->startVac->isChecked())
                eta=eta-vacLen;
        if(crossing)
            pen.setColor(Qt::red);
        else
            pen.setColor(color);
        line->setLinePen(pen);
        QDateTime tm;
        tm.setTimeSpec(Qt::UTC);
        tm.setTime_t(eta);
        QString name;
        name.sprintf("Twa %.1f",twa[page]);
        POI * arrival=parent->slot_addPOI(name+tr(" ETA: ")+tm.toString("dd MMM-hh:mm"),0,lat,lon,-1,0,false,myBoat);
        arrival->setPartOfTwa(true);
        list.append(arrival);
    }
    line->slot_showMe();
    QApplication::processEvents();
}
double DialogTwaLine::A360(double hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

void DialogTwaLine::on_doubleSpinBox_valueChanged(double /*d => unused*/)
{
    traceIt();
}
void DialogTwaLine::on_spinBox_valueChanged(int /* i => unused*/)
{
    traceIt();
}
void DialogTwaLine::on_doubleSpinBox_2_valueChanged(double )
{
    traceIt();
}
void DialogTwaLine::on_spinBox_2_valueChanged(int )
{
    traceIt();
}

void DialogTwaLine::on_doubleSpinBox_3_valueChanged(double )
{
    traceIt();
}

void DialogTwaLine::on_spinBox_3_valueChanged(int )
{
    traceIt();
}

void DialogTwaLine::on_doubleSpinBox_4_valueChanged(double )
{
    traceIt();
}

void DialogTwaLine::on_spinBox_4_valueChanged(int )
{
    traceIt();
}

void DialogTwaLine::on_doubleSpinBox_5_valueChanged(double )
{
    traceIt();
}

void DialogTwaLine::on_spinBox_5_valueChanged(int )
{
    traceIt();
}

void DialogTwaLine::on_startGrib_clicked(void)
{
    traceIt();
}
void DialogTwaLine::on_startVac_clicked(void)
{
    traceIt();
}
