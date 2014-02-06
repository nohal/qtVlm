#ifndef TWALINE_H
#define TWALINE_H
#include "class_list.h"
#include "ui_twaline.h"
#include "POI.h"

class DialogTwaLine : public QDialog, public Ui::twaLine
{ Q_OBJECT
public:
    DialogTwaLine(QPointF start, myCentralWidget *parent, MainWindow *main);
    ~DialogTwaLine();
    void setStart(QPointF start);
protected:
    void closeEvent(QCloseEvent *event);
private:
    vlmLine * line;
    myCentralWidget *parent;
    DataManager * dataManager;
    QPointF start;
    double twa[5];
    int nbVac[5];
    bool mode[5];
    QList<POI*> list;
    boat * myBoat;
    void traceIt();
    MainWindow * main;
    QPen pen;
    QPoint position;
    QColor color;
public slots:
    void slot_delPOI_list(POI *);
    void slotTwa1(bool b);
    void slotTwa2(bool b);
    void slotTwa3(bool b);
    void slotTwa4(bool b);
    void slotTwa5(bool b);
    void slot_screenResize();
private slots:
    void on_spinBox_5_valueChanged(int );
    void on_doubleSpinBox_5_valueChanged(double );
    void on_spinBox_4_valueChanged(int );
    void on_doubleSpinBox_4_valueChanged(double );
    void on_spinBox_3_valueChanged(int );
    void on_doubleSpinBox_3_valueChanged(double );
    void on_spinBox_2_valueChanged(int );
    void on_doubleSpinBox_2_valueChanged(double );
    void on_spinBox_valueChanged(int );
    void on_doubleSpinBox_valueChanged(double );
    void on_startGrib_clicked(void);
    void on_startVac_clicked(void);
};

#endif // TWALINE_H
