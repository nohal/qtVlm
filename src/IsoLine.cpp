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

#include "IsoLine.h"

#include "GribRecord.h"

//---------------------------------------------------------------
IsoLine::IsoLine(double val, time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt)
{
    value = val;
    W = rec_prev->get_Ni();
    H = rec_prev->get_Nj();
    int gr = 80;
    isoLineColor = QColor(gr,gr,gr);
    //---------------------------------------------------------
    // Genere la liste des segments.
    extractIsoLine(now,tPrev,tNxt, rec_prev,rec_nxt);
//printf("create Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());
}
//---------------------------------------------------------------
IsoLine::~IsoLine()
{
//printf("delete Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());

    std::list<Segment *>::iterator it;
    for (it=trace.begin(); it!=trace.end(); ++it) {
        delete *it;
        *it = NULL;
    }
    trace.clear();
}



//---------------------------------------------------------------
void IsoLine::drawIsoLine(QPainter &pnt,
                            const Projection *proj)
{
    std::list<Segment *>::iterator it;
    int   a,b,c,d;
    int nb = 0;
	pnt.setRenderHint(QPainter::Antialiasing, true);

    //---------------------------------------------------------
    // Dessine les segments
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); ++it,++nb)
    {
        Segment *seg = *it;

        // Teste la visibilite (bug clipping sous windows avec pen.setWidthF())
        if ( proj->isPointVisible(seg->px1, seg->py1)
                 ||    proj->isPointVisible(seg->px2, seg->py2))
        {
            proj->map2screen( seg->px1, seg->py1, &a, &b );
            proj->map2screen( seg->px2, seg->py2, &c, &d );
            pnt.drawLine(a,b, c,d);
        }
        // tour du monde ?
        if ( proj->isPointVisible(seg->px1-360.0, seg->py1)
                 ||    proj->isPointVisible(seg->px2-360.0, seg->py2))
        {
            proj->map2screen( seg->px1-360.0, seg->py1, &a, &b );
            proj->map2screen( seg->px2-360.0, seg->py2, &c, &d );
            pnt.drawLine(a,b, c,d);
        }
    }
}

