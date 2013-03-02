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
#ifdef QT_V5
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDesktopWidget>
#else
#include <QMessageBox>
#include <QColorDialog>
#include <QDesktopWidget>
#endif
#include <cmath>
#include <cassert>

#include <QPainter>

#include "DialogGraphicsParams.h"
#include "settings.h"
#include <QDebug>
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

        bdDefault = new QPushButton(tr("Valeurs par defauts"), this);
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
InputLineParams::InputLineParams(double width, QColor color,
                                                double defaultWidth, QColor defaultColor, QWidget *parent,
                                                double minWidth, double maxWidth,int decimal,bool useTestZone)
		: QWidget(parent)
{
	this->defaultWidth = defaultWidth;
	this->defaultColor = defaultColor;
        this->useTestZone = useTestZone;
	QHBoxLayout *layout = new QHBoxLayout;

    sbWidth = new QDoubleSpinBox(this);

    	sbWidth->setRange(minWidth, maxWidth);
        sbWidth->setDecimals(decimal);
        if (decimal!=0)
                sbWidth->setSingleStep(0.2);
        else
                sbWidth->setSingleStep(1);
        sbWidth->setValue(width);
        layout->addWidget(sbWidth);
        if(useTestZone)
        {
            testZone = new InputLineParams_testZone(width, color);
            testZone->setMinimumSize(220, 20);
            layout->addWidget(testZone);
        }

        bdDefault = new QPushButton(tr("Valeurs par defauts"), this);
	layout->addWidget(bdDefault);
	
	setLayout(layout);
	connect(sbWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged()));
	connect(bdDefault, SIGNAL(clicked()), this, SLOT(resetDefault()));
}
//----------------------------------------------------------------------
void InputLineParams::lineWidthChanged()
{
    if(!useTestZone) return;
	testZone->setWidth(sbWidth->value());
}
//----------------------------------------------------------------------
void InputLineParams::resetDefault()
{
	sbWidth->setValue(defaultWidth);
        if(useTestZone)
        {
            testZone->setWidth(defaultWidth);
            testZone->setColor(defaultColor);
        }
}

//===========================================================================
// DialogGraphicsParams
//===========================================================================
DialogGraphicsParams::DialogGraphicsParams()
		 : QDialog()
{
    setWindowTitle(tr("Parametres graphiques"));
    QFrame *ftmp;
    QLabel *label;
    frameGui = createFrameGui(this);
    layout = new QGridLayout(this);
    int lig=0;
    //-------------------------
    lig ++;
    QFont font;
    font.setBold(true);
    label = new QLabel(tr("Parametres graphiques"), this);
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
    Util::setFontDialog(this);
    QWidget *wid=new QWidget();
    wid->setLayout(layout);
    scroll=new QScrollArea(this);
    this->resize(wid->size());
    scroll->resize(wid->size());
    scroll->setWidget(wid);
    QSize mySize=QSize(wid->size().width()+20,wid->size().height()+20);
    QSize screenSize=QApplication::desktop()->screenGeometry().size()*.8;
    if(mySize.height() > screenSize.height())
    {
        mySize.setHeight(screenSize.height());
    }
    if(mySize.width() > screenSize.width())
    {
        mySize.setWidth(screenSize.width());
    }
    this->resize(mySize);
    scroll->resize(mySize);

    //===============================================================
    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
}

