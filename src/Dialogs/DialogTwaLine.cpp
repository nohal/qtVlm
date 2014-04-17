#include "DialogTwaLine.h"
#include "mycentralwidget.h"
#include "DataManager.h"
#include "boatVLM.h"
#include "vlmLine.h"
#include "Util.h"
#include "Polar.h"
#include <QDebug>
#include "settings.h"
#include "GshhsReader.h"
#include "Projection.h"
DialogTwaLine::DialogTwaLine(QPointF start, myCentralWidget *parent, MainWindow *main) : QDialog(parent)
{
    this->parent=parent;
    this->start=start;
    this->dataManager=parent->get_dataManager();
    this->myBoat=parent->getSelectedBoat();

    color=Settings::getSetting(traceLineColor).value<QColor>();
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(Settings::getSetting(traceLineWidth).toDouble());
    this->line=new vlmLine(parent->getProj(),parent->getScene(),Z_VALUE_ROUTE);
    line->setLinePen(pen);
    this->setWindowFlags(Qt::Tool);
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    setupUi(this);
    this->tabWidget->setCurrentIndex(4);
    Util::setFontDialog(this);
    this->tabWidget->setCurrentIndex(0);
    this->doubleSpinBox->setFocus();
    this->doubleSpinBox->selectAll();
    this->main=main;
    this->position=this->pos();
    if(myBoat->get_boatType()!=BOAT_VLM)
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
    connect(TWA1,SIGNAL(toggled(bool)),this,SLOT(slotTwa1(bool)));
    connect(TWA2,SIGNAL(toggled(bool)),this,SLOT(slotTwa2(bool)));
    connect(TWA3,SIGNAL(toggled(bool)),this,SLOT(slotTwa3(bool)));
    connect(TWA4,SIGNAL(toggled(bool)),this,SLOT(slotTwa4(bool)));
    connect(TWA5,SIGNAL(toggled(bool)),this,SLOT(slotTwa5(bool)));
    Util::setWidgetSize(this);
    this->tabWidget->setCurrentIndex(0);
    traceIt();
}
void DialogTwaLine::slot_screenResize()
{
    Util::setWidgetSize(this);
}
void DialogTwaLine::slotTwa1(bool b)
{
    this->blockSignals(true);
    if(b)
    {
        ltwa1->setText(tr("Twa"));
        this->doubleSpinBox->setMaximum(180);
        this->doubleSpinBox->setMinimum(-179.99);
    }
    else
    {
        ltwa1->setText(tr("Cap"));
        this->doubleSpinBox->setMaximum(360);
        this->doubleSpinBox->setMinimum(0);
    }
    this->blockSignals(false);
    traceIt();
}
void DialogTwaLine::slotTwa2(bool b)
{
    this->blockSignals(true);
    if(b)
    {
        ltwa2->setText(tr("Twa"));
        this->doubleSpinBox_2->setMaximum(180);
        this->doubleSpinBox_2->setMinimum(-179.99);
    }
    else
    {
        ltwa2->setText(tr("Cap"));
        this->doubleSpinBox_2->setMaximum(360);
        this->doubleSpinBox_2->setMinimum(0);
    }
    this->blockSignals(false);
    traceIt();
}
void DialogTwaLine::slotTwa3(bool b)
{
    this->blockSignals(true);
    if(b)
    {
        ltwa3->setText(tr("Twa"));
        this->doubleSpinBox_3->setMaximum(180);
        this->doubleSpinBox_3->setMinimum(-179.99);
    }
    else
    {
        ltwa3->setText(tr("Cap"));
        this->doubleSpinBox_3->setMaximum(360);
        this->doubleSpinBox_3->setMinimum(0);
    }
    this->blockSignals(false);
    traceIt();
}
void DialogTwaLine::slotTwa4(bool b)
{
    this->blockSignals(true);
    if(b)
    {
        ltwa4->setText(tr("Twa"));
        this->doubleSpinBox_4->setMaximum(180);
        this->doubleSpinBox_4->setMinimum(-179.99);
    }
    else
    {
        ltwa4->setText(tr("Cap"));
        this->doubleSpinBox_4->setMaximum(360);
        this->doubleSpinBox_4->setMinimum(0);
    }
    this->blockSignals(false);
    traceIt();
}
void DialogTwaLine::slotTwa5(bool b)
{
    this->blockSignals(true);
    if(b)
    {
        ltwa5->setText(tr("Twa"));
        this->doubleSpinBox_5->setMaximum(180);
        this->doubleSpinBox_5->setMinimum(-179.99);
    }
    else
    {
        ltwa5->setText(tr("Cap"));
        this->doubleSpinBox_5->setMaximum(360);
        this->doubleSpinBox_5->setMinimum(0);
    }
    this->blockSignals(false);
    traceIt();
}

