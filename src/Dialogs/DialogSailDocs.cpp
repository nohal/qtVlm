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

#include "DialogSailDocs.h"
#include "Util.h"
#include "settings.h"
#ifdef QT_V5
#include <QScroller>
#endif
#include "mycentralwidget.h"
DialogSailDocs::DialogSailDocs(QString param, myCentralWidget * parent) : QDialog(parent->getMainWindow())
{
    setupUi(this);
#ifdef QT_V5
    QScroller::grabGesture(this->scrollArea->viewport());
#endif
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    this->param->setText(param);
    this->param->setFocus();
    this->param->selectAll();
}
void DialogSailDocs::slot_screenResize()
{
    Util::setWidgetSize(this);
}
DialogSailDocs::~DialogSailDocs()
{
    Settings::saveGeometry(this);
}

