#ifndef DIALOGHORN_H
#define DIALOGHORN_H

#include "class_list.h"
#include "mycentralwidget.h"
#include "ui_dialoghorn.h"


class DialogHorn : public QDialog, public Ui::DialogHorn
{ Q_OBJECT
public:
    DialogHorn(myCentralWidget *parent);
    ~DialogHorn();

protected:
private:
    myCentralWidget *parent;

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // DIALOGHORN_H