DialogTwaLine::~DialogTwaLine()
{
    Settings::saveGeometry(this);
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

void DialogTwaLine::closeEvent(QCloseEvent *)
{
    Settings::saveGeometry(this);
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
    if(!dataManager->isOk()) return;
    if(!myBoat->getPolarData()) return;
    time_t eta;
    if(this->startGrib->isChecked() || myBoat->get_boatType()!=BOAT_VLM)
        eta=dataManager->get_currentDate();
    else
        eta=((boatVLM*)myBoat)->getPrevVac()+myBoat->getVacLen();
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
    mode[0]=this->TWA1->isChecked();
    mode[1]=this->TWA2->isChecked();
    mode[2]=this->TWA3->isChecked();
    mode[3]=this->TWA4->isChecked();
    mode[4]=this->TWA5->isChecked();
    int vacLen=myBoat->getVacLen();
    vlmPoint current(start.x(),start.y());
    line->addVlmPoint(current);
    double wind_speed,wind_angle,cap;
    double lon=0,lat=0;
    time_t maxDate=dataManager->get_maxDate();
    bool crossing=false;
    //int i1,j1,i2,j2;
    GshhsReader *map=parent->get_gshhsReader();
    int mapQuality=map?map->getQuality():4;
    for (int page=0;page<5;page++)
    {
        if (nbVac[page]==0) continue;
        for(int i=1;i<=nbVac[page];i++)
        {
            double current_speed=-1;
            double current_angle=0;
            if(!dataManager->getInterpolatedWind(current.lon, current.lat,
                eta,&wind_speed,&wind_angle,INTERPOLATION_DEFAULT) || eta>maxDate)
                break;
            wind_angle=radToDeg(wind_angle);
            if(dataManager->getInterpolatedCurrent(current.lon, current.lat,
                eta,&current_speed,&current_angle,INTERPOLATION_DEFAULT))
            {
                current_angle=radToDeg(current_angle);
                QPointF p=Util::calculateSumVect(wind_angle,wind_speed,current_angle,current_speed);
                //qWarning()<<"cs="<<current_speed<<"cd="<<current_angle<<"tws="<<wind_speed<<"twd="<<wind_angle<<"Ltws="<<p.x()<<"Ltwd="<<p.y();
                wind_speed=p.x();
                wind_angle=p.y();
            }
            double TWA;
            if(mode[page])
            {
                cap=AngleUtil::A360(wind_angle+twa[page]);
                TWA=twa[page];
            }
            else
            {
                cap=twa[page];
                TWA=AngleUtil::A360(cap-wind_angle);
                if(qAbs(TWA)>180)
                {
                    if(TWA<0)
                        TWA=360+TWA;
                    else
                        TWA=TWA-360;
                }
            }
            double newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,TWA);
            if(current_speed>0)
            {
                QPointF p=Util::calculateSumVect(cap,newSpeed,AngleUtil::A360(current_angle+180.0),current_speed);
                newSpeed=p.x(); //in this case newSpeed is SOG
                cap=p.y(); //in this case cap is COG
            }
            double distanceParcourue=newSpeed*vacLen/3600.00;
            Util::getCoordFromDistanceAngle(current.lat, current.lon, distanceParcourue, cap,&lat,&lon);
            if(!crossing && map && mapQuality>=3)
            {
                double I1,J1,I2,J2;
                parent->getProj()->map2screenDouble(current.lon,current.lat,&I1,&J1);
                parent->getProj()->map2screenDouble(lon,lat,&I2,&J2);
                crossing=map->crossing(QLineF(I1,J1,I2,J2),QLineF(current.lon,current.lat,lon,lat));
            }
            current.lon=lon;
            current.lat=lat;
            line->addVlmPoint(current);
            eta=eta+vacLen;
        }
        if(crossing)
            pen.setColor(Qt::red);
        else
            pen.setColor(color);
        line->setLinePen(pen);
        QDateTime tm;
        tm.setTimeSpec(Qt::UTC);
        if(this->startVac->isChecked())
            tm.setTime_t(eta-vacLen);
        else
            tm.setTime_t(eta);
        //QString name;
        //name.sprintf("Twa %.1f",twa[page]);
        POI * arrival=parent->slot_addPOI(tr("ETA: ")+tm.toString("dd MMM-hh:mm"),0,lat,lon,-1);
        arrival->setPartOfTwa(true);
        list.append(arrival);
    }
    line->slot_showMe();
    QApplication::processEvents();
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
