#ifndef DIALOGVLMLOG_H
#define DIALOGVLMLOG_H

#include <QDialog>
#include <boatVLM.h>
#include "ui_DialogVlmLog.h"

namespace Ui {
    class DialogVlmLog;
}

class DialogVlmLog : public QDialog, public Ui::DialogVlmLog
{
    Q_OBJECT

public:
    DialogVlmLog(myCentralWidget *parent);
    void initWithBoat (boatVLM * activeBoat);
    ~DialogVlmLog();

public slots:
    void slot_updateData(void);
private:
    Ui::DialogVlmLog *ui;
    QList<QVariantMap> boatLog;
    QStringList tableKeys;
    QStandardItemModel * model;
    boatVLM * vlmBoat;
    void buildModel(QList<QVariantMap> boatInfoLog);
private slots:
    void on_saveLogButton_clicked(void);
};

#endif // DIALOGVLMLOG_H
