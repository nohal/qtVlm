#include "dialogLoadImg.h"
#include "ui_dialogLoadImg.h"
#include "math.h"
#include "settings.h"
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include "Util.h"
#include <QMessageBox>
#include "bsb.h"

dialogLoadImg::dialogLoadImg(loadImg * carte, myCentralWidget *parent)
    : QDialog(parent)
{
    this->carte=carte;
    this->parent=parent;
    setupUi(this);
    Util::setFontDialog(this);
    this->alpha->setMaximum(100);
    this->gribAlpha->setMaximum(100);
    this->FileName->setText(carte->getMyImgFileName());
    this->alpha->setValue(carte->getAlpha()*100);
    this->drawGribOverKap->setChecked(carte->getDrawGribOverKap());
    this->gribColored->setChecked(!carte->getGribColored());
    this->gribAlpha->setValue(carte->getGribAlpha()*100);
    connect(this->Browse,SIGNAL(clicked()),this,SLOT(browseFile()));
    connect(this->alpha,SIGNAL(valueChanged(int)),this,SLOT(setKapOpacity(int)));
    connect(this->drawGribOverKap,SIGNAL(toggled(bool)),this,SLOT(slotGribKap()));
    connect(this->gribColored,SIGNAL(toggled(bool)),this,SLOT(slotGribKap()));
    connect(this->gribAlpha,SIGNAL(valueChanged(int)),this,SLOT(setGribOpacity(int)));
    connect(this->zoom,SIGNAL(clicked()),this,SLOT(nominalZoom()));
    timerResize=new QTimer(this);
    timerResize->setSingleShot(true);
    timerResize->setInterval(300);
    connect(timerResize,SIGNAL(timeout()),this,SLOT(showSnapshot()));
//    showSnapshot();
}
void dialogLoadImg::resizeEvent(QResizeEvent *)
{
    timerResize->start();
}
void dialogLoadImg::nominalZoom()
{
    this->done(3);
}

void dialogLoadImg::showSnapshot()
{
    if(!FileName->text().isEmpty())
    {
        loadImg * myCarte=new loadImg(parent->getProj(),parent);
        int OK=myCarte->setMyImgFileName(FileName->text(),false);
        if(OK!=0)
        {
            snapShot->setPixmap(myCarte->getSnapshot(snapShot->size()));
            kapInfo->setText(tr("Name: ")+myCarte->getBsb()->name);
            kapInfo-lupdat>append(tr("Projection: ")+myCarte->getBsb()->projection);
            if(myCarte->getBsb()->num_wpxs==0 || myCarte->getBsb()->num_wpys==0)
                kapInfo->append(tr("No polynomials found in kap file, using internal solution"));
            else
                kapInfo->append(tr("Polynomials found in kap file"));
            kapInfo->append(tr("Pixel size: ")+QString().setNum(myCarte->getBsb()->width)+"x"+QString().setNum(myCarte->getBsb()->height));
            zoom->setEnabled(true);
        }
        else
        {
            kapInfo->setText(tr("Error while loading")+" "+FileName->text()+"\n");
            snapShot->clear();
            zoom->setEnabled(false);
        }
        delete myCarte;
    }
    else
    {
        snapShot->clear();
        kapInfo->clear();
        zoom->setEnabled(false);
    }
}
dialogLoadImg::~dialogLoadImg()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    Settings::setSetting(this->objectName()+".positionx",this->pos().x());
    Settings::setSetting(this->objectName()+".positiony",this->pos().y());
}
void dialogLoadImg::done(int result)
{
    if(result == QDialog::Accepted || result==3)
    {
        carte->setParams(this->alpha->value()/100.0,
                         this->gribAlpha->value()/100.0,
                         drawGribOverKap->isChecked(),
                         !gribColored->isChecked());
        int res=carte->setMyImgFileName(this->FileName->text(),result!=3);
        if(res!=1)
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Fichier Kap invalide"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            Settings::setSetting("LastKap",QString());
            return;
        }
        Settings::setSetting("LastKap",this->FileName->text());
        if(result==3)
        {
            carte->nominalZoom();
            result=QDialog::Accepted;
        }
    }
    else
    {
        carte->setGribOpacity(carte->getGribAlpha());
        carte->setOpacity(carte->getAlpha());
    }
    QDialog::done(result);
}
void dialogLoadImg::slotGribKap()
{
    carte->redraw(this->drawGribOverKap->isChecked(),!this->gribColored->isChecked());
}

void dialogLoadImg::browseFile()
{
    QString filter;
    filter =  tr("Fichiers kap (*.kap *.KAP)");
    QString cartePath=Settings::getSetting("cartePath",".").toString();
    if(cartePath==".") cartePath=Util::currentPath();
    QDir dircarte(cartePath);
    if(!dircarte.exists())
    {
        cartePath=Util::currentPath();
        Settings::setSetting("cartePath",cartePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier kap"),
                         cartePath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        cartePath = finfo.absolutePath();
        this->FileName->setText(fileName);
        Settings::setSetting("cartePath",cartePath);
    }
    showSnapshot();
}
void dialogLoadImg::setGribOpacity(int i)
{
    carte->setGribOpacity((double)i/100.0);
}
void dialogLoadImg::setKapOpacity(int i)
{
    carte->setOpacity((double)i/100.0);
}
