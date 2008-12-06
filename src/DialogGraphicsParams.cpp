/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <QMessageBox>
#include <cmath>
#include <cassert>

#include <QPainter>
#include <QColorDialog>

#include "DialogGraphicsParams.h"
#include "Util.h"


//===========================================================================
// InputColor
//===========================================================================
InputColor_testZone::InputColor_testZone(QColor color, QWidget *parent)
		: QWidget(parent)
{
	this->color = color;
}
//----------------------------------------------------------------------
void InputColor_testZone::mouseReleaseEvent(QMouseEvent * )
{
	// Open Choose color dialog
	QColor col = QColorDialog::getColor(color, this);
	if (col.isValid()) {
		color = col;
		update();
	}
}
//----------------------------------------------------------------------
void InputColor_testZone::paintEvent(QPaintEvent *)
{
	QPainter pnt(this);
	pnt.fillRect(0,0, width(), height(), QBrush(color));
}

//----------------------------------------------------------------------
InputColor::InputColor(QColor color, QColor defaultColor, QWidget *parent)
		: QWidget(parent)
{
	this->defaultColor = defaultColor;
	
	QHBoxLayout *layout = new QHBoxLayout;
	testZone = new InputColor_testZone(color, this);
	testZone->setMinimumSize(220, 20);
	layout->addWidget(testZone);

	bdDefault = new QPushButton(tr("Valeurs par défauts"), this);
	layout->addWidget(bdDefault);
	
	setLayout(layout);
	connect(bdDefault, SIGNAL(clicked()), this, SLOT(resetDefault()));
}
//----------------------------------------------------------------------
void InputColor::resetDefault()
{
	testZone->setColor(defaultColor);
}

//===========================================================================
// InputLineParams
//===========================================================================
void InputLineParams_testZone::paintEvent(QPaintEvent *)
{
	QPainter pnt(this);
	pnt.setRenderHint(QPainter::Antialiasing, true);
	
	QLinearGradient linearGrad(QPointF(0, 0), QPointF(width()-1, 0));
 	linearGrad.setColorAt(0, Qt::black);
 	linearGrad.setColorAt(1, Qt::white);
	pnt.fillRect(0,0, width(), height(), QBrush(linearGrad));

	QPen pen(lineColor);
	pen.setWidthF(lineWidth);
	pnt.setPen(pen);
	pnt.drawLine(0,height()-1, width()-1,0);
}
//----------------------------------------------------------------------
void InputLineParams_testZone::mouseReleaseEvent(QMouseEvent * )
{
	// Open Choose color dialog
	QColor col = QColorDialog::getColor(lineColor, this);
	if (col.isValid()) {
		lineColor = col;
		update();
	}
}
//----------------------------------------------------------------------
InputLineParams::InputLineParams(float width, QColor color,
						float defaultWidth, QColor defaultColor, QWidget *parent,
						float minWidth, float maxWidth)
		: QWidget(parent)
{
	this->defaultWidth = defaultWidth;
	this->defaultColor = defaultColor;

	QHBoxLayout *layout = new QHBoxLayout;

    sbWidth = new QDoubleSpinBox(this);
    	sbWidth->setValue(width);
    	sbWidth->setRange(minWidth, maxWidth);
    	sbWidth->setDecimals(1);
    	sbWidth->setSingleStep(0.2);
	layout->addWidget(sbWidth);
	
	testZone = new InputLineParams_testZone(width, color);
		testZone->setMinimumSize(220, 20);
	layout->addWidget(testZone);

	bdDefault = new QPushButton(tr("Valeurs par défauts"), this);
	layout->addWidget(bdDefault);
	
	setLayout(layout);
	connect(sbWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged()));
	connect(bdDefault, SIGNAL(clicked()), this, SLOT(resetDefault()));
}
//----------------------------------------------------------------------
void InputLineParams::lineWidthChanged()
{
	testZone->setWidth(sbWidth->value());
}
//----------------------------------------------------------------------
void InputLineParams::resetDefault()
{
	sbWidth->setValue(defaultWidth);
	testZone->setWidth(defaultWidth);
	testZone->setColor(defaultColor);
}

