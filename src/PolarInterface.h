#ifndef POLARINTERFACE_H
#define POLARINTERFACE_H
#include <QObject>
class PolarInterface: public QObject
{ Q_OBJECT
    public:
        ~PolarInterface(){}
        virtual double   getSpeed(double windSpeed, double angle, bool engine=true, bool * engineUsed=NULL)=0;
        virtual double   getBvmgUp(double windSpeed, bool engine=true)=0;
        virtual double   getBvmgDown(double windSpeed, bool engine=true)=0;
        virtual QString getName()=0;
};

#endif // POLARINTERFACE_H
