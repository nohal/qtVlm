#ifndef DIALOGFAXMETEO_H
#define DIALOGFAXMETEO_H

#include <QDialog>
#include "class_list.h"
#include "faxMeteo.h"
#include "mycentralwidget.h"
#include "ui_dialogFaxMeteo.h"

class dialogFaxMeteo : public QDialog, public Ui::dialogFaxMeteo_ui
{ Q_OBJECT
    
public:
    dialogFaxMeteo(faxMeteo * fax, myCentralWidget * parent);
    ~dialogFaxMeteo();
    void done(int result);
public slots:
    void browseFile();
    void slotPreset1();
    void slotPreset2();
    void slotPreset3();
    void slotPreset4();
    void slotPreset5();
    void slotPreset6();
    void slotPreset7();
    void slotPreset8();
    void slot_screenResize();
private:
    QString presetNb;
    void loadPreset();
    void savePreset();
    QString previousPresetNb;
    faxMeteo * fax;
    myCentralWidget * parent;
};

#endif // DIALOGFAXMETEO_H