//===========================================================================
// DialogGraphicsParams
//===========================================================================
DialogGraphicsParams::DialogGraphicsParams()
		 : QDialog()
{
    setWindowTitle(tr("Paramètres graphiques"));
    QFrame *ftmp;
    QLabel *label;
    frameGui = createFrameGui(this);
    
    layout = new QGridLayout(this);
    int lig=0;
    //-------------------------
    lig ++;
    QFont font;
    font.setBold(true);
    label = new QLabel(tr("Paramètres graphiques"), this);
    label->setFont(font);
    layout->addWidget( label,    lig,0, 1,-1, Qt::AlignCenter);
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    layout->addWidget( frameGui,  lig,0,   1, 2);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    btOK     = new QPushButton(tr("Valider"), this);
    btCancel = new QPushButton(tr("Annuler"), this);
    layout->addWidget( btOK,    lig,0);
    layout->addWidget( btCancel, lig,1);
    
    //===============================================================
    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
}

//-------------------------------------------------------------------------------
void DialogGraphicsParams::slotBtOK()
{
	Util::setSetting("seaColor", inputSeaColor->getColor());
	Util::setSetting("landColor", inputLandColor->getColor());
	Util::setSetting("backgroundColor", inputBackgroundColor->getColor());
	
	Util::setSetting("seaBordersLineWidth", inputSeaBordersLine->getLineWidth());
	Util::setSetting("seaBordersLineColor", inputSeaBordersLine->getLineColor());
	Util::setSetting("boundariesLineWidth", inputBoundariesLine->getLineWidth());
	Util::setSetting("boundariesLineColor", inputBoundariesLine->getLineColor());
	Util::setSetting("riversLineWidth",   inputRiversLine->getLineWidth());
	Util::setSetting("riversLineColor",   inputRiversLine->getLineColor());
	Util::setSetting("isobarsLineWidth",  inputIsobarsLine->getLineWidth());
	Util::setSetting("isobarsLineColor",  inputIsobarsLine->getLineColor());
    accept();
}
//-------------------------------------------------------------------------------
void DialogGraphicsParams::slotBtCancel()
{
    reject();
}

//=============================================================================
// GUI
//=============================================================================
QFrame *DialogGraphicsParams::createFrameGui(QWidget *parent)
{
    QFrame * frm = new QFrame(parent);
    QLabel * label;
    QGridLayout  *lay = new QGridLayout(frm);
    int lig=0;
    
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur de fond :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputBackgroundColor =
		new InputColor(
				Util::getSetting("backgroundColor", QColor(0,0,45)).value<QColor>(),
				QColor(0,0,45),
				this);
    lay->addWidget( inputBackgroundColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des océans :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaColor =
		new InputColor(
				Util::getSetting("seaColor", QColor(50,50,150)).value<QColor>(),
				QColor(50,50,150),
				this);
    lay->addWidget( inputSeaColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des terres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputLandColor =
		new InputColor(
				Util::getSetting("landColor", QColor(200,200,120)).value<QColor>(),
				QColor(200,200,120),
				this);
    lay->addWidget( inputLandColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Traits de côtes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaBordersLine =
		new InputLineParams(
				Util::getSetting("seaBordersLineWidth", 1.8).toDouble(),
				Util::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>(),
				1.8,  QColor(40,45,30),
				this);
    lay->addWidget( inputSeaBordersLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Frontières :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputBoundariesLine =
		new InputLineParams(
				Util::getSetting("boundariesLineWidth", 1.4).toDouble(),
				Util::getSetting("boundariesLineColor", QColor(40,40,40)).value<QColor>(),
				1.4,  QColor(40,40,40),
				this);
    lay->addWidget( inputBoundariesLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Rivières :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputRiversLine =
		new InputLineParams(
				Util::getSetting("riversLineWidth", 1.0).toDouble(),
				Util::getSetting("riversLineColor", QColor(50,50,150)).value<QColor>(),
				1.0,  QColor(50,50,150),
				this);
    lay->addWidget( inputRiversLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Isobares :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputIsobarsLine =
		new InputLineParams(
				Util::getSetting("isobarsLineWidth", 2.0).toDouble(),
				Util::getSetting("isobarsLineColor", QColor(80,80,80)).value<QColor>(),
				2.0,  QColor(80,80,80),
				this);
    lay->addWidget( inputIsobarsLine, lig,1, Qt::AlignLeft);
    
    return frm;
}











