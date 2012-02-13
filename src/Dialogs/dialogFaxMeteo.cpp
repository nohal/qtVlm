#include "dialogFaxMeteo.h"
#include "ui_dialogFaxMeteo.h"
#include "math.h"
#include "settings.h"
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include "Util.h"

dialogFaxMeteo::dialogFaxMeteo(faxMeteo * fax, myCentralWidget *parent)
    : QDialog(parent)
{
    this->fax=fax;
    fax->savePreset();
    setupUi(this);
    Util::setFontDialog(this);
    this->presetNb=fax->getPresetNb();
    this->previousPresetNb=presetNb;
    if(presetNb=="1")
        this->preset1->setChecked(true);
    else if(presetNb=="2")
        this->preset2->setChecked(true);
    else if(presetNb=="3")
        this->preset3->setChecked(true);
    else if(presetNb=="4")
        this->preset4->setChecked(true);
    this->loadPreset();
    connect(this->Browse,SIGNAL(clicked()),this,SLOT(browseFile()));
    connect(this->preset1,SIGNAL(clicked()),this,SLOT(slotPreset1()));
    connect(this->preset2,SIGNAL(clicked()),this,SLOT(slotPreset2()));
    connect(this->preset3,SIGNAL(clicked()),this,SLOT(slotPreset3()));
    connect(this->preset4,SIGNAL(clicked()),this,SLOT(slotPreset4()));
}

dialogFaxMeteo::~dialogFaxMeteo()
{
}
void dialogFaxMeteo::slotPreset1()
{
    this->presetNb="1";
    this->loadPreset();
}
void dialogFaxMeteo::slotPreset2()
{
    this->presetNb="2";
    this->loadPreset();
}
void dialogFaxMeteo::slotPreset3()
{
    this->presetNb="3";
    this->loadPreset();
}
void dialogFaxMeteo::slotPreset4()
{
    this->presetNb="4";
    this->loadPreset();
}
void dialogFaxMeteo::loadPreset()
{
    fax->setPresetNb(this->presetNb);
    fax->loadPreset();
    this->FileName->setText(fax->getFileName());
    QPointF leftCorner=fax->getLonLat();
    if (leftCorner.y()<0)
        this->lat_sig->setCurrentIndex(1);
    else
        this->lat_sig->setCurrentIndex(0);
    if (leftCorner.x()<0)
        this->lon_sig->setCurrentIndex(1);
    else
        this->lon_sig->setCurrentIndex(0);
    this->lat_deg->setValue(floor(qAbs(leftCorner.y())));
    this->lat_min->setValue((qAbs(leftCorner.y())-floor(qAbs(leftCorner.y())))*60.0);
    this->lon_deg->setValue(floor(qAbs(leftCorner.x())));
    this->lon_min->setValue((qAbs(leftCorner.x())-floor(qAbs(leftCorner.x())))*60.0);
    this->latRange->setValue(fax->getLatRange());
    this->lonRange->setValue(fax->getLonRange());
    this->alpha->setValue(qRound((1.0-fax->getAlpha())*100));
    this->alpha->setMaximum((1.0-MIN_ALPHA)*100);
}

void dialogFaxMeteo::done(int result)
{
    if(result == QDialog::Accepted)
    {
        fax->setImgFileName(this->FileName->text());
        double lat=this->lat_deg->value()+this->lat_min->value()/60.0;
        if(this->lat_sig->currentIndex()==1)
            lat=-lat;
        double lon=this->lon_deg->value()+this->lon_min->value()/60.0;
        if(this->lon_sig->currentIndex()==1)
            lon=-lon;
        fax->setLonLat(lon,lat);
        fax->setLonLatRange(this->lonRange->value(),this->latRange->value());
        fax->setAlpha(1.0-(this->alpha->value()/100.0));
        fax->slot_updateProjection();
    }
    else if(presetNb!=previousPresetNb)
    {
        fax->setPresetNb(previousPresetNb);
        fax->loadPreset();
    }
    QDialog::done(result);
}
void dialogFaxMeteo::browseFile()
{
    QString filter;
    filter =  tr("Fichiers Fax (*.png *.jpg *.tiff *.gif *.bmp)")
            + tr(";;Autres fichiers (*)");
    QString faxPath=Settings::getSetting("faxPath",".").toString();
    if(faxPath==".") faxPath=QDir::currentPath();
    QDir dirFax(faxPath);
    if(!dirFax.exists())
    {
        faxPath=QDir::currentPath();
        Settings::setSetting("faxPath",faxPath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier FAX METEO"),
                         faxPath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        faxPath = finfo.absolutePath();
        this->FileName->setText(fileName);
        Settings::setSetting("faxPath",faxPath);
    }
}
