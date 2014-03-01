#ifndef INFOVIEW_H
#define INFOVIEW_H
#include "class_list.h"
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>

class InfoView : public QLabel
{
    Q_OBJECT
public:
    explicit InfoView(myCentralWidget * parent);
    void showView(const QString &text, const bool &animate);
    void hideView();
signals:

public slots:
    void slot_finished();
private:
    myCentralWidget * parent;
    MainWindow * main;
    //QLabel * label;
    QPropertyAnimation *animation;
};

#endif // INFOVIEW_H
