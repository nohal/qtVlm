/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#include <QMessageBox>
#include <QMenu>

#include "MainWindow.h"
#include "BoardReal.h"
#include "settings.h"

boardReal::boardReal(MainWindow * mainWin, board * parent) : QWidget(mainWin)
{
    setupUi(this);
    this->mainWin=mainWin;
    this->parent=parent;

    /* Contextual Menu */
    popup = new QMenu(this);
    ac_showHideCompass = new QAction(tr("Cacher compas"),popup);
    popup->addAction(ac_showHideCompass);
    connect(ac_showHideCompass,SIGNAL(triggered()),this,SLOT(slot_hideShowCompass()));

    /* Etat du compass */
    if(Settings::getSetting("boardCompassShown", "1").toInt()==1)
        windAngle->show();
    else
        windAngle->hide();
}

void boardReal::boatUpdated(void)
{

}

void boardReal::setChangeStatus(bool /*status*/)
{

}

void boardReal::paramChanged()
{

}

void boardReal::disp_boatInfo()
{
    QMessageBox::information(this,tr("Information"),"Bientot des infos ici");
}

void boardReal::slot_hideShowCompass()
{
    if(windAngle->isVisible())
    {
        Settings::setSetting("boardCompassShown",0);
        windAngle->hide();
    }
    else
    {
        Settings::setSetting("boardCompassShown",1);
        windAngle->show();
    }
}

void boardReal::contextMenuEvent(QContextMenuEvent  *)
{
    if(windAngle->isVisible())
        ac_showHideCompass->setText(tr("Cacher le compas"));
    else
        ac_showHideCompass->setText(tr("Afficher le compas"));
    popup->exec(QCursor::pos());
}
