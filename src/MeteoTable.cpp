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

#include "MeteoTable.h"
#include "GribPlot.h"
#include "Util.h"


//-------------------------------------------------------------------------------
MeteoTableWidget::MeteoTableWidget(GribPlot *gribplot, float lon, float lat, QWidget *parent)
	: QWidget(parent)
{
	this->gribplot = gribplot;
	this->lon = lon;
	this->lat = lat;

	layout = new QGridLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	
	createTable();
}

//-------------------------------------------------------------------------------
void MeteoTableWidget::createTable()
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	std::set<time_t>::iterator iter2;
	int lig, col, colspan;
	QString dstr;
	//-----------------------------------------------
	// Titre 1 : une colonne par jour, regroupant plusieurs horaires
	//-----------------------------------------------
	col = 0;
	lig = 0;
	addCell_title("", true, layout, lig,col, 2,1);
	col ++;
	QString actuel = "";
	for (iter=sdates.begin(); iter!=sdates.end(); iter++)
	{
		dstr = Util::formatDateLong(*iter);
		if (dstr != actuel)
		{
			colspan = 0;
			actuel = dstr;
			iter2 = iter;
			do
			{
				colspan ++;
				iter2 ++;
				dstr = Util::formatDateLong(*iter2);
			//printf("%s : %d\n", qPrintable(Util::formatDateTimeShort(*iter2)),colspan);
			} while (actuel==dstr);
			
			addCell_title(actuel, true, layout, lig,col, 1,colspan);
			col += colspan;
		}
	}
	//-----------------------------------------------
	// Titre 2 : une colonne par date+horaires
	//-----------------------------------------------
	col = 1;
	lig = 1;
	for (iter=sdates.begin(); iter!=sdates.end(); iter++)
	{
		addCell_title(Util::formatDateTime_hour(*iter), false, layout, lig,col);
		col ++;
	}
	//-----------------------------------------------
	// Contenus
	//-----------------------------------------------
	lig++;
	addLine_Wind(lig);
	lig++;
	addLine_CloudCover(lig);
	lig++;
	addLine_Rain(lig);
	lig++;
	addLine_Temperature(lig);
	lig++;
	addLine_Pressure(lig);
}


//-----------------------------------------------------------------
void MeteoTableWidget::addLine_Pressure(int lig)
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	time_t now;
	int col;
	GribRecord *rec;
	QString   txt;
	GribPlot  plot;
	QColor    bgColor = Qt::white;
	col = 0;
	addCell_title(tr("Pression"), true, layout, lig,col);
	col ++;	
	for (iter=sdates.begin(); iter!=sdates.end(); iter++, col++)
	{
		now = *iter;
		rec = gribread->getGribRecord(GRB_PRESS_MSL, now);
		txt = "";
		if (rec) {
			float v = rec->getInterpolatedValue(lon, lat);
			if (v != GRIB_NOTDEF) {
				v = v/100.0;
				txt.sprintf("%.1f %s", v, qPrintable(tr("hPa")));
				bgColor = QColor(plot.getPressureColor(v, true));
			}
		}
		addCell_content(txt, layout,lig,col, 1,1, bgColor);
	}
}
//-----------------------------------------------------------------
void MeteoTableWidget::addLine_Temperature(int lig)
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	time_t now;
	int col;
	GribRecord *rec;
	QString   txt;
	GribPlot  plot;
	QColor    bgColor = Qt::white;
	col = 0;
	addCell_title(tr("Température"), true, layout, lig,col);
	col ++;	
	for (iter=sdates.begin(); iter!=sdates.end(); iter++, col++)
	{
		now = *iter;
		rec = gribread->getGribRecord(GRB_TEMP, now);
		txt = "";
		if (rec) {
			float v = rec->getInterpolatedValue(lon, lat);
			if (v != GRIB_NOTDEF) {
				txt = Util::formatTemperature(v);
				bgColor = QColor(plot.getTemperatureColor(v, true));
			}
		}
		addCell_content(txt, layout,lig,col, 1,1, bgColor);
	}
}
//-----------------------------------------------------------------
void MeteoTableWidget::addLine_Rain(int lig)
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	time_t now;
	int col;
	GribRecord *rec;
	QString   txt;
	GribPlot  plot;
	QColor    bgColor = Qt::white;
	col = 0;
	addCell_title(tr("Précipitations"), true, layout, lig,col);
	col ++;
	for (iter=sdates.begin(); iter!=sdates.end(); iter++, col++)
	{
		now = *iter;
		rec = gribread->getGribRecord(GRB_PRECIP_TOT, now);
		txt = "";
		if (rec) {
			float v = rec->getInterpolatedValue(lon, lat);
			if (v != GRIB_NOTDEF) {
				txt.sprintf("%.2f %s", v, qPrintable(tr("mm/h")));
				bgColor = QColor(plot.getRainColor(v, true));
			}
		}
		addCell_content(txt, layout,lig,col, 1,1, bgColor);
	}
}
//-----------------------------------------------------------------
void MeteoTableWidget::addLine_CloudCover(int lig)
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	time_t now;
	int col;
	GribRecord *rec;
	QString   txt;
	GribPlot  plot;
	QColor    bgColor = Qt::white;
	col = 0;
	addCell_title(tr("Nébulosité"), true, layout, lig,col);
	col ++;	
	for (iter=sdates.begin(); iter!=sdates.end(); iter++, col++)
	{
		now = *iter;
		rec = gribread->getGribRecord(GRB_CLOUD_TOT, now);
		txt = "";
		if (rec) {
			float v = rec->getInterpolatedValue(lon, lat);
			if (v != GRIB_NOTDEF) {
				txt = Util::formatPercentValue(v);
				bgColor = QColor(plot.getCloudColor(v, true));
			}
		}
		addCell_content(txt, layout,lig,col, 1,1, bgColor);
	}
}
//-----------------------------------------------------------------
void MeteoTableWidget::addLine_Wind(int lig)
{
	GribReader *gribread = gribplot->getGribReader();
	std::set<time_t> sdates = gribread->getListDates();
	std::set<time_t>::iterator iter;
	time_t now;
	int col;
	GribRecord *recx,*recy;
	QString   tmp, txt;
	GribPlot  plot;
	QColor    bgColor = Qt::white;
	col = 0;
	float vx=0, vy=0;
	addCell_title(tr("Vent"), true, layout, lig,col);
	col ++;	
	for (iter=sdates.begin(); iter!=sdates.end(); iter++, col++)
	{
		now = *iter;
		recx = gribread->getGribRecord(GRB_WIND_VX, now);
		recy = gribread->getGribRecord(GRB_WIND_VY, now);
		txt = "";
		if (recx && recy) {
			vx = recx->getInterpolatedValue(lon, lat);
			vy = recy->getInterpolatedValue(lon, lat);
			if (vx != GRIB_NOTDEF && vx != GRIB_NOTDEF) {
				float v = sqrt(vx*vx+vy*vy);
				float dir = -atan2(-vx, vy) *180.0/M_PI + 180;
				if (dir < 0)
					dir += 360.0;
				if (dir >= 360)
					dir -= 360.0;

				QString tmp;
				txt = "";
				tmp.sprintf("%.0f", dir);
				txt += tmp + tr(" °") + "\n";
			 	txt += Util::formatSpeed(v) + "\n";
				tmp.sprintf("%2d", Util::kmhToBeaufort(v*3.6));
				txt += tmp + tr(" Bf");
				
				bgColor = QColor(plot.getWindColor(v*3.6, true));
			}
		}
		addCell_content(txt, layout,lig,col, 1,1, bgColor, true,vx,vy);
	}
}



