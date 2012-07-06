#ifndef ROUTEINFO_H
#define ROUTEINFO_H

#include <QDialog>
#include "ui_routeInfo.h"
#include "mycentralwidget.h"
#include "route.h"

class routeInfo : public QDialog, public Ui::routeInfo
{
    Q_OBJECT
    
public:
    routeInfo(myCentralWidget *parent, ROUTE *route);
    ~routeInfo();
    void setValues(double twd, double tws, double twa, double bs, double hdg, double cnm, double dnm, bool engineUsed);
protected:
    void closeEvent(QCloseEvent *event);
private:
    ROUTE *route;
};

#endif // ROUTEINFO_H
