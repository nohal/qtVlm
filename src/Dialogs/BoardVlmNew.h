#ifndef BOARDVLMNEW_H
#define BOARDVLMNEW_H

#include "class_list.h"

#include "ui_BoardVlmNew.h"
#include "MainWindow.h"
#include <QDialog>


class BoardVlmNew : public QDialog, public Ui::BoardVlmNew
{
    Q_OBJECT
    
public:
    explicit BoardVlmNew(MainWindow *main);
    ~BoardVlmNew();
private slots:
    void slot_vlmSync();
    void slot_updateData();
    void slot_wpChanged();
    void slot_outDatedVlmData();
    void slot_sendOrder();
    void slot_dialChanged(int value);
    void slot_flipAngle();
    void slot_TWAChanged();
    void slot_HDGChanged();
    void slot_timerElapsed();
private:
    MainWindow * main;
    boatVLM * myBoat;
    void updateLcds();
    QString defaultStyleSheet;
    QTimer * timer;
    QLabel * currentRB;
    void timerStop();
    bool blocking;
};

#endif // BOARDVLMNEW_H