//===================================================================
void MeteoTableWidget::addCell_content( QString txt,
QGridLayout *layout,int lig,int col,
				int    rowspan,
				int    colspan,
				QColor bgcolor,
				bool   isWindCell,
				float  vx,
				float  vy
				)
{
	// Ecriture contrastée
	float gris = 0.30*bgcolor.redF() + 0.59*bgcolor.greenF() + 0.11*bgcolor.blueF();
	QColor fgcolor;
	if (gris < 0.45)
		fgcolor = QColor(230,230,230);
	else
		fgcolor = Qt::black;
		
	TableCell *cell;
 	if (isWindCell) {
	 	cell = new TableCell_Wind(vx, vy, (lat<0), gribplot,
	 				this, txt, false, bgcolor, fgcolor);
 	}
 	else {
	 	cell = new TableCell(this, txt, false, bgcolor, fgcolor);
	}
	if (lig==0 && col==0)
		cell->setBorders(TableCell::all);
	else if (lig==0)
		cell->setBorders(TableCell::south+TableCell::east+TableCell::north);
	else if (col==0)
		cell->setBorders(TableCell::west+TableCell::south+TableCell::east);
	else
		cell->setBorders(TableCell::south+TableCell::east);
	layout->addWidget(cell, lig,col, rowspan,colspan );
}

//----------------------------------------------------------------
void MeteoTableWidget::addCell_title( QString txt, bool bold,
				QGridLayout *layout,int lig,int col, int rowspan,int colspan)
{
	QColor bgcolor(200,200,255);
	TableCell *cell = new TableCell(this, txt, bold, bgcolor);	
	if (lig==0 && col==0)
		cell->setBorders(TableCell::all);
	else if (lig==0)
		cell->setBorders(TableCell::south+TableCell::east+TableCell::north);
	else if (col==0)
		cell->setBorders(TableCell::west+TableCell::south+TableCell::east);
	else
		cell->setBorders(TableCell::south+TableCell::east);
	layout->addWidget(cell, lig,col, rowspan,colspan );
}

