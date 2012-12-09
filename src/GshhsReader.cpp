/**********************************************************************
zUGrib: meteorologic GRIB file data viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

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

#include <QDebug>

#include "GshhsReader.h"

#include "Projection.h"

//==========================================================
// GshhsPolygon  (compatible avec le format .rim de RANGS)
//==========================================================
GshhsPolygon::GshhsPolygon(ZUFILE *file_)
{
    file  = file_;
    ok = true;
    id    = readInt4();
    n     = readInt4();
    flag  = readInt4();
    west  = readInt4() * 1e-6;
    east  = readInt4() * 1e-6;
    south = readInt4() * 1e-6;
    north = readInt4() * 1e-6;
    area  = readInt4();
    greenwich = readInt2();
    readInt2();   // source
    //printf("%d %d %d\n", id, n, ok);
    antarctic = (west==0 && east==360);
    if (ok)
    {
        double x, y=-90;

        for (int i=0; i<n; i++) {
            x = readInt4() * 1e-6;
            if (greenwich && x > 270)
                x -= 360;
            y = readInt4() * 1e-6;
            lsPoints.push_back(new GshhsPoint(x,y));
            /*if (antarctic)
{
	printf("x %12.8f %12.8f\n", x,y);
}*/
        }

    	// force l'Antarctic �  être un "rectangle" qui passe par le pôle
        if (antarctic) {
            lsPoints.push_front(new GshhsPoint(360, y));
            lsPoints.push_front(new GshhsPoint(360,-90));
            lsPoints.push_back(new GshhsPoint(0,-90));
        }

    }
}

//==========================================================
// GshhsPolygon_WDB     (entete de type GSHHS récent)
//==========================================================
GshhsPolygon_WDB::GshhsPolygon_WDB(ZUFILE *file_)
{
    file  = file_;
    ok = true;
    id    = readInt4();
    n     = readInt4();
    flag  = readInt4();
    west  = readInt4() * 1e-6;
    east  = readInt4() * 1e-6;
    south = readInt4() * 1e-6;
    north = readInt4() * 1e-6;
    area  = readInt4();

    if ( ( (flag >> 8) & 255 ) >= 7) { //GSHHS Release 2.0
        areaFull  = readInt4();
        container  = readInt4();
        ancestor  = readInt4();

        greenwich = ( flag >> 16) & 1;
        antarctic = (west==0 && east==360);
        if (ok) {
            double x=0, y=0;
            for (int i=0; i<n; i++) {
                x = readInt4() * 1e-6;
                if (greenwich && x > 270)
                    x -= 360;
                y = readInt4() * 1e-6;
                lsPoints.push_back(new GshhsPoint(x,y));
            }
            if (antarctic) {
                    lsPoints.push_front(new GshhsPoint(360, y));
                    lsPoints.push_front(new GshhsPoint(360,-90));
                lsPoints.push_back(new GshhsPoint(0,-90));
            }
        }
    }
    else {
        greenwich = ( flag >> 16) & 1;
        antarctic = (west==0 && east==360);
        if (ok) {
            for (int i=0; i<n; i++) {
                double x=0, y=0;
                x = readInt4() * 1e-6;
                if (greenwich && x > 270)
                    x -= 360;
                y = readInt4() * 1e-6;
                lsPoints.push_back(new GshhsPoint(x,y));
            }
        }
    }
}


//--------------------------------------------------------
// Destructeur
GshhsPolygon::~GshhsPolygon() {
    std::list<GshhsPoint *>::iterator itp;
    for (itp=lsPoints.begin(); itp != lsPoints.end(); ++itp) {
        delete *itp;
        *itp = NULL;
    }
    lsPoints.clear();
}


//==========================================================
//==========================================================
//==========================================================
// GshhsReader
//==========================================================
//==========================================================
//==========================================================
GshhsReader::GshhsReader(std::string fpath_):
   quality (-1),
   fpath (fpath_)
{
    gshhsPoly_reader = new GshhsPolyReader(fpath);
    for (int qual=0; qual<5; qual++)
    {
        lsPoly_boundaries[qual] = new std::list<GshhsPolygon*>;
        lsPoly_rivers[qual] = new std::list<GshhsPolygon*>;
    }
    setQuality(4);
}
int GshhsReader::getPolyVersion()
{
    return gshhsPoly_reader->getPolyVersion();
}

//-------------------------------------------------------

//-------------------------------------------------------
// Destructeur
GshhsReader::~GshhsReader() {
    clearLists();
    if(gshhsPoly_reader)
        delete gshhsPoly_reader;
}

