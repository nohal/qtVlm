#ifndef DIALOGVIEWPOLAR_H
#define DIALOGVIEWPOLAR_H

#include <QtWidgets/QDialog>
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
};
#endif // DIALOGVIEWPOLAR_H
