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

#include "Isobar.h"


//---------------------------------------------------------------
Isobar::Isobar(float press, const GribRecord *rec_)
{
    pressure = press;
    rec = rec_;
    W = rec->getNi();
    H = rec->getNj();
    int gr = 80;
    isobarColor = QColor(gr,gr,gr);
    //---------------------------------------------------------
    // Génère la liste des segments.
    extractIsobar(rec);
//printf("create Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());
}
//---------------------------------------------------------------
Isobar::~Isobar()
{
//printf("delete Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());

    std::list<Segment *>::iterator it;
    for (it=trace.begin(); it!=trace.end(); it++) {
        delete *it;
        *it = NULL;
    }
    trace.clear();
}

    

//---------------------------------------------------------------
void Isobar::drawIsobar(QPainter &pnt,
                            const Projection *proj)
{    
    std::list<Segment *>::iterator it;
    int   a,b,c,d;
    int nb = 0;
	pnt.setRenderHint(QPainter::Antialiasing, true);
    
    //---------------------------------------------------------
    // Dessine les segments
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++,nb++)
    {
        Segment *seg = *it;

        // Teste la visibilité (bug clipping sous windows avec pen.setWidthF())
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
void Isobar::drawIsobarLabels(QPainter &pnt,
                            const Projection *proj,
                            int density, int first)
{    
    std::list<Segment *>::iterator it;
    int   a,b,c,d;
    int nb = first;
    QString label;
    label = label.sprintf("%d", (int)(pressure/100+0.5));
    
    QPen penText(QColor(0,0,0));
    QFont fontText("Helvetica", 9);
    QFontMetrics fmet(fontText);
    QRect rect = fmet.boundingRect(label);
    pnt.setPen(penText);
    pnt.setFont(fontText);
    int r = 255;
    pnt.setBrush(QColor(r,r,r, 170));
    
    //---------------------------------------------------------
    // Ecrit les labels
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++,nb++)
    {
        if (nb % density == 0) {
            Segment *seg = *it;
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
                const GribRecord *rec, float pressure)
{
    traduitCode(I,J, c1, i,j);
    traduitCode(I,J, c2, k,l);
    traduitCode(I,J, c3, m,n);
    traduitCode(I,J, c4, o,p);
    
    intersectionAreteGrille(i,j, k,l,  &px1,&py1, rec, pressure);
    intersectionAreteGrille(m,n, o,p,  &px2,&py2, rec, pressure);
}
//-----------------------------------------------------------------------
void Segment::intersectionAreteGrille(int i,int j, int k,int l, float *x, float *y,
                const GribRecord *rec, float pressure)
{
    float a,b, pa, pb, dec;
    pa = rec->getValue(i,j);
    pb = rec->getValue(k,l);    
    // Abscisse
    a = rec->getX(i);
    b = rec->getX(k);
    if (pb != pa)
        dec = (pressure-pa)/(pb-pa);
    else
        dec = 0.5;
    if (fabs(dec)>1)
        dec = 0.5;
    *x = a+(b-a)*dec;
    // Ordonnée
    a = rec->getY(j);
    b = rec->getY(l);
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
// Génère la liste des segments.
// Les coordonnées sont les indices dans la grille du GribRecord
//---------------------------------------------------------
void Isobar::extractIsobar(const GribRecord *rec)
{
    int i, j, W, H;
    float x, y, a,b,c,d;        
    W = rec->getNi();
    H = rec->getNj();
    
    for (j=1; j<H; j++)     // !!!! 1 to end
    {
        for (i=1; i<W; i++)
        {
            x = rec->getX(i);
            y = rec->getY(j);
            
            a = rec->getValue( i-1, j-1 );            
            b = rec->getValue( i,   j-1 );
            c = rec->getValue( i-1, j   );
            d = rec->getValue( i,   j   );
            
            // Détermine si 1 ou 2 segments traversent la case ab-cd
            // a  b
            // c  d
            //--------------------------------
            // 1 segment en diagonale
            //--------------------------------
            if     ((a<=pressure && b<=pressure && c<=pressure  && d>pressure)
                 || (a>pressure && b>pressure && c>pressure  && d<=pressure))
                trace.push_back(new Segment(i,j, 'c','d',  'b','d', rec,pressure));
            else if ((a<=pressure && c<=pressure && d<=pressure  && b>pressure)
                 || (a>pressure && c>pressure && d>pressure  && b<=pressure))
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', rec,pressure));
            else if ((c<=pressure && d<=pressure && b<=pressure  && a>pressure)
                 || (c>pressure && d>pressure && b>pressure  && a<=pressure))
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', rec,pressure));
            else if ((a<=pressure && b<=pressure && d<=pressure  && c>pressure)
                 || (a>pressure && b>pressure && d>pressure  && c<=pressure))
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', rec,pressure));
            //--------------------------------
            // 1 segment H ou V
            //--------------------------------
            else if ((a<=pressure && b<=pressure   &&  c>pressure && d>pressure)
                 || (a>pressure && b>pressure   &&  c<=pressure && d<=pressure))
                trace.push_back(new Segment(i,j, 'a','c',  'b','d', rec,pressure));
            else if ((a<=pressure && c<=pressure   &&  b>pressure && d>pressure)
                 || (a>pressure && c>pressure   &&  b<=pressure && d<=pressure))
                trace.push_back(new Segment(i,j, 'a','b',  'c','d', rec,pressure));
            //--------------------------------
            // 2 segments en diagonale
            //--------------------------------
            else if  (a<=pressure && d<=pressure   &&  c>pressure && b>pressure) {
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', rec,pressure));
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', rec,pressure));
            }
            else if  (a>pressure && d>pressure   &&  c<=pressure && b<=pressure) {
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', rec,pressure));
                trace.push_back(new Segment(i,j, 'b','d',  'c','d', rec,pressure));
            }
                
        }
    }
}