//===================================================================
// TableCell : case seule
//===================================================================
TableCell::TableCell(QWidget *parent, QString txt, bool bold,
						QColor bgcolor, QColor fgcolor)
	: QWidget(parent)
{
	this->bgcolor = bgcolor;
	this->bordercolor = QColor(100,100,100);
	this->borders = TableCell::none;
	
	QGridLayout *layout = new QGridLayout();
	
	label = new QLabel(txt, this);
	label->setAlignment(Qt::AlignHCenter);
	
	if (bold) {
		QFont font;
		font.setBold(true);
		label->setFont(font);
	}
	
	QPalette p;
	p.setBrush(QPalette::Active, QPalette::WindowText, fgcolor);
	p.setBrush(QPalette::Inactive, QPalette::WindowText, fgcolor);
	label->setPalette(p);
	layout->addWidget(label, 0,0, Qt::AlignHCenter|Qt::AlignBottom);
	this->setLayout(layout);
}	
//---------------------------------------------------------
void TableCell::paintEvent(QPaintEvent * /*event*/)
{
    QPainter pnt(this);
	
	pnt.fillRect(0,0,width(),height(), QBrush(bgcolor));
	
	QPen pen(bordercolor);
	pen.setWidth(1);
	pnt.setPen(pen);
	
	if (borders & TableCell::north)
		pnt.drawLine(0,0, width()-1,0);
	if (borders & TableCell::south)
		pnt.drawLine(0,height()-1, width()-1,height()-1);
	
	if (borders & TableCell::west)
		pnt.drawLine(0,0, 0,height()-1);
	if (borders & TableCell::east)
		pnt.drawLine(width()-1,0, width()-1,height()-1);
	
}


//===================================================================
// TableCell : case seule spécialisée pour le vent (flêche+barbules)
//===================================================================
TableCell_Wind::TableCell_Wind(float vx, float vy, bool south,
        			GribPlot *gribplot,
        			QWidget *parent, QString txt, bool bold,
        			QColor bgcolor, QColor fgcolor)
	: TableCell(parent, txt, bold, bgcolor, fgcolor)
{
	this->vx = vx;
	this->vy = vy;
	this->south = south;
	this->gribplot = gribplot;

	windArrowsColor = QColor(40,40,40);

	setMinimumHeight(label->minimumSizeHint().height()+50);
}	
//---------------------------------------------------------
void TableCell_Wind::paintEvent(QPaintEvent * e)
{
	TableCell::paintEvent(e);
    QPainter pnt(this);
	pnt.setRenderHint(QPainter::Antialiasing, true);
	
    gribplot->drawWindArrowWithBarbs(
    			pnt, width()/2, 25, vx, vy, south, windArrowsColor);
	
}


//===================================================================
// MeteoTable : dialog + MeteoTableWidget
//===================================================================
MeteoTableDialog::MeteoTableDialog(GribPlot *gribplot, float lon, float lat, QString posName)
	: QDialog()
{
    if (!gribplot->isGribReaderOk()) {
        QMessageBox::critical (this,
			tr("Erreur"),tr("Création du Météotable impossible:\n\nPas de fichier GRIB ouvert."));
		delete this;
		return;
    }
    GribReader *gribread = gribplot->getGribReader();
    GribRecord *grbrec;
	if ((grbrec=gribread->getFirstGribRecord()) == NULL) {
        QMessageBox::critical (this,
            	tr("Erreur"),tr("Création du Météotable impossible:\n\nZone GRIB indéterminée."));
		delete this;
		return;
    }
	if (!grbrec->isPointInMap(lon, lat)) {
		QMessageBox::critical (this,
				tr("Erreur"),tr("Création du Météotable impossible :\n\nPoint en dehors de la zone couverte par le fichier GRIB."));
		delete this;
		return;
	}

	//----------------------------------------------
	setModal(false);
	
	meteoTableWidget = new MeteoTableWidget(gribplot, lon,lat, this);
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidget(meteoTableWidget);
	
	QVBoxLayout *layout = new QVBoxLayout(this);

	QString position = Util::formatPosition(lon, lat);
	QLabel *lbpos;
	if (posName == "") {
		setWindowTitle(position);
		lbpos = new QLabel(tr("Position : ") + position);
	}
	else {
		setWindowTitle(posName);
		lbpos = new QLabel(tr("Position : ") +posName + " : " + position);
	}
	QLabel *lbdate = new QLabel(tr("Date de référence : ")
							+ Util::formatDateTimeLong(gribread->getRefDate()));

	btClose = new QPushButton(tr("Fermer"));
	btClose->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	
	connect(btClose, SIGNAL(clicked()), this, SLOT(reject()));
							
	layout->addWidget( lbpos);
	layout->addWidget( lbdate);
	layout->addWidget(scrollArea);
	layout->addWidget(btClose, 0, Qt::AlignHCenter);
	
	// taille par défaut pour la 1ère ouverture
	adjustSize();
	int w = 800;
	int h = this->height()+60;
    resize( Util::getSetting("meteoTableDialogSize", QSize(w,h)).toSize() );
	show();
}

//-----------------------------------------
MeteoTableDialog::~MeteoTableDialog()
{
	Util::setSetting("meteoTableDialogSize", size());
}






    

















