#include "DialogHorn.h"
#include "ui_dialoghorn.h"
#include "Util.h"
#include "settings.h"


DialogHorn::DialogHorn(myCentralWidget *parent) : QDialog(parent)
{
    this->parent=parent;
    setupUi(this);
    Util::setFontDialog(this);
    if(parent->getHornDate()<(QDateTime::currentDateTime()).toUTC())
    {
        this->date->setDateTime((QDateTime::currentDateTime().toUTC()));
        parent->setHornIsActivated(false);
    }
    else
        this->date->setDateTime(parent->getHornDate());
    if(parent->hornIsActivated())
        this->activated->setChecked(true);
}

DialogHorn::~DialogHorn()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}


void DialogHorn::on_buttonBox_accepted()
{
    parent->setHornDate(this->date->dateTime().toUTC());
    parent->setHornIsActivated(this->activated->isChecked());
    QDialog::done(QDialog::Accepted);
}

void DialogHorn::on_buttonBox_rejected()
{
    QDialog::done(QDialog::Rejected);

}
