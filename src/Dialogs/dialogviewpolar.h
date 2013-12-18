#ifndef DIALOGVIEWPOLAR_H
#define DIALOGVIEWPOLAR_H
#ifdef QT_V5
#include <QtWidgets/QDialog>
#else
#include <QDialog>
#endif
#include <QPainter>
#include "class_list.h"
#include "ui_dialogviewpolar.h"
#include "boat.h"
#include "Polar.h"

class DialogViewPolar;

class DialogViewPolar : public QDialog, public Ui::DialogViewPolar
{
    Q_OBJECT

public:
    explicit DialogViewPolar(QWidget *parent = 0);
    ~DialogViewPolar();
    void setBoat(boat * myboat);
    bool eventFilter(QObject *obj, QEvent *event);
public slots:
    void drawIt();
    void reloadPolar();
private:
    QPixmap image;
    QPen pen;
    QPainter pnt;
    boat * myBoat;
    QPolygonF polarLine,polarGreen;
    QList<double> polarValues;
    QString stringMaxSpeed;
};
#endif // DIALOGVIEWPOLAR_H
