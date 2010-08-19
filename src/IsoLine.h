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

/*************************************
Dessin des données GRIB (avec QT)
*************************************/

#ifndef ISOLINE_H
#define ISOLINE_H

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <set>

#include <QApplication>
#include <QPainter>

#include "Grib.h"
#include "Projection.h"
#include "Util.h"

// TODO: join segments and draw a spline

//===============================================================
// Elément d'isobare qui passe dans un carré (ab-cd)de la grille.
// a  b
// c  d
// Rejoint l'arête (i,j)-(k,l) �  l'arête (m,n)-(o,p) (indices ds la grille GRIB)
class Segment
{
    public:
        Segment (int I, int J,
                char c1, char c2, char c3, char c4,
                time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt, double pressure);

        int   i,j,  k,l;   // arête 1
        double px1,  py1;   // Coordonées de l'intersection (i,j)-(k,l)
        int m,n, o,p;      // arête 2
        double px2,  py2;   // Coordonées de l'intersection (m,n)-(o,p)

    private:
        void traduitCode(int I, int J, char c1, int &i, int &j);

        void intersectionAreteGrille(int i,int j, int k,int l,
                double *x, double *y,
                time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt, double pressure);
};

//===============================================================
class IsoLine
{
    public:
        IsoLine(double val, time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt);
        ~IsoLine();


        void drawIsoLine(QPainter &pnt, const Projection *proj);

        void drawIsoLineLabels(QPainter &pnt, QColor &couleur, const Projection *proj,
                                int density, int first, double coef);

        int getNbSegments()     {return trace.size();}

    private:
        double value;
        int    W, H;     // taille de la grille
        const  GribRecord *rec;

        QColor isoLineColor;
        std::list<Segment *> trace;

        void intersectionAreteGrille(int i,int j, int k,int l, double *x, double *y,
                        const GribRecord *rec);

        //-----------------------------------------------------------------------
        // Génère la liste des segments.
        // Les coordonnées sont les indices dans la grille du GribRecord
        //---------------------------------------------------------
        void extractIsoLine(time_t now, time_t tPrev, time_t tNxt,GribRecord *rec_prev,GribRecord *rec_nxt);
};




#endif
