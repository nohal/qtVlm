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
private:
    QString presetNb;
    void loadPreset();
    void savePreset();
    QString previousPresetNb;
    faxMeteo * fax;
    myCentralWidget * parent;
};

#endif // DIALOGFAXMETEO_H
