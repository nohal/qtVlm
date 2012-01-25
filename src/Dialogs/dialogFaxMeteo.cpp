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
    setupUi(this);
    Util::setFontDialog(this);
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
    connect(this->Browse,SIGNAL(clicked()),this,SLOT(browseFile()));
    //this->debug->setText(fax->getDebugString());
}

dialogFaxMeteo::~dialogFaxMeteo()
{
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
    QDialog::done(result);
}
void dialogFaxMeteo::browseFile()
{
    QString filter;
    filter =  tr("Fichiers Fax (*.png *.jpg *.tiff *.gif *.bmp)")
            + tr(";;Autres fichiers (*)");
    QString faxPath=Settings::getSetting("faxPath",".").toString();
    if(faxPath==".") faxPath=QApplication::applicationDirPath();
    QDir dirFax(faxPath);
    if(!dirFax.exists())
    {
        faxPath=QApplication::applicationDirPath();
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
