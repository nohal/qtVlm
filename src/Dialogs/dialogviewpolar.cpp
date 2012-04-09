#include "dialogviewpolar.h"
#include "ui_dialogviewpolar.h"
#include "dataDef.h"
#include <QDebug>
//#include <QEvent>

DialogViewPolar::DialogViewPolar(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    image=QPixmap(this->imageContainer->size());
    image.fill(Qt::red);
    pnt.begin(&image);
    pnt.setRenderHint(QPainter::Antialiasing);
    this->imageContainer->setPixmap(image);
    connect(this->doubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(drawIt()));
    connect(this->allSpeed,SIGNAL(clicked()),this,SLOT(drawIt()));
    imageContainer->installEventFilter(this);
    connect(this->closeButton,SIGNAL(clicked()),this,SLOT(close()));
}
bool DialogViewPolar::eventFilter(QObject *, QEvent *event)
{
    if(this->allSpeed->isChecked()) return false;
    if(event->type()==QEvent::MouseButtonRelease)
    {
        imageContainer->setPixmap(image);
        info->clear();
        return true;
    }
    if(event->type()!=QEvent::MouseMove)
        return false;
    QMouseEvent * mouseEvent=static_cast<QMouseEvent*>(event);
    QPixmap i2=image;
    QPen pe(Qt::blue);
    pe.setWidthF(3);
    QPainter pp(&i2);
    pp.setRenderHint(QPainter::Antialiasing);
    pp.setPen(pe);
    QPointF center(0,image.height()/2.0);
    QLineF line(center,mouseEvent->pos());
    int angle=qRound(line.angle());
    if(angle>180)
        angle=angle-360;
    angle=90-angle;
    if(angle<0 || angle>180)
        return true;
    pp.drawLine(center,polarLine.at(angle));
    pe.setWidthF(1);
    pp.setPen(pe);
    QString s;
    double Y=90-angle;
    double a=this->doubleSpinBox->value()*cos(degToRad(Y));
    double b=this->doubleSpinBox->value()*sin(degToRad(Y));
    double bb=b+polarValues.at(angle);
    double aws=sqrt(a*a+bb*bb);
    double awa=90-radToDeg(atan(bb/a));
    double vmg=polarValues.at(angle)*cos(degToRad(angle));
    s=s.sprintf(QObject::tr("TWA %ddeg, BS %.2fnds\nAWA %.2fdeg, AWS %.2fnds\nVMG %.2fnds").toAscii(),angle,polarValues.at(angle),awa,aws,vmg);
    //QRect r(polarLine.at(angle).x(),polarLine.at(angle).y(),500,500);
    //pp.drawText(r,s);
    imageContainer->setPixmap(i2);
    info->setText(s);
    return true;
}
void DialogViewPolar::setBoat(boat *myboat)
{
    this->myBoat=myboat;
    this->setWindowTitle(tr("Analyse de la polaire: ")+myBoat->getPolarName());
    this->drawIt();
}

DialogViewPolar::~DialogViewPolar()
{
}
void DialogViewPolar::reloadPolar()
{
    myBoat->reloadPolar(true);
    drawIt();
}

void DialogViewPolar::drawIt()
{
    Polar * polar=myBoat->getPolarData();
    if(!polar) return;
    image.fill(Qt::white);
    double maxSpeed=-1;
    if(this->allSpeed->isChecked())
        maxSpeed=polar->getMaxSpeed();
    pen.setBrush(Qt::red);
    double maxSize=(image.height()/2)*0.92;
    double ws=this->doubleSpinBox->value();
    double wsMin=ws;
    double wsMax=ws;
    if(this->allSpeed->isChecked())
    {
        wsMin=5;
        wsMax=50;
    }
    QString s;
    if(this->allSpeed->isChecked())
    {
        this->BVMG_down->clear();
        this->BVMG_up->clear();
    }
    QPointF center(0,image.height()/2.0);
    int step=5;
    int nn=0;
    for(ws=wsMin;ws<=wsMax;ws+=step)
    {
        ++nn;
        polarValues.clear();
        polarLine.clear();
        polarGreen.clear();
        double bvmgUp=polar->getBvmgUp(ws);
        double bvmgDown=polar->getBvmgDown(ws);
        pen.setColor(Qt::red);
        pen.setWidth(2);
        pnt.setPen(pen);
        if(!this->allSpeed->isChecked())
        {
            s=s.sprintf("%.1f",bvmgUp);
            this->BVMG_up->setText(s);
            s=s.sprintf("%.1f",bvmgDown);
            this->BVMG_down->setText(s);
        }
        for(int angle=0;angle<=180;++angle)
        {
            double speed=polar->getSpeed(ws,angle);
            polarValues.append(speed);
            if(speed>maxSpeed && !this->allSpeed->isChecked()) maxSpeed=speed;
        }
        maxSpeed=ceil(maxSpeed);
        for (int angle=0;angle<=180;++angle)
        {
            QLineF line(center,QPointF(image.width(),image.height()/2.0));
            line.setAngle(90-angle);
            line.setLength(polarValues.at(angle)*maxSize/maxSpeed);
            polarLine.append(line.p2());
            if(angle>bvmgUp && angle < bvmgDown)
                polarGreen.append(line.p2());
        }
        pnt.drawPolyline(polarLine.toPolygon());
        pen.setColor(Qt::green);
        pnt.setPen(pen);
        pnt.drawPolyline(polarGreen.toPolygon());
        if(this->allSpeed->isChecked())
        {
            pen.setColor(Qt::blue);
            pen.setWidthF(0.5);
            pnt.setPen(pen);
            s=s.sprintf("tws %.0f",ws);
            pnt.drawText(polarLine.at(qMin(180,nn*15+40)),s);
        }
    }
    pen.setColor(Qt::black);
    pen.setWidthF(0.5);
    pnt.setPen(pen);
    QPolygonF lines;
    for (int bs=0;bs<=maxSpeed+1;bs+=2)
    {
        lines.clear();
        for (int angle=0;angle<=180;++angle)
        {
            QLineF line(center,QPointF(image.width(),image.height()/2.0));
            line.setAngle(90-angle);
            line.setLength(bs*maxSize/maxSpeed);
            lines.append(line.p2());
        }
        pnt.drawPolyline(lines.toPolygon());
        s=s.sprintf("%d",bs);
        pnt.drawText(lines.first(),s);
    }
    for (int angle=0;angle<=180;angle+=15)
    {
        QLineF line(center,QPointF(image.width(),image.height()/2.0));
        line.setAngle(90-angle);
        line.setLength((maxSpeed+1)*maxSize/maxSpeed);
        pnt.drawLine(line);
        if(angle==0) continue;
        line.setLength(line.length()*1.03);
        s=s.sprintf("%d",angle);
        pnt.drawText(line.p2(),s);
    }
    this->imageContainer->setPixmap(image);
}
