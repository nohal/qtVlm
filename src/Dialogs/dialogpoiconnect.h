#ifndef DIALOGPOICONNECT_H
#define DIALOGPOICONNECT_H

#include "class_list.h"
#include <QDialog>
#include "ui_dialogpoiconnect.h"
#include "POI.h"

class DialogPoiConnect : public QDialog, public Ui::DialogPoiConnect
{
    Q_OBJECT

public:
    DialogPoiConnect(POI* poi,myCentralWidget *parent);
    ~DialogPoiConnect();
    void done(int result);

public slots:
    void slot_screenResize();
private:
    InputLineParams *inputLineColor;
    POI * poi;
    myCentralWidget * parent;

};

#endif // DIALOGPOICONNECT_H
