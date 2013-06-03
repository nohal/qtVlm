#ifndef BOARDVLMNEW_H
#define BOARDVLMNEW_H

#include "class_list.h"
#include "BoardTools.h"
#include "ui_BoardVlmNew.h"
#include "MainWindow.h"
#include <QPainter>
#include <QDialog>


class BoardVlmNew : public QDialog, public Ui::BoardVlmNew
{
    Q_OBJECT
    
public:
    explicit BoardVlmNew(MainWindow *main);
    ~BoardVlmNew();
    static bool confirmChange();
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
    void slot_timerDialElapsed();
    void slot_clearPilototo();
    void slot_editWP();
    void slot_clearWP();
    void slot_drawPolar();
    void slot_tabChanged(int tabNb);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
public slots:
    void slot_updateBtnWP();
    void slot_selectPOI(bool doSelect);
    void slot_selectPOI(POI *);
    void slot_selectWP_POI();
    void slot_lock();
    void slot_reloadSkin();
private:
    MainWindow * main;
    boatVLM * myBoat;
    void updateLcds();
    QString defaultStyleSheet;
    QTimer * timer;
    QTimer * timerDial;
    QLabel * currentRB;
    void timerStop();
    bool blocking;
    DialogWp * wpDialog;
    double computeAngle();
    void update_btnPilototo();
    void set_style(QPushButton * button, QColor color=QColor(230,230,230), QColor color2=Qt::white);
    void set_enabled(const bool &b);
    QPainter polarPnt;
    QPixmap polarImg;
    QPolygonF polarLine;
    QList<double> polarValues;
    QPixmap imgBack0,imgBack1,imgBack2;
};

#endif // BOARDVLMNEW_H