//-------------------------------------------------------------------------------
void DialogGraphicsParams::resizeEvent ( QResizeEvent * /*event*/ )
{
    scroll->resize(this->size());
}
void DialogGraphicsParams::slotBtOK()
{
        Settings::setSetting("seaColor", inputSeaColor->getColor());
        Settings::setSetting("landColor", inputLandColor->getColor());
        Settings::setSetting("backgroundColor", inputBackgroundColor->getColor());
	
        Settings::setSetting("seaBordersLineWidth", inputSeaBordersLine->getLineWidth());
        Settings::setSetting("seaBordersLineColor", inputSeaBordersLine->getLineColor());
        Settings::setSetting("boundariesLineWidth", inputBoundariesLine->getLineWidth());
        Settings::setSetting("boundariesLineColor", inputBoundariesLine->getLineColor());
        Settings::setSetting("riversLineWidth",   inputRiversLine->getLineWidth());
        Settings::setSetting("riversLineColor",   inputRiversLine->getLineColor());
        Settings::setSetting("isobarsLineWidth",  inputIsobarsLine->getLineWidth());
        Settings::setSetting("isobarsLineColor",  inputIsobarsLine->getLineColor());
        Settings::setSetting("nextGateLineWidth",  inputNextGateLine->getLineWidth());
        Settings::setSetting("nextGateLineColor",  inputNextGateLine->getLineColor());
        Settings::setSetting("gateLineWidth",  inputGateLine->getLineWidth());
        Settings::setSetting("gateLineColor",  inputGateLine->getLineColor());
        Settings::setSetting("landOpacity",  inputOpacity->getLineWidth());
        Settings::setSetting("estimeLineWidth",  inputEstimeLine->getLineWidth());
        Settings::setSetting("estimeLineColor",  inputEstimeLine->getLineColor());
        Settings::setSetting("routeLineWidth",  inputRouteLine->getLineWidth());
        Settings::setSetting("routeLineColor",  inputRouteLine->getLineColor());
        Settings::setSetting("traceLineWidth",  inputTraceLine->getLineWidth());
        Settings::setSetting("traceLineColor",  inputTraceLine->getLineColor());
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
                                Settings::getSetting("backgroundColor", QColor(0,0,45)).value<QColor>(),
				QColor(0,0,45),
				this);
    lay->addWidget( inputBackgroundColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des oceans :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaColor =
		new InputColor(
                                Settings::getSetting("seaColor", QColor(50,50,150)).value<QColor>(),
				QColor(50,50,150),
				this);
    lay->addWidget( inputSeaColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des terres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputLandColor =
		new InputColor(
                                Settings::getSetting("landColor", QColor(200,200,120)).value<QColor>(),
				QColor(200,200,120),
				this);
    lay->addWidget( inputLandColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Opacite des terres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputOpacity =
                new InputLineParams(
                                Settings::getSetting("landOpacity", 180).toDouble(),
                                QColor(Qt::black).value(),
                                180,  QColor(Qt::black),
                                this,0,255,0,false);

    lay->addWidget( inputOpacity, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Estime :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputEstimeLine =
                new InputLineParams(
                                Settings::getSetting("estimeLineWidth", 1.6).toDouble(),
                                Settings::getSetting("estimeLineColor", QColor(Qt::darkMagenta)).value<QColor>(),
                                1.6,  QColor(Qt::darkMagenta),
                                this);
    lay->addWidget( inputEstimeLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Traits de cotes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaBordersLine =
		new InputLineParams(
                                Settings::getSetting("seaBordersLineWidth", 1.8).toDouble(),
                                Settings::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>(),
				1.8,  QColor(40,45,30),
                                this);
    lay->addWidget( inputSeaBordersLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Frontieres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputBoundariesLine =
		new InputLineParams(
                                Settings::getSetting("boundariesLineWidth", 1.4).toDouble(),
                                Settings::getSetting("boundariesLineColor", QColor(40,40,40)).value<QColor>(),
				1.4,  QColor(40,40,40),
				this);
    lay->addWidget( inputBoundariesLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Rivieres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputRiversLine =
		new InputLineParams(
                                Settings::getSetting("riversLineWidth", 1.0).toDouble(),
                                Settings::getSetting("riversLineColor", QColor(50,50,150)).value<QColor>(),
				1.0,  QColor(50,50,150),
				this);
    lay->addWidget( inputRiversLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Isobares :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputIsobarsLine =
		new InputLineParams(
                                Settings::getSetting("isobarsLineWidth", 2.0).toDouble(),
                                Settings::getSetting("isobarsLineColor", QColor(80,80,80)).value<QColor>(),
				2.0,  QColor(80,80,80),
				this);
    lay->addWidget( inputIsobarsLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Prochaine porte :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputNextGateLine =
                new InputLineParams(
                                Settings::getSetting("nextGateLineWidth", 3.0).toDouble(),
                                Settings::getSetting("nextGateLineColor", QColor(Qt::blue)).value<QColor>(),
                                2.0,  Qt::blue,
                                this);
    lay->addWidget( inputNextGateLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Portes suivantes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    QVariant v;
        inputGateLine =
                new InputLineParams(
                                Settings::getSetting("gateLineWidth", 3.0).toDouble(),
                    Settings::getSetting("gateLineColor", QColor(Qt::magenta)).value<QColor>(),
                                2.0,  Qt::magenta,
                                this);
    lay->addWidget( inputGateLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Routes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputRouteLine =
                new InputLineParams(
                                Settings::getSetting("routeLineWidth", 2.0).toDouble(),
                                Settings::getSetting("routeLineColor", QColor(Qt::yellow)).value<QColor>(),
                                2.0,  Qt::yellow,
                                this);
    lay->addWidget( inputRouteLine, lig,1, Qt::AlignLeft);

    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Trace TWA :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputTraceLine =
                new InputLineParams(
                                Settings::getSetting("traceLineWidth", 2.0).toDouble(),
                                Settings::getSetting("traceLineColor", QColor(Qt::yellow)).value<QColor>(),
                                2.0,  Qt::yellow,
                                this);
    lay->addWidget( inputTraceLine, lig,1, Qt::AlignLeft);

    return frm;
}











