#include "finePosit.h"
#include "POI.h"
#include "mycentralwidget.h"


finePosit::finePosit(POI * poi,myCentralWidget *parent)
    : QDialog(parent)
{
    this->poi=poi;
    this->parent=parent;
    setupUi(this);
    setWindowTitle(tr("Parametres du positionnement automatique"));
    etendueLon->setValue(poi->getSearchRangeLon());
    etendueLat->setValue(poi->getSearchRangeLat());
    step->setValue(poi->getSearchStep());
    drawRoute->setChecked(poi->getOptimizing());
    this->etendueLon->setFocus();
    this->etendueLon->selectAll();
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
}
finePosit::~finePosit()
{
}
//---------------------------------------
void finePosit::done(int result)
{
    if(result == QDialog::Accepted)
    {
        poi->setSearchRangeLon(etendueLon->value());
        poi->setSearchRangeLat(etendueLat->value());
        poi->setSearchStep(step->value());
        poi->setOptimizing(drawRoute->isChecked());
    }
    if(result == QDialog::Rejected)
    {
    }
    QDialog::done(result);
}