//-----------------------------------------------------------------------
void GshhsReader::clearLists() {
    std::list<GshhsPolygon*>::iterator itp;
    for (int qual=0; qual<5; qual++)
    {
        for (itp=lsPoly_boundaries[qual]->begin(); itp != lsPoly_boundaries[qual]->end(); ++itp) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_rivers[qual]->begin(); itp != lsPoly_rivers[qual]->end(); ++itp) {
            delete *itp;
            *itp = NULL;
        }

        lsPoly_boundaries[qual]->clear();
        lsPoly_rivers[qual]->clear();
    }
}
//-----------------------------------------------------------------------
// extension du nom de fichier gshhs selon la qualite
std::string GshhsReader::getNameExtension(int quality)
{
    std::string ext;
    switch (quality) {
        case 0: ext = "c"; break;
        case 1: ext = "l"; break;
        case 2: ext = "i"; break;
        case 3: ext = "h"; break;
        case 4: ext = "f"; break;
        default: ext = "l"; break;
    }
    return ext;
}

std::string GshhsReader::getFileName_boundaries(int quality) {
    std::string fname, ext;
    ext = getNameExtension(quality);
    fname = fpath+"/"+"wdb_borders_" + ext + ".b";
    return fname;
}
std::string GshhsReader::getFileName_rivers(int quality) {
    std::string fname, ext;
    ext = getNameExtension(quality);
    fname = fpath+"/"+"wdb_rivers_" + ext + ".b";
    return fname;
}

//-----------------------------------------------------------------------
bool GshhsReader::gshhsFilesExists(int quality)
{
    std::string fname, ext;
    ext = getNameExtension(quality);
    fname = fpath+"/"+"poly-" + ext + "-1.dat";
    if(!QFile::exists(QString().fromStdString(fname)))
        return false;
    if (zu_can_read_file( getFileName_boundaries(quality).c_str() ) == 0)
        return false;
    if (zu_can_read_file( getFileName_rivers(quality).c_str() ) == 0)
        return false;
    return true;
}


//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
void GshhsReader::setQuality(int quality_) // 5 levels: 0=low ... 4=full
{
    if(quality==quality_ && gshhsPoly_reader->currentQuality==quality_) return;
    std::string fname;
    ZUFILE *file;
    bool   ok;

    quality = quality_;
    if (quality < 0) quality = 0;
    else if (quality > 4) quality = 4;

    //gshhsRangsReader->setQuality(quality);
    gshhsPoly_reader->setQuality(quality);

    // Frontieres politiques
    if (lsPoly_boundaries[quality]->size() == 0) { // on ne lit qu'une fois le fichier
        fname = getFileName_boundaries(quality);
        file = zu_open(fname.c_str(), "rb");
        if (file != NULL) {
            ok = true;
            while (ok)
            {
                GshhsPolygon *poly = new GshhsPolygon_WDB(file);
                ok = poly->isOk();
                if (ok)
                {
                    if (poly->getLevel() < 2)
                        lsPoly_boundaries[quality]->push_back(poly);
                    else
                        delete poly;
                }
                else
                    delete poly;
            }
            zu_close(file);
        }
    }
    // Rivieres
    if (lsPoly_rivers[quality]->size() == 0) { // on ne lit qu'une fois le fichier
        fname = getFileName_rivers(quality);
        file = zu_open(fname.c_str(), "rb");
        if (file != NULL) {
            ok = true;
            while (ok) {
                GshhsPolygon *poly = new GshhsPolygon_WDB(file);
                ok = poly->isOk();
                if (ok)
                {
                    lsPoly_rivers[quality]->push_back(poly);
                }
                else
                    delete poly;

            }
            zu_close(file);
        }
    }
}

//-----------------------------------------------------------------------
std::list<GshhsPolygon*> & GshhsReader::getList_boundaries() {
    return * lsPoly_boundaries[quality];
}
//-----------------------------------------------------------------------
std::list<GshhsPolygon*> & GshhsReader::getList_rivers() {
    return * lsPoly_rivers[quality];
}

//=====================================================================
// Dessin de la carte
//=====================================================================
int GshhsReader::GSHHS_scaledPoints(
            GshhsPolygon *pol, QPoint *pts, double decx,
            Projection *proj
        )
{
    // Elimine les polygones en dehors de la zone visible
    if (! proj->intersect(pol->west+decx, pol->east+decx, pol->south, pol->north)) {
        //printf("%f %f   -  %f %f \n", pol->west+decx, pol->east+decx, pol->south, pol->north);
        return 0;
    }

    // Elimine les polygones trop petits
    int a1, b1;
    int a2, b2;
    proj->map2screen(pol->west+decx, pol->north, &a1, &b1);
    proj->map2screen(pol->east+decx, pol->south, &a2, &b2);
    if (a1==a2 && b1==b2) {
        return 0;
    }

    double x, y;
    std::list<GshhsPoint *>::iterator itp;
    int xx, yy, oxx=0, oyy=0;
    int j = 0;

    for  (itp=(pol->lsPoints).begin(); itp!=(pol->lsPoints).end(); ++itp)
    {
        x = (*itp)->lon+decx;
        y = (*itp)->lat;
        // Ajustement d'echelle
        proj->map2screen(x, y, &xx, &yy);
        if (j==0 || (oxx!=xx || oyy!=yy))  // elimine les ponts trop proches
        {
            oxx = xx;
            oyy = yy;
            pts[j].setX(xx);
            pts[j].setY(yy);
            j ++;
        }
    }
	//if (j>1000)printf("%d\n", j);
    return j;
}

