#include "dialoghorn.h"
#include "ui_dialoghorn.h"


DialogHorn::DialogHorn(myCentralWidget *parent) : QDialog(parent)
{
    this->parent=parent;
    setupUi(this);
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
