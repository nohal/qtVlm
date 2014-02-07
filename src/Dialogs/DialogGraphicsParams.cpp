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
#include <QScroller>
#include "mycentralwidget.h"


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
DialogGraphicsParams::DialogGraphicsParams(myCentralWidget * mcp)
         : QDialog(mcp)
{
    setWindowTitle(tr("Parametres graphiques"));
    this->setObjectName("GraphicSettingsDialog");
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
    ftmp = new QFrame(this);
    ftmp->setFrameShape(QFrame::HLine);
    layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    btOK     = new QPushButton(tr("Valider"), this);
    btCancel = new QPushButton(tr("Annuler"), this);
    layout->addWidget( btOK,    lig,0);
    layout->addWidget( btCancel, lig,1);
    QFrame *wid=new QFrame(this);
    wid->setLayout(layout);
    scroll=new QScrollArea(this);
    scroll->setWidget(wid);
    scroll->setLayout(new QVBoxLayout(this));

    QScroller::grabGesture(this->scroll->viewport());
    Util::setFontDialog(this);
    int h,w,px,py;
    Settings::restoreGeometry(this,&h,&w,&px,&py);
    if(px<=0)
    {
        this->resize(wid->size());
        this->move(this->parentWidget()->window()->frameGeometry().topLeft() +
            this->parentWidget()->window()->rect().center() -
            this->rect().center());
    }
    connect(mcp,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));

    //===============================================================
    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
}
DialogGraphicsParams::~DialogGraphicsParams()
{
    Settings::saveGeometry(this);
}

void DialogGraphicsParams::slot_screenResize()
{
    Util::setWidgetSize(this,this->sizeHint());
}