//---------------------------------------------------------------
void IsoLine::drawIsoLineLabels(QPainter &pnt, QColor &couleur,
                            const Projection *proj,
                            int density, int first, double coef)
{
    std::list<Segment *>::iterator it;
    int   a,b,c,d;
    int nb = first;
    QString label;

    label = label.sprintf("%d", (int)(value*coef+0.5));

    QPen penText(couleur);
    QFont fontText(QApplication::font());
    QFontMetrics fmet(fontText);
    QRect rect = fmet.boundingRect(label);
    pnt.setPen(penText);
    pnt.setFont(fontText);

	// use a gradient, because it's a bug sometimes with solid pattern (black background)
	QLinearGradient gradient;
    int r = 255;
	gradient.setColorAt(0, QColor(r,r,r, 170));
	gradient.setColorAt(1, QColor(r,r,r, 170));
    pnt.setBrush(gradient);

    //---------------------------------------------------------
    // Ecrit les labels
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); ++it,++nb)
    {
        if (nb % density == 0) {
            Segment *seg = *it;
    		rect = fmet.boundingRect(label);
            proj->map2screen( seg->px1, seg->py1, &a, &b );
            proj->map2screen( seg->px2, seg->py2, &c, &d );
            rect.moveTo((a+c)/2-rect.width()/2, (b+d)/2-rect.height()/2);
            pnt.drawRect(rect.x()-1, rect.y(), rect.width()+2, fmet.ascent()+2);
            pnt.drawText(rect, Qt::AlignHCenter|Qt::AlignVCenter, label);

            // tour du monde ?
            proj->map2screen( seg->px1-360.0, seg->py1, &a, &b );
            proj->map2screen( seg->px2-360.0, seg->py2, &c, &d );
            rect.moveTo((a+c)/2-rect.width()/2, (b+d)/2-rect.height()/2);
            pnt.drawRect(rect.x()-1, rect.y(), rect.width()+2, fmet.ascent()+2);
            pnt.drawText(rect, Qt::AlignHCenter|Qt::AlignVCenter, label);
        }
    }
}
//==================================================================================
// Segment
//==================================================================================
Segment::Segment(int I, int J,
                char c1, char c2, char c3, char c4,
                time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt, double pressure)
{
    traduitCode(I,J, c1, i,j);
    traduitCode(I,J, c2, k,l);
    traduitCode(I,J, c3, m,n);
    traduitCode(I,J, c4, o,p);

    intersectionAreteGrille(i,j, k,l,  &px1,&py1, now,tPrev,tNxt, rec_prev,rec_nxt, pressure);
    intersectionAreteGrille(m,n, o,p,  &px2,&py2, now,tPrev,tNxt, rec_prev,rec_nxt, pressure);
}
//-----------------------------------------------------------------------
void Segment::intersectionAreteGrille(int i,int j, int k,int l, double *x, double *y,
                time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt, double pressure)
{
    double a,b, pa, pb, dec;
    pa = rec_prev->getValue(i,j);
    pb = rec_prev->getValue(k,l);
    if(tPrev!=tNxt)
    {
        const double pa1=rec_nxt->getValue(i,j);
        pa = pa + ((pa1-pa)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
        const double pb1=rec_nxt->getValue(k,l);
        pb = pb + ((pb1-pb)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
    }
    // Abscisse
    a = rec_prev->getX(i);
    b = rec_prev->getX(k);
    if (pb != pa)
        dec = (pressure-pa)/(pb-pa);
    else
        dec = 0.5;
    if (fabs(dec)>1)
        dec = 0.5;
    *x = a+(b-a)*dec;
    // Ordonnee
    a = rec_prev->getY(j);
    b = rec_prev->getY(l);
    if (pb != pa)
        dec = (pressure-pa)/(pb-pa);
    else
        dec = 0.5;
    if (fabs(dec)>1)
        dec = 0.5;
    *y = a+(b-a)*dec;
}
//---------------------------------------------------------------
void Segment::traduitCode(int I, int J, char c1, int &i, int &j) {
    switch (c1) {
        case 'a':  i=I-1;  j=J-1; break;
        case 'b':  i=I  ;  j=J-1; break;
        case 'c':  i=I-1;  j=J  ; break;
        case 'd':  i=I  ;  j=J  ; break;
        default:   i=I  ;  j=J  ;
    }
}

//-----------------------------------------------------------------------
// Genere la liste des segments.
// Les coordonnees sont les indices dans la grille du GribRecord
//---------------------------------------------------------
void IsoLine::extractIsoLine(time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt)
{
    int i, j, W, H;
    double a,b,c,d,a1,b1,c1,d1;
//    double x,y;
    W = rec_prev->get_Ni();
    H = rec_prev->get_Nj();

    for (j=1; j<H; j++)     // !!!! 1 to end
    {
        for (i=1; i<W; i++)
        {
//            x = rec_prev->getX(i);
//            y = rec_prev->getY(j);

            a = rec_prev->getValue( i-1, j-1 );
            b = rec_prev->getValue( i,   j-1 );
            c = rec_prev->getValue( i-1, j   );
            d = rec_prev->getValue( i,   j   );

            if(tPrev!=tNxt)
            {
                a1 = rec_nxt->getValue( i-1, j-1 );
                b1 = rec_nxt->getValue( i,   j-1 );
                c1 = rec_nxt->getValue( i-1, j   );
                d1 = rec_nxt->getValue( i,   j   );

                a = a + ((a1-a)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
                b = b + ((b1-b)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
                c = c + ((c1-c)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
                d = d + ((d1-d)/((double)(tNxt-tPrev)))*((double)(now-tPrev));
            }

            // Determine si 1 ou 2 segments traversent la case ab-cd
            // a  b
            // c  d
            //--------------------------------
            // 1 segment en diagonale
            //--------------------------------
            if     ((a<=value && b<=value && c<=value  && d>value)
                 || (a>value && b>value && c>value  && d<=value))
                trace.push_back(new Segment(i,j, 'c','d',  'b','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            else if ((a<=value && c<=value && d<=value  && b>value)
                 || (a>value && c>value && d>value  && b<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            else if ((c<=value && d<=value && b<=value  && a>value)
                 || (c>value && d>value && b>value  && a<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            else if ((a<=value && b<=value && d<=value  && c>value)
                 || (a>value && b>value && d>value  && c<=value))
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            //--------------------------------
            // 1 segment H ou V
            //--------------------------------
            else if ((a<=value && b<=value   &&  c>value && d>value)
                 || (a>value && b>value   &&  c<=value && d<=value))
                trace.push_back(new Segment(i,j, 'a','c',  'b','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            else if ((a<=value && c<=value   &&  b>value && d>value)
                 || (a>value && c>value   &&  b<=value && d<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'c','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            //--------------------------------
            // 2 segments en diagonale
            //--------------------------------
            else if  (a<=value && d<=value   &&  c>value && b>value) {
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            }
            else if  (a>value && d>value   &&  c<=value && b<=value) {
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', now,tPrev,tNxt, rec_prev,rec_nxt,value));
                trace.push_back(new Segment(i,j, 'b','d',  'c','d', now,tPrev,tNxt, rec_prev,rec_nxt,value));
            }

        }
    }
}

