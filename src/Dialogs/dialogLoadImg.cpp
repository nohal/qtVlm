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
        if(!carte->setMyImgFileName(this->FileName->text()))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Fichier Kap invalide"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        carte->setAlpha(1.0-(this->alpha->value()/100.0));
        carte->slot_updateProjection();
    }
    QDialog::done(result);
}
void dialogLoadImg::browseFile()
{
    QString filter;
    filter =  tr("Fichiers kap (*.kap *.KAP");
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
