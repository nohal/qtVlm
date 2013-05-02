/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "BoardComponent.h"
#include "MainWindow.h"

BoardComponent::BoardComponent(MainWindow * mainWindow): QWidget(mainWindow) {
    this->mainWindow=mainWindow;
    chgingVisibility=false;
}

BoardComponent::~BoardComponent() {

}

void BoardComponent::init_dock(QString title,int typeNum) {
    name=title;
    this->typeNum=typeNum;
    displayed=true;
    dockWidget=new MyDockWidget(this,title);
    dockWidget->setWidget(this);
    dockWidget->setContentsMargins(QMargins(0,0,0,0));

    connect(dockWidget,SIGNAL(visibilityChanged(bool)),this,SLOT(slot_visibilityChanged(bool)));
}

void BoardComponent::apply_displayed(bool displayed) {
    chgingVisibility=true;
    this->displayed=displayed;
    if(displayed) {
        mainWindow->addDockWidget(Qt::LeftDockWidgetArea,dockWidget);
        dockWidget->show();
    }
    else
        mainWindow->removeDockWidget(dockWidget);
    chgingVisibility=false;
}

void BoardComponent::set_visible(bool visible) {
    chgingVisibility=true;
    if(visible) {
        if(displayed) {
            mainWindow->addDockWidget(Qt::LeftDockWidgetArea,dockWidget);
            dockWidget->show();
        }
    }
    else {
        mainWindow->removeDockWidget(dockWidget);
    }
    chgingVisibility=false;
}

void BoardComponent::slot_visibilityChanged(bool displayed) {

    if(displayed)
        set_displayed(displayed);
}

/****************************************/
/* MyDockWidget                         */
/****************************************/

MyDockWidget::MyDockWidget(BoardComponent * boardComponent,QString title) : QDockWidget(title) {
    this->boardComponent=boardComponent;
}

void MyDockWidget::closeEvent(QCloseEvent *event) {
    boardComponent->set_displayed(false);
    QDockWidget::closeEvent(event);
}

