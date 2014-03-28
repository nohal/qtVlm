#ifndef DIALOGROUTAGE_VIEW_MOBILE_H
#define DIALOGROUTAGE_VIEW_MOBILE_H

#include <QDialog>

#include "Dialog_view_pc.h"
#include "DialogRoutage_view.h"
#include "ui_DialogRoutage_mobile.h"

#include "class_list.h"

class DialogRoutage_view_mobile: public Dialog_view_pc, public DialogRoutage_view, public Ui::DialogRoutage_mobile_ui
{
    Q_OBJECT

public:
    DialogRoutage_view_mobile(myCentralWidget * centralWidget, DialogRoutage_ctrl *ctrl);
    ~DialogRoutage_view_mobile();
    void initDialogState(RoutageData *routageData);
    void set_dialogVisibility(bool visible);

    void closeEvent(QCloseEvent * );

public slots:
    void slot_default(void);
    void GybeTack(int i);
    void slot_ok();
    void slot_cancel();

private:
    RoutageData * routageData;

    InputLineParams *inputTraceColor;
};

#endif // DIALOGROUTAGE_VIEW_MOBILE_H
