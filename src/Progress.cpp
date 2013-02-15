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

#include <QObject>

#include "MainWindow.h"
#include "Progress.h"

Progress::Progress(MainWindow * mainWindow): QProgressDialog(mainWindow,Qt::FramelessWindowHint/*Qt::SplashScreen*/)
{
    this->mainWindow=mainWindow;

    setLabelText(QObject::tr("Starting qtVLM"));
    setMaximum(100);
    setMinimum(0);
    setCancelButton(NULL);
    setMinimumDuration (0);
    setValue(1);
    setAutoClose(false);
    setAutoReset(false);
    show();
    raise();
    activateWindow();
}

void Progress::newStep(const int &step,const QString &msg) {
    setValue(step);
    setLabelText(msg);
#ifdef __MAC_QTVLM
    raise();
    activateWindow();
#endif
}