//-----------------------------------------------------------------------
void GshhsReader::GsshDrawLines(QPainter &pnt, std::list<GshhsPolygon*> &lst,
                                Projection *proj, bool isClosed
        )
{
    std::list<GshhsPolygon*>::iterator iter;
    GshhsPolygon *pol;
    QPoint *pts = NULL;
    int i;
    int nbp;

    int nbmax = 10000;
    pts = new QPoint[nbmax];
    assert(pts);

    for  (i=0, iter=lst.begin(); iter!=lst.end(); ++iter,++i) {
        pol = *iter;

        if (nbmax < pol->n+2) {
            nbmax = pol->n+2;
            pts = new QPoint[nbmax];
            assert(pts);
        }


		//--------------------------------------------------------------
        nbp = GSHHS_scaledPoints(pol, pts, 0, proj);
		if (nbp > 1) {
			if (pol->isAntarctic()) {
                // Ne pas tracer les bords artificiels qui rejoignent le pole
                // ajoutes lors de la creation des polygones (2 au debut, 1 a la fin).
				pts ++;
				nbp -= 2;
				pnt.drawPolyline(pts, nbp);
				pts --;
			}
			else {
				pnt.drawPolyline(pts, nbp);
				if (isClosed)
					pnt.drawLine(pts[0], pts[nbp-1]);
			}
		}

		//--------------------------------------------------------------
        nbp = GSHHS_scaledPoints(pol, pts, -360, proj);
		if (nbp > 1) {
			if (pol->isAntarctic()) {
                // Ne pas tracer les bords artificiels qui rejoignent le pole
                // ajoutes lors de la creation des polygones (2 au debut, 1 a  la fin).
				pts ++;
				nbp -= 2;
				pnt.drawPolyline(pts, nbp);
				pts --;
			}
			else {
				pnt.drawPolyline(pts, nbp);
				if (isClosed)
					pnt.drawLine(pts[0], pts[nbp-1]);
			}
		}
    }
    delete [] pts;
}

//-----------------------------------------------------------------------
void GshhsReader::drawBackground( QPainter &pnt, Projection *proj,
            QColor seaColor, QColor backgroundColor
        )
{
    // fond de carte
    pnt.setBrush(backgroundColor);
    pnt.setPen(backgroundColor);
    pnt.drawRect(0,0,proj->getW(), proj->getH());

    // oceans bleus (peint toute la zone entre les 2 poles)
    pnt.setBrush(seaColor);
    pnt.setPen(seaColor);
    int x0,y0,x1,y1;
    proj->map2screen(0,90, &x0,&y0);
    proj->map2screen(0,-90, &x1,&y1);

	//printf("drawBackground y0=%d y1=%d\n", y0,y1);

    pnt.drawRect(0, y0, proj->getW(), y1-y0);

}
//-----------------------------------------------------------------------
void GshhsReader::drawContinents( QPainter &pnt, Projection *proj,
            QColor seaColor, QColor landColor
        )
{
    selectBestQuality(proj);

    //pnt.setPen(Qt::transparent);

    gshhsPoly_reader->drawGshhsPolyMapPlain(pnt, proj, seaColor, landColor);
}

//-----------------------------------------------------------------------
void GshhsReader::drawSeaBorders( QPainter &pnt, Projection *proj)
{
    selectBestQuality(proj);

    pnt.setBrush(Qt::transparent);

    gshhsPoly_reader->drawGshhsPolyMapSeaBorders(pnt, proj);
}

//-----------------------------------------------------------------------
void GshhsReader::drawBoundaries( QPainter &pnt, Projection *proj)
{
    // Frontieres
    GsshDrawLines(pnt, getList_boundaries(), proj, false);
}

//-----------------------------------------------------------------------
void GshhsReader::drawRivers( QPainter &pnt, Projection *proj)
{
    // Rivieres
    GsshDrawLines(pnt, getList_rivers(), proj, false);
}

//-----------------------------------------------------------------------
void GshhsReader::selectBestQuality(Projection *proj)
{
    int bestQuality = 0;
    if (proj->getCoefremp() > 120)
        bestQuality = 0;
    else if (proj->getCoefremp() > 50)
        bestQuality = 1;
    else if (proj->getCoefremp() > 12)
        bestQuality = 2;
    else if (proj->getCoefremp() > 2)
        bestQuality = 3;
    else
        bestQuality = 4;
    setQuality(bestQuality);
}

