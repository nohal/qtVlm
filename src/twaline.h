#ifndef TWALINE_H
#define TWALINE_H
#include "class_list.h"
#include "ui_twaline.h"
#include "POI.h"

class twaLine : public QDialog, public Ui::twaLine
{ Q_OBJECT
public:
    twaLine(QPointF start, myCentralWidget *parent, MainWindow *main);
    ~twaLine();
    void setStart(QPointF start);
protected:
    void closeEvent(QCloseEvent *event);
private:
    vlmLine * line;
    myCentralWidget *parent;
    QPointF start;
    double twa[5];
    int nbVac[5];
    QList<POI*> list;
    Grib * grib;
    boatAccount * boat;
    void traceIt();
    double A360(double hdg);
    MainWindow * main;
    QPen pen;
    QPoint position;
public slots:
    void slot_delPOI_list(POI *);
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
};

#endif // TWALINE_H
