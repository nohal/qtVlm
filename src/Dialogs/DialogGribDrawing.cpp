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

#include "Terrain.h"
#include "mycentralwidget.h"
#include "dataDef.h"
#include "Util.h"

#include "DialogGribDrawing.h"

DialogGribDrawing::DialogGribDrawing(QWidget *parent, myCentralWidget *centralWidget) : QDialog(parent) {
    this->centralWidget=centralWidget;
    this->terrain=centralWidget->getTerre();

    setupUi(this);
    Util::setFontDialog(this);

    init_stringList();

    init_state();
    show();

}

void DialogGribDrawing::init_state(void) {

}

void DialogGribDrawing::done(int result) {

}

void DialogGribDrawing::slot_bgDataType(int) {

}

void DialogGribDrawing::slot_bgDataAlt(int) {

}

void DialogGribDrawing::slot_frstArwType(int) {

}

void DialogGribDrawing::slot_showTemp(bool) {

}

void DialogGribDrawing::slot_smooth(bool) {

}

void DialogGribDrawing::slot_frstArwAlt(int) {

}

void DialogGribDrawing::slot_secArwType(int) {

}

void DialogGribDrawing::slot_secArwAlt(int) {

}

void DialogGribDrawing::slot_showBarbule(bool) {

}

void DialogGribDrawing::slot_showIsoBar(bool) {

}

void DialogGribDrawing::slot_showIsoTherm(bool) {

}

void DialogGribDrawing::slot_isoBarSpacing(int) {

}

void DialogGribDrawing::slot_isoThermSpacing(int) {

}

void DialogGribDrawing::slot_isoBarShowLabel(bool) {

}

void DialogGribDrawing::slot_isoThermShowLabel(bool) {

}

void DialogGribDrawing::slot_isoBarShowMinMax(bool) {

}

void DialogGribDrawing::init_stringList(void) {

    /* data types */
    dataTypes.append(tr("Aucun"));
    dataTypes.append(tr("Carte du vent"));
    dataTypes.append(tr("Carte du courant"));
    dataTypes.append(tr("Couverture nuageuse"));
    dataTypes.append(tr("Carte des preecipitations"));
    dataTypes.append(tr("Carte de l'humidite relative"));
    dataTypes.append(tr("Carte de la temperature"));
    dataTypes.append(tr("Carte de la temperature potentielle"));
    dataTypes.append(tr("Point de rosee"));
    dataTypes.append(tr("Ecart temperature-point de rosee"));
    dataTypes.append(tr("Neige (chute possible)"));
    dataTypes.append(tr("Pluie verglacante (chute possible)"));
    dataTypes.append(tr("CAPE (surface)"));
    dataTypes.append(tr("CIN (surface)"));
    dataTypes.append(tr("Waves combined"));
    dataTypes.append(tr("Wind waves"));
    dataTypes.append(tr("Swell waves"));
    dataTypes.append(tr("Max waves"));
    dataTypes.append(tr("White cap prob"));

    /* arrow types */
    arrowTypesFst.append(tr("Aucun"));
    arrowTypesFst.append(tr("Vent"));
    arrowTypesFst.append(tr("Courant"));

    arrowTypesSec.append(tr("Aucun"));
    arrowTypesSec.append(tr("Courant"));
    arrowTypesSec.append(tr("Waves combined"));
    arrowTypesSec.append(tr("Swell waves"));
    arrowTypesSec.append(tr("Max waves"));
    arrowTypesSec.append(tr("Primary waves"));
    arrowTypesSec.append(tr("Secondary waves"));

    /* levels */
    levelTypes.append(tr("Surface"));
    levelTypesUnit.append("unit ?");
    levelTypes.append(tr("Isotherm 0C"));
    levelTypesUnit.append("unit ?");
    levelTypes.append(tr("Isobaric"));
    levelTypesUnit.append("hPa");
    levelTypes.append(tr("Mean Sea Level"));
    levelTypesUnit.append("unit ?");
    levelTypes.append(tr("Above ground"));
    levelTypesUnit.append("m");
    levelTypes.append(tr("Sigma"));
    levelTypesUnit.append("?");
    levelTypes.append(tr("Entire atmosphere"));
    levelTypesUnit.append("unit ?");
    levelTypes.append(tr("Ordered sequence"));
    levelTypesUnit.append("?");
}
