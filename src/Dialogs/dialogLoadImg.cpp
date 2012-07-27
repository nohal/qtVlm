#include "dialogLoadImg.h"
#include "ui_dialogLoadImg.h"
#include "math.h"
#include "settings.h"
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include "Util.h"

dialogLoadImg::dialogLoadImg(loadImg * carte, myCentralWidget *parent)
    : QDialog(parent)
{
    this->carte=carte;
    setupUi(this);
    Util::setFontDialog(this);
    //this->FileName->setText(carte->getFileName());
    //this->alpha->setValue(qRound((1.0-carte->getAlpha())*100));
    //this->alpha->setMaximum((1.0-MIN_ALPHA)*100);
    connect(this->Browse,SIGNAL(clicked()),this,SLOT(browseFile()));
}

dialogLoadImg::~dialogLoadImg()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}
void dialogLoadImg::done(int result)
{
    if(result == QDialog::Accepted)
    {
        carte->setMyImgFileName(this->FileName->text());
        double lat1=this->lat_deg_UL->value()+this->lat_min_UL->value()/60.0;
        if(this->lat_sig_UL->currentIndex()==1)
            lat1=-lat1;
        double lon1=this->lon_deg_UL->value()+this->lon_min_UL->value()/60.0;
        if(this->lon_sig_UL->currentIndex()==1)
            lon1=-lon1;
        double lat2=this->lat_deg_LR->value()+this->lat_min_LR->value()/60.0;
        if(this->lat_sig_LR->currentIndex()==1)
            lat2=-lat2;
        double lon2=this->lon_deg_LR->value()+this->lon_min_LR->value()/60.0;
        if(this->lon_sig_LR->currentIndex()==1)
            lon2=-lon2;
//        lat2=35.7465123;
//        lon2=28.8281250;
//        lat1=41.2447723;
//        lon1=21.7968750;
        carte->setLonLat(lon1,lat1,lon2,lat2);
        carte->setAlpha(1.0-(this->alpha->value()/100.0));
        carte->slot_updateProjection();
    }
    QDialog::done(result);
}
void dialogLoadImg::browseFile()
{
    QString filter;
    filter =  tr("Fichiers image (*.png *.jpg *.tiff *.gif *.bmp)")
            + tr(";;Autres fichiers (*)");
    QString cartePath=Settings::getSetting("cartePath",".").toString();
    if(cartePath==".") cartePath=QDir::currentPath();
    QDir dircarte(cartePath);
    if(!dircarte.exists())
    {
        cartePath=QDir::currentPath();
        Settings::setSetting("cartePath",cartePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier image"),
                         cartePath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        cartePath = finfo.absolutePath();
        this->FileName->setText(fileName);
        Settings::setSetting("cartePath",cartePath);
    }
}
