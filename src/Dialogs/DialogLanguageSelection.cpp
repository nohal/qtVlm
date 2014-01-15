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

#include "DialogLanguageSelection.h"

#include "settings.h"



DialogLanguageSelection::DialogLanguageSelection(QWidget *parent): QDialog(parent)
{
    setupUi(this);

    QString lang=Settings::getSetting(appLanguage).toString();

    if(lang=="NO") lang="en";

    if(lang=="en")
        rd_english->setChecked(true);
    if(lang=="fr")
        rd_french->setChecked(true);
    if(lang=="es")
        rd_spanish->setChecked(true);
    if(lang=="cz")
        rd_tchec->setChecked(true);
}

void DialogLanguageSelection::done(int result) {
    if(result == QDialog::Accepted) {
        QString lang;
        if(rd_english->isChecked())
            lang="en";
        if(rd_french->isChecked())
            lang="fr";
        if(rd_spanish->isChecked())
            lang="es";
        if(rd_tchec->isChecked())
            lang="cz";
        Settings::setSetting(appLanguage,lang);
    }
    QDialog::done(result);
}
