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

dialogLoadImg::dialogLoadImg(loadImg * carte, myCentralWidget *parent)
    : QDialog(parent)
{
    this->carte=carte;
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
        carte->setParams(this->alpha->value()/100.0,
                         this->gribAlpha->value()/100.0,
                         drawGribOverKap->isChecked(),
                         !gribColored->isChecked());
        if(carte->getMyImgFileName()!=this->FileName->text())
        {
            int res=carte->setMyImgFileName(this->FileName->text());
            if(res!=1)
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Fichier Kap invalide"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                Settings::setSetting("LastKap",QString());
                return;
            }
        }
        Settings::setSetting("LastKap",this->FileName->text());
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
    if(cartePath==".") cartePath=QDir::currentPath();
    QDir dircarte(cartePath);
    if(!dircarte.exists())
    {
        cartePath=QDir::currentPath();
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
}
void dialogLoadImg::setGribOpacity(int i)
{
    carte->setGribOpacity((double)i/100.0);
}
void dialogLoadImg::setKapOpacity(int i)
{
    carte->setOpacity((double)i/100.0);
}
