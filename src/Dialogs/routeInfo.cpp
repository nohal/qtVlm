#include "routeInfo.h"
#include "ui_routeInfo.h"
#include "Util.h"


routeInfo::routeInfo(myCentralWidget *parent, ROUTE *route) :
    QDialog(parent)
{
    this->setWindowFlags(Qt::Tool);
    setupUi(this);
    Util::setFontDialog(this);
    this->route=route;
    this->setWindowTitle(QObject::tr("Information au point d'interpolation pour ")+route->getName());
    //this->activateWindow();
}
void routeInfo::setValues(double twd, double tws, double twa, double bs, double hdg, double cnm, double dnm, bool engineUsed)
{
    TWD->setValue(twd);
    TWS->setValue(tws);
    TWA->setValue(qAbs(twa));
    BS->setValue(bs);
    HDG->setValue(hdg);
    CNM->setValue(cnm);
    DNM->setValue(dnm);
    double Y=90-qAbs(twa);
    double a=tws*cos(degToRad(Y));
    double b=tws*sin(degToRad(Y));
    double bb=b+bs;
    double aws=sqrt(a*a+bb*bb);
    double awa=90-radToDeg(atan(bb/a));
    AWA->setValue(awa);
    AWS->setValue(aws);
    if(engineUsed)
    {
        amure->setText(tr("Au moteur"));
    }
    else
    {
        if(twa>=0)
            amure->setText(tr("Babord amure"));
        else
            amure->setText(tr("Tribord amure"));
    }
}

routeInfo::~routeInfo()
{
}
void routeInfo::closeEvent(QCloseEvent *)
{
    QPoint position=this->pos();
    route->setShowInterpolData(false);
    this->move(position);
}
