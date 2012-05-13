#include "dialogpoiconnect.h"
#include "DialogGraphicsParams.h"
#include <QMessageBox>

DialogPoiConnect::DialogPoiConnect(POI * poi,myCentralWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    this->poi=poi;
    this->parent=parent;
    inputLineColor =new InputLineParams(poi->getLineWidth(),poi->getLineColor(),1.6,  QColor(Qt::red),this,0.1,5);
    colorBox->layout()->addWidget( inputLineColor);
    setWindowTitle(tr("Tracer une ligne entre deux POIs"));
    QList<POI*> poiList=parent->getPois();
    for(int m=0;m<poiList.count();m++)
    {
        POI * p = poiList.at(m);
        if(p!=poi && p->getRoute()==NULL && (p->getConnectedPoi()==NULL || p->getConnectedPoi()==poi))
        {
            this->comboPoi->addItem(p->getName(),m);
            if(p->getConnectedPoi()==poi)
                this->comboPoi->setCurrentIndex(comboPoi->count()-1);
        }
    }
}

DialogPoiConnect::~DialogPoiConnect()
{
}
void DialogPoiConnect::done(int result)
{
    if(this->checkBox->isChecked())
    {
        if(poi->getConnectedPoi()!=NULL)
        {
            poi->getConnectedPoi()->setConnectedPoi(NULL);
            poi->getConnectedPoi()->setLineBetweenPois(NULL);
            poi->setConnectedPoi(NULL);
        }
        QDialog::done(QDialog::Accepted);
        return;
    }
    if(result == QDialog::Accepted)
    {
        if(comboPoi->currentIndex()==-1)
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Vous devez selectionner un poi dans la liste"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        poi->setConnectedPoi(parent->getPois().at(comboPoi->itemData(comboPoi->currentIndex(),Qt::UserRole).toInt()));
        poi->getConnectedPoi()->setConnectedPoi(poi);
        poi->setLineWidth(inputLineColor->getLineWidth());
        poi->setLineColor(inputLineColor->getLineColor());
        poi->getConnectedPoi()->setLineWidth(inputLineColor->getLineWidth());
        poi->getConnectedPoi()->setLineColor(inputLineColor->getLineColor());
    }
    QDialog::done(result);
}