//-------------------------------------------------------------------------------
void DialogGraphicsParams::resizeEvent ( QResizeEvent * /*event*/ )
{
    scroll->resize(this->size());
}
void DialogGraphicsParams::slotBtOK()
{
        Settings::setSetting(seaColor, inputSeaColor->getColor());
        Settings::setSetting(landColor, inputLandColor->getColor());
        Settings::setSetting(backgroundColor, inputBackgroundColor->getColor());
	
        Settings::setSetting(seaBordersLineWidth, inputSeaBordersLine->getLineWidth());
        Settings::setSetting(seaBordersLineColor, inputSeaBordersLine->getLineColor());
        Settings::setSetting(boundariesLineWidth, inputBoundariesLine->getLineWidth());
        Settings::setSetting(boundariesLineColor, inputBoundariesLine->getLineColor());
        Settings::setSetting(riversLineWidth,   inputRiversLine->getLineWidth());
        Settings::setSetting(riversLineColor,  inputRiversLine->getLineColor());
        Settings::setSetting(isobarsLineWidth,  inputIsobarsLine->getLineWidth());
        Settings::setSetting(isobarsLineColor,  inputIsobarsLine->getLineColor());
        Settings::setSetting(isotherms0LineWidth,  inputIsotherms0Line->getLineWidth());
        Settings::setSetting(isotherms0LineColor,  inputIsotherms0Line->getLineColor());
        Settings::setSetting(nextGateLineWidth,  inputNextGateLine->getLineWidth());
        Settings::setSetting(nextGateLineColor,  inputNextGateLine->getLineColor());
        Settings::setSetting(gateLineWidth,  inputGateLine->getLineWidth());
        Settings::setSetting(gateLineColor,  inputGateLine->getLineColor());
        Settings::setSetting(landOpacity,  inputOpacity->getLineWidth());
        Settings::setSetting(nightOpacity,  inputOpacityNuit->getLineWidth());
        Settings::setSetting(estimeLineWidth,  inputEstimeLine->getLineWidth());
        Settings::setSetting(estimeLineColor,  inputEstimeLine->getLineColor());
        Settings::setSetting(routeLineWidth,  inputRouteLine->getLineWidth());
        Settings::setSetting(routeLineColor,  inputRouteLine->getLineColor());
        Settings::setSetting(traceLineWidth,  inputTraceLine->getLineWidth());
        Settings::setSetting(traceLineColor,  inputTraceLine->getLineColor());
        Settings::setSetting(orthoLineWidth,  inputOrthoLine->getLineWidth());
        Settings::setSetting(orthoLineColor,  inputOrthoLine->getLineColor());
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
                                Settings::getSetting(backgroundColor).value<QColor>(),
                Settings::getSettingDefault(backgroundColor).value<QColor>(),
				this);
    lay->addWidget( inputBackgroundColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des oceans :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaColor =
		new InputColor(
                                Settings::getSetting(seaColor).value<QColor>(),
                Settings::getSettingDefault(seaColor).value<QColor>(),
				this);
    lay->addWidget( inputSeaColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Couleur des terres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputLandColor =
		new InputColor(
                                Settings::getSetting(landColor).value<QColor>(),
                Settings::getSettingDefault(landColor).value<QColor>(),
				this);
    lay->addWidget( inputLandColor, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Opacite des terres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputOpacity =
                new InputLineParams(
                                Settings::getSetting(landOpacity).toDouble(),
                                QColor(Qt::black).value(),
                                Settings::getSettingDefault(landOpacity).toDouble(),  QColor(Qt::black),
                                this,0,255,0,false);

    lay->addWidget( inputOpacity, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Opacite des zones de nuit :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputOpacityNuit =
                new InputLineParams(
                                Settings::getSetting(nightOpacity).toDouble(),
                                QColor(Qt::black).value(),
                                Settings::getSettingDefault(nightOpacity).toDouble(),  QColor(Qt::black),
                                this,0,255,0,false);

    lay->addWidget( inputOpacityNuit, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Estime :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputEstimeLine =
                new InputLineParams(
                                Settings::getSetting(estimeLineWidth).toDouble(),
                                Settings::getSetting(estimeLineColor).value<QColor>(),
                    Settings::getSettingDefault(estimeLineWidth).toDouble(),
                    Settings::getSettingDefault(estimeLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputEstimeLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Traits de cotes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputSeaBordersLine =
		new InputLineParams(
                                Settings::getSetting(seaBordersLineWidth).toDouble(),
                                Settings::getSetting(seaBordersLineColor).value<QColor>(),
                Settings::getSettingDefault(seaBordersLineWidth).toDouble(),
                Settings::getSettingDefault(seaBordersLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputSeaBordersLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Frontieres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputBoundariesLine =
		new InputLineParams(
                                Settings::getSetting(boundariesLineWidth).toDouble(),
                                Settings::getSetting(boundariesLineColor).value<QColor>(),
                Settings::getSettingDefault(boundariesLineWidth).toDouble(),
                Settings::getSettingDefault(boundariesLineColor).value<QColor>(),
				this);
    lay->addWidget( inputBoundariesLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Rivieres :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputRiversLine =
		new InputLineParams(
                                Settings::getSetting(riversLineWidth).toDouble(),
                                Settings::getSetting(riversLineColor).value<QColor>(),
                Settings::getSettingDefault(riversLineWidth).toDouble(),
                Settings::getSettingDefault(riversLineColor).value<QColor>(),
				this);
    lay->addWidget( inputRiversLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Isobares :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
	inputIsobarsLine =
		new InputLineParams(
                                Settings::getSetting(isobarsLineWidth).toDouble(),
                                Settings::getSetting(isobarsLineColor).value<QColor>(),
                Settings::getSettingDefault(isobarsLineWidth).toDouble(),
                Settings::getSettingDefault(isobarsLineColor).value<QColor>(),
				this);
    lay->addWidget( inputIsobarsLine, lig,1, Qt::AlignLeft);
    lig ++;
    label = new QLabel(tr("Isothermes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    inputIsotherms0Line =
        new InputLineParams(
                                Settings::getSetting(isotherms0LineWidth).toDouble(),
                                Settings::getSetting(isotherms0LineColor).value<QColor>(),
                Settings::getSettingDefault(isotherms0LineWidth).toDouble(),
                Settings::getSettingDefault(isotherms0LineColor).value<QColor>(),
                this);
    lay->addWidget( inputIsotherms0Line, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Prochaine porte :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputNextGateLine =
                new InputLineParams(
                                Settings::getSetting(nextGateLineWidth).toDouble(),
                                Settings::getSetting(nextGateLineColor).value<QColor>(),
                    Settings::getSettingDefault(nextGateLineWidth).toDouble(),
                    Settings::getSettingDefault(nextGateLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputNextGateLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Portes suivantes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    QVariant v;
        inputGateLine =
                new InputLineParams(
                                Settings::getSetting(gateLineWidth).toDouble(),
                                Settings::getSetting(gateLineColor).value<QColor>(),
                    Settings::getSettingDefault(gateLineWidth).toDouble(),
                    Settings::getSettingDefault(gateLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputGateLine, lig,1, Qt::AlignLeft);
    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Routes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputRouteLine =
                new InputLineParams(
                                Settings::getSetting(routeLineWidth).toDouble(),
                                Settings::getSetting(routeLineColor).value<QColor>(),
                    Settings::getSettingDefault(routeLineWidth).toDouble(),
                    Settings::getSettingDefault(routeLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputRouteLine, lig,1, Qt::AlignLeft);

    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Trace TWA :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputTraceLine =
                new InputLineParams(
                                Settings::getSetting(traceLineWidth).toDouble(),
                                Settings::getSetting(traceLineColor).value<QColor>(),
                    Settings::getSettingDefault(traceLineWidth).toDouble(),
                    Settings::getSettingDefault(traceLineColor).value<QColor>(),
                                this);
    lay->addWidget( inputTraceLine, lig,1, Qt::AlignLeft);

    //-------------------------------------------------
    lig ++;
    label = new QLabel(tr("Ortho selection :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
        inputOrthoLine =
                new InputLineParams(
                                Settings::getSetting(orthoLineWidth).toDouble(),
                                Settings::getSetting(orthoLineColor).value<QColor>(),
                                1.0,  Qt::red,
                                this);
    lay->addWidget( inputOrthoLine, lig,1, Qt::AlignLeft);
    return frm;
}











