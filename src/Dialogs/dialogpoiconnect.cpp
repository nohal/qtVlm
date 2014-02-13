#include "dialogpoiconnect.h"
#include "DialogGraphicsParams.h"
#include <QMessageBox>
#include "Util.h"
#include "settings.h"
#ifdef QT_V5
#include <QScroller>
#endif
DialogPoiConnect::DialogPoiConnect(POI * poi,myCentralWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
#ifdef QT_V5
    QScroller::grabGesture(this->scrollArea->viewport());
#endif
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    this->poi=poi;
    this->parent=parent;
    inputLineColor =new InputLineParams(poi->getLineWidth(),poi->getLineColor(),1.6,  QColor(Qt::red),this,0.1,5);
    colorBox->layout()->addWidget( inputLineColor);
    setWindowTitle(tr("Tracer une ligne entre deux POIs"));
    QList<POI*> poiList=parent->getPois();
    for(int m=0;m<poiList.count();m++)
    {
        POI * p = poiList.at(m);
        if(p!=poi && (p->getRoute()==NULL || p->getRoute()==poi->getRoute()) && (p->getConnectedPoi()==NULL || p->getConnectedPoi()==poi))
        {
            this->comboPoi->addItem(p->getName(),m);
            if(p->getConnectedPoi()==poi)
                this->comboPoi->setCurrentIndex(comboPoi->count()-1);
        }
    }
    ortho->setChecked(poi->get_drawLineOrtho());
}
void DialogPoiConnect::slot_screenResize()
{
    Util::setWidgetSize(this,this->sizeHint());
}

DialogPoiConnect::~DialogPoiConnect()
{
    Settings::saveGeometry(this);
}
void DialogPoiConnect::done(int result)
{
    Settings::saveGeometry(this);
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
        poi->set_drawLineOrtho(ortho->isChecked());
        poi->getConnectedPoi()->set_drawLineOrtho(ortho->isChecked());
    }
    QDialog::done(result);
}
