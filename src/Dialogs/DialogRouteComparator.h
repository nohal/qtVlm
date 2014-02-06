#ifndef DIALOGROUTECOMPARATOR_H
#define DIALOGROUTECOMPARATOR_H

#include <QDialog>

#include "class_list.h"

#include "ui_RouteComparator.h"
#include "mycentralwidget.h"
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTimer>

//===================================================================
class DialogRouteComparator : public QDialog, public Ui::DialogRouteComparator
{
    Q_OBJECT
    
public:
    explicit DialogRouteComparator(myCentralWidget *parent = 0);
    ~DialogRouteComparator();
public slots:
    void slot_screenResize();
private slots:
    void slot_deleteRoute();
    void slot_removeRoute();
    void slot_contextMenu(QPoint P);
    void slot_insertRoute(int i);
private:
    void insertRoute(const int &n);
    QStandardItemModel * model;
    myCentralWidget *mcw;
    QStandardItem *item;
};

#endif // DIALOGROUTECOMPARATOR_H
