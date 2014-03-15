#include "InfoView.h"
#include "mycentralwidget.h"
#include <QLayout>
#include "MenuBar.h"
InfoView::InfoView(myCentralWidget *parent) :
    QLabel(parent)
{
    this->parent=parent;
    this->main=parent->getMainWindow();
    this->setTextFormat(Qt::RichText);
    this->setBackgroundRole(QPalette::Window);
    this->setAutoFillBackground(true);
    this->hide();
    this->setContentsMargins(5,5,5,5);
    animation=new QPropertyAnimation(this,"pos");
    animation->setDuration(700);
    connect(animation,SIGNAL(finished()),this,SLOT(slot_finished()));
    Util::setFontObject(this);
}
void InfoView::showView(const QString &text, const bool &animate, QList<QMenu*> *newMenu)
{
#ifndef __ANDROID__
    Q_UNUSED(text);
    Q_UNUSED(animate);
    Q_UNUSED(newMenu);
    return;
#endif
    parent->get_menuBar()->setNewMenu(newMenu);
    if(text.isEmpty())
    {
        if(!this->text().isEmpty())
            this->hideView();
        return;
    }
    this->setText(text);
    this->adjustSize();
    if(animate)
    {
        animation->setStartValue(QPoint((parent->width()/2)-(this->width()/2),parent->height()));
        animation->setEndValue(QPoint((parent->width()/2)-(this->width()/2),parent->height()-this->height()));
        animation->start();
    }
    show();
}
void InfoView::hideView()
{
    parent->get_menuBar()->setNewMenu(NULL);
    animation->setEndValue(QPoint(this->pos().x(),parent->height()));
    animation->setStartValue(this->pos());
    animation->start();
}
void InfoView::slot_finished()
{
}
