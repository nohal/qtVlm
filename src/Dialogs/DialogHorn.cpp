#include "DialogHorn.h"
#include "ui_dialoghorn.h"
#include "Util.h"
#include "settings.h"
DialogHorn::DialogHorn(myCentralWidget *parent) : QDialog(parent)
{
    this->parent=parent;
    setupUi(this);
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
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
void DialogHorn::slot_screenResize()
{
    Util::setWidgetSize(this);
}

DialogHorn::~DialogHorn()
{
    Settings::saveGeometry(this);
}


void DialogHorn::on_buttonBox_accepted()
{
    Settings::saveGeometry(this);
    parent->setHornDate(this->date->dateTime().toUTC());
    parent->setHornIsActivated(this->activated->isChecked());
    QDialog::done(QDialog::Accepted);
}

void DialogHorn::on_buttonBox_rejected()
{
    Settings::saveGeometry(this);
    QDialog::done(QDialog::Rejected);
}
