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

#include <QDebug>

#include "GshhsReader.h"

#define FILL_POLY 1
#define NO_FILL_POLY 1

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

    greenwich = false;
    antarctic = false;
    if (ok) {
        for (int i=0; i<n; i++) {
            double x, y;
            x = readInt4() * 1e-6;
            if (greenwich && x > 270)
                x -= 360;
            y = readInt4() * 1e-6;
            lsPoints.push_back(new GshhsPoint(x,y));
        }
    }
}


//--------------------------------------------------------
// Destructeur
GshhsPolygon::~GshhsPolygon() {
    std::list<GshhsPoint *>::iterator itp;
    for (itp=lsPoints.begin(); itp != lsPoints.end(); itp++) {
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
GshhsReader::GshhsReader(std::string fpath_, int quality)
{
    fpath = fpath_;
    gshhsRangsReader = new GshhsRangsReader(fpath);
    isUsingRangsReader = true;
    for (int qual=0; qual<5; qual++)
    {
        lsPoly_level1[qual] = new std::list<GshhsPolygon*>;
        lsPoly_level2[qual] = new std::list<GshhsPolygon*>;
        lsPoly_level3[qual] = new std::list<GshhsPolygon*>;
        lsPoly_level4[qual] = new std::list<GshhsPolygon*>;
        lsPoly_boundaries[qual] = new std::list<GshhsPolygon*>;
        lsPoly_rivers[qual] = new std::list<GshhsPolygon*>;
    }
    userPreferredQuality = quality;
    setQuality(quality);
    mode=0;
}

// Destructeur
GshhsReader::~GshhsReader() {
    clearLists();
}

//-----------------------------------------------------------------------
void GshhsReader::clearLists() {
    std::list<GshhsPolygon*>::iterator itp;
    for (int qual=0; qual<5; qual++)
    {
        for (itp=lsPoly_level1[qual]->begin(); itp != lsPoly_level1[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_level2[qual]->begin(); itp != lsPoly_level2[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_level3[qual]->begin(); itp != lsPoly_level3[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_level4[qual]->begin(); itp != lsPoly_level4[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_boundaries[qual]->begin(); itp != lsPoly_boundaries[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        for (itp=lsPoly_rivers[qual]->begin(); itp != lsPoly_rivers[qual]->end(); itp++) {
            delete *itp;
            *itp = NULL;
        }
        lsPoly_level1[qual]->clear();
        lsPoly_level2[qual]->clear();
        lsPoly_level3[qual]->clear();
        lsPoly_level4[qual]->clear();
        lsPoly_boundaries[qual]->clear();
        lsPoly_rivers[qual]->clear();
    }
}
//-----------------------------------------------------------------------
// extension du nom de fichier gshhs selon la qualité
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
std::string GshhsReader::getFileName_gshhs(int quality)
{
/*    std::string fname, ext;
    ext = getNameExtension(quality);
    fname = fpath+"/"+"gshhs_" + ext + ".b";*/

    // Lit le .rim de RANGS �  la place du fichier initial
    char txtn[16];
    if (quality < 0)   quality = 0;
    if (quality > 4)   quality = 4;
    snprintf(txtn, 10, "%d", 4-quality);   // précision inversée :(

    std::string fname;
    fname = fpath+"/"+"gshhs_" + txtn + ".rim";

//printf("%s\n", fname.c_str());
    return fname;
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
    if (zu_can_read_file( getFileName_gshhs(quality).c_str() ) == 0)
        return false;
    if (zu_can_read_file( getFileName_boundaries(quality).c_str() ) == 0)
        return false;
    if (zu_can_read_file( getFileName_rivers(quality).c_str() ) == 0)
        return false;
    return true;
}

//-----------------------------------------------------------------------
void GshhsReader::readGshhsFiles()
{
    std::string fname;
    ZUFILE *file;
    bool   ok;
    int qual=quality;

    // Bordures des continents (4 niveaux) (gshhs_[clihf].b)
    if (lsPoly_level1[quality]->size() == 0) { // on ne lit qu'une fois le fichier
        fname = getFileName_gshhs(qual);
        //qWarning("Reading %s",fname.c_str());
        file = zu_open(fname.c_str(), "rb");
        if (file != NULL) {

            ok = true;
            while (ok) {
                GshhsPolygon *poly = new GshhsPolygon(file);
                ok = poly->isOk();
                if (ok) {
                    switch (poly->getLevel()) {
                        case 1: lsPoly_level1[qual]->push_back(poly); break;
                        case 2: lsPoly_level2[qual]->push_back(poly); break;
                        case 3: lsPoly_level3[qual]->push_back(poly); break;
                        case 4: lsPoly_level4[qual]->push_back(poly); break;
                    }
                }
            }
            zu_close(file);
        }
    }
}

//-----------------------------------------------------------------------
void GshhsReader::setUserPreferredQuality(int quality_) // 5 levels: 0=low ... 4=full
{
    userPreferredQuality = quality_;
}

//-----------------------------------------------------------------------
void GshhsReader::setQuality(int quality_) // 5 levels: 0=low ... 4=full
{
    std::string fname;
    ZUFILE *file;
    bool   ok;

    quality = quality_;
    if (quality < 0) quality = 0;
    else if (quality > 4) quality = 4;

    gshhsRangsReader->setQuality(quality);
    if (!isUsingRangsReader) {
        readGshhsFiles();
    }

    // Frontières politiques
    if (lsPoly_boundaries[quality]->size() == 0) { // on ne lit qu'une fois le fichier
        fname = getFileName_boundaries(quality);
        file = zu_open(fname.c_str(), "rb");
        if (file != NULL) {
            ok = true;
            while (ok) {
                GshhsPolygon *poly = new GshhsPolygon_WDB(file);
                ok = poly->isOk();
                if (ok) {
                    if (poly->getLevel() < 2)
                        lsPoly_boundaries[quality]->push_back(poly);
                }

            }
            zu_close(file);
        }
    }
    // Rivières
    if (lsPoly_rivers[quality]->size() == 0) { // on ne lit qu'une fois le fichier
        fname = getFileName_rivers(quality);
        file = zu_open(fname.c_str(), "rb");
        if (file != NULL) {
            ok = true;
            while (ok) {
                GshhsPolygon *poly = new GshhsPolygon_WDB(file);
                ok = poly->isOk();
                if (ok) {
                    lsPoly_rivers[quality]->push_back(poly);
                }

            }
            zu_close(file);
        }
    }
}

//-----------------------------------------------------------------------
std::list<GshhsPolygon*> & GshhsReader::getList_level(int level) {
    switch (level) {
        case 1: return * lsPoly_level1[quality];
        case 2: return * lsPoly_level2[quality];
        case 3: return * lsPoly_level3[quality];
        case 4: return * lsPoly_level4[quality];
        default: return * lsPoly_level1[quality];
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

#define IS_IN(x,y,w,h) (x>=0 && y>=0 && x<=w && y<=h)

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

    for  (itp=(pol->lsPoints).begin(); itp!=(pol->lsPoints).end(); itp++)
    {
        x = (*itp)->lon+decx;
        y = (*itp)->lat;
        // Ajustement d'échelle

        proj->map2screen(x, y, &xx, &yy);

        /*if(!IS_IN(xx,yy,w,h))
            continue;*/

        if (j==0 || (oxx!=xx || oyy!=yy))  // élimine les points trop proches
        {
            oxx = xx;
            oyy = yy;
            pts[j].setX(xx);
            pts[j].setY(yy);
            j ++;
        }
    }
    return j;
}

//=====================================================================
// Dessin de la carte
//=====================================================================


bool GshhsReader::segIntersect(double Ax, double Ay, double Bx, double By,
                                double Cx, double Cy, double Dx, double Dy, double * res_x, double * res_y)
{
    if(!res_x || !res_y)
        return false;
    double r,s;
    r=((Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy))/((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx));
    s=((Ay-Cy)*(Bx-Ax)-(Ax-Cx)*(By-Ay))/((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx));
    if(0<=r && r<=1 && 0<=s && s <=1)
    {
        //qWarning() << "r=" << r << ", s=" << s;
        *res_x=Ax+r*(Bx-Ax);
        *res_y=Ay+r*(By-Ay);
        return true;
    }
    else
        return false;

}

bool GshhsReader::addIntersect(double xx,double yy,double oxx,double oyy,clipPoint * A, clipPoint * B,clipPoint ** lst,bool isIn)
{
    double Ix,Iy;
    bool added=false;
    if(segIntersect(xx,yy,oxx,oyy,A->x,A->y,B->x,B->y,&Ix,&Iy))
    {
        int dim,sig;

        /*qWarning() << "Isec: " << oxx << "," << oyy << "-" << xx << "," << yy << " is:" << Ix
            << "," << Iy  << " going In " << !isIn;*/

        if(Ix==A->x && Iy==A->y)
        {
            A->isIntersection=true;
            A->goingIn=!isIn;
            if(*lst)
                (*lst)->append(A);
            *lst=A;
            added=true;
        }
        else if(Ix==B->x && Iy==B->y)
        {
            B->isIntersection=true;
            B->goingIn=!isIn;
            if(*lst)
                (*lst)->append(B);
            *lst=B;
            added=true;
        }
        else
        {
            if(A->y==B->y)
            {
                dim=1;
                if(A->x<B->x) sig=1;
                else sig=-1;
            }
            else
            {
                dim=2;
                if(A->y<B->y) sig=1;
                else sig=-1;
            }

            clipPoint * pt = new clipPoint(Ix,Iy,true,!isIn);
            clipPoint * prec=A;
            for(clipPoint * l=A->nextBorder;l!=B;l=l->nextBorder)
            {
                double val=(sig)*(dim==1?l->x:l->y);

                if( val > (sig)*(dim==1?Ix:Iy))
                {
                    prec->appendBorder(pt);
                    if(*lst)
                        (*lst)->append(pt);
                    *lst=pt;
                    added=true;
                    break;
                }
                prec=l;
            }
            if(!added)
            {
                double val=(sig)*(dim==1?B->x:B->y);
                if( val > (sig)*(dim==1?Ix:Iy))
                {
                    prec->appendBorder(pt);
                    if(*lst)
                        (*lst)->append(pt);
                    *lst=pt;
                    added=true;
                }
            }
        }
        if(added)
            return true;
        else
        {
            //qWarning() << "Not adding isec !!!";
            return false;
        }
    }
    return false;
}

void GshhsReader::GSHHS_scaleAndClip(QPainter &pnt, GshhsPolygon *pol, QPoint *pts, double decx,Projection *proj,
                                     int order,int fillType)
{
    clipPoint* clipList=NULL;
    clipPoint* borderList=NULL;

    bool allIn = true;
    // Elimine les polygones en dehors de la zone visible
    if (! proj->intersect(pol->west+decx, pol->east+decx, pol->south, pol->north)) {
        //printf("%f %f   -  %f %f \n", pol->west+decx, pol->east+decx, pol->south, pol->north);
        return ;
    }

    // Elimine les polygones trop petits
    int a1, b1;
    int a2, b2;
    proj->map2screen(pol->west+decx, pol->north, &a1, &b1);
    proj->map2screen(pol->east+decx, pol->south, &a2, &b2);
    if (a1==a2 && b1==b2) {
        return ;
    }

    int x, y;
    int ox=-1000,oy=-1000;
    std::list<GshhsPoint *>::iterator itp;
    double xx, yy, oxx, oyy;
    bool isIn,oIsIn;
    int nbPts=0;

    //qWarning() << "nb points init: " << (pol->lsPoints).size();


    float xMin,xMax,yMin,yMax;
    xMin=(proj->getXmin()<-180?-180:proj->getXmin());
    xMax=(proj->getXmax()>180?180:proj->getXmax());
    //if(xMin>xMax) { a=xMax;xMax=xMin;xMin=a; }
    yMin=(proj->getYmin()<-90?-90:proj->getYmin());
    yMax=(proj->getYmax()>90?90:proj->getYmax());
    //if(yMin>yMax) { a=yMax;yMax=yMin;yMin=a; }

    int x0,y0,x1,y1;
    qWarning() << "screen: " << proj->getW() << " " << proj->getH();
    proj->map2screen(proj->getXmin(),proj->getYmin(), &x0,&y0);
    proj->map2screen(proj->getXmax(),proj->getYmax(), &x1,&y1);
    qWarning() << "min: " << proj->getXmin() << " " << proj->getYmin() << " - " << x0 << " " << y0;
    qWarning() << "max: " << proj->getXmax() << " " << proj->getYmax() << " - " << x1 << " " << y1;
    proj->map2screen(xMin,yMin, &x0,&y0);
    proj->map2screen(xMax,yMax, &x1,&y1);
    qWarning() << "min: " << xMin << " " << yMin << " - " << x0 << " " << y0;
    qWarning() << "max: " << xMax << " " << yMax << " - " << x1 << " " << y1;
    proj->map2screen(0,90, &x0,&y0);
    proj->map2screen(0,-90, &x1,&y1);
    qWarning() << "sea: " << 0 << " " << y0 << " - " << proj->getW() << y1;

    clipPoint *A,*B,*C,*D;
    if(order==0)
    {
        A=new clipPoint(xMin,yMax);
        B=new clipPoint(xMin,yMin);
        C=new clipPoint(xMax,yMin);
        D=new clipPoint(xMax,yMax);
    }
    else
    {
        A=new clipPoint(xMin,yMax);
        B=new clipPoint(xMax,yMax);
        C=new clipPoint(xMax,yMin);
        D=new clipPoint(xMin,yMin);
    }

    /* creating bording list*/
    borderList=A;
    borderList->appendBorder(B);
    B->appendBorder(C);
    C->appendBorder(D);

    /* first point*/
    itp=(pol->lsPoints).begin();
    if(itp==(pol->lsPoints).end()) /*no point in the list*/
        return;
    xx = (*itp)->lon+decx;
    yy = (*itp)->lat;
    //proj->map2screen(x, y, &xx, &yy);

    /*init prev point to something different to first point */
    oxx=xx+5;
    oyy=yy+5;

    /*are we starting inside / outside drawing zone*/
    isIn=proj->isPointVisible(xx,yy);

    if(!isIn) /*first point outside => not all point inside */
        allIn=false;

    /*if(isIn)
        qWarning() << "Starting inside";
    else
        qWarning() << "Starting outside";*/

    for  (/*nothing*/; itp!=(pol->lsPoints).end(); itp++)
    {
        xx = (*itp)->lon+decx;
        yy = (*itp)->lat;

        /*entry or exit from drawing zone ?*/
        oIsIn=isIn;
        isIn=proj->isPointVisible(xx,yy);
        if(!isIn)
            allIn=false;

        if(isIn!=oIsIn)
        {
            /* let's find intersection */
            if(!addIntersect(xx,yy,oxx,oyy,A,B,&clipList,oIsIn)
                && !addIntersect(xx,yy,oxx,oyy,B,C,&clipList,oIsIn)
                && !addIntersect(xx,yy,oxx,oyy,C,D,&clipList,oIsIn)
                && !addIntersect(xx,yy,oxx,oyy,D,A,&clipList,oIsIn))
            {
                qWarning() << "Intersect not found!";
            }
            else
                nbPts++;
        }

        if(isIn)
        {
            clipPoint * ptr = new clipPoint(xx,yy);
            if(clipList)
                clipList->append(ptr);
            clipList=ptr;
            nbPts++;
        }

        /* saving new point */
        oxx = xx;
        oyy = yy;
    }

    /*if(clipList)
        qWarning() << "Scan of polygon finished, nb points: " << clipList->count() << " (" << nbPts << ") border:"
            << borderList->countBorder();*/

    //qWarning() << "border: " << borderList->printBorder();

    if(nbPts <= 3 || clipList==NULL)
    {
        //qWarning() << "Not enough point to draw a filled polygon";
    }
    else if(allIn)
    {
        //qWarning() << "All points are inside";
        int j=0;
        ox=oy=-1000;
        for(clipPoint * ptr=clipList->next;ptr!=clipList;ptr=ptr->next)
        {
            proj->map2screen(ptr->x, ptr->y, &x, &y);
            if(ox!=x || oy!=y)
            {
                pts[j].setX(x);
                pts[j].setY(y);
                j++;
                ox=x;
                oy=y;
            }
        }
        if(fillType==FILL_POLY)
            pnt.drawPolygon(pts, j);
        else
            pnt.drawPolyline(pts, j);
    }
    else
    {
        /* searching for the first border going from outside -> inside*/
        //qWarning() << "drawing list of polygons ";
        clipPoint * start=NULL;
        if(clipList->isIntersection && clipList->goingIn)
            start=clipList;
        else
            for(clipPoint * ptr=clipList->next;ptr!=clipList;ptr=ptr->next)
            {
                if(ptr->isIntersection && ptr->goingIn)
                {
                    start=ptr;
                    break;
                }
            }

        if(!start)
        {
            //qWarning() << "Can't find a start point";
            goto error_end;
        }
        else
        {
            /*qWarning() << "Start is " << start->x << "," << start->y << " intersect " << start->isIntersection
               << ", going in " << start->goingIn << ", visited " << start->visited;*/
            /* we have a start, adding it to draw polygon */
            proj->map2screen(start->x, start->y, &x, &y);
            pts[0].setX(x);
            pts[0].setY(y);
            ox=x;
            oy=y;
            start->visited=true;

            clipPoint * ptr = start->next;
            int j=1; /*index in polygon*/

            while(!ptr->visited)
            {
                /* ajout du point */
                proj->map2screen(ptr->x, ptr->y, &x, &y);
                if(ox!=x || oy!=y)
                {
                    pts[j].setX(x);
                    pts[j].setY(y);
                    j++;
                    ox=x;
                    oy=y;
                }
                ptr->visited=true;

                if(ptr->isIntersection) /* border reached */
                {
                    /*qWarning() << "New intersection: " << ptr->x << "," << ptr->y << ", going in "
                        << ptr->goingIn << ", visited " << ptr->visited;*/
                    if(ptr->goingIn)
                    {
                       //qWarning() << "error we are in and finding an entering intersection";
                       goto error_end;
                    }
                    else
                    {
                        /*adding border points*/
                        clipPoint * ptr2;
                        for(ptr2=ptr->nextBorder;!ptr2->isIntersection && ptr2!=start;ptr2=ptr2->nextBorder)
                        {
                            //qWarning() << "Adding border " << ptr2->x << "," << ptr2->y;
                            proj->map2screen(ptr2->x, ptr2->y, &x, &y);
                            if(ox!=x || oy!=y)
                            {
                                pts[j].setX(x);
                                pts[j].setY(y);
                                j++;
                                ox=x;
                                oy=y;
                            }
                            ptr2->visited=true;
                        }
                        if(ptr2==start)
                        {
                            //qWarning() << "Drawing " << j << " points";
                            if(fillType==FILL_POLY)
                                pnt.drawPolygon(pts, j);
                            else
                                pnt.drawPolyline(pts, j);
                            start=ptr;
                            /* searching next not visited going in point */
                            for(ptr=ptr->next;ptr!=start && (ptr->visited||!ptr->isIntersection);ptr=ptr->next) /*nothing*/;
                            if(ptr==start)
                            {
                                //qWarning() << "can't find a new start";
                                break;
                            }
                            start=ptr;
                            /* adding first point */
                            /*qWarning() << "New Start is " << start->x << "," << start->y
                                        << " intersect " << start->isIntersection  << ", going in "
                                        << start->goingIn << ", visited " << start->visited;*/
                            proj->map2screen(ptr->x, ptr->y, &x, &y);
                            pts[0].setX(x);
                            pts[0].setY(y);
                            start->visited=true;
                            ptr=start->next;
                            j=1;
                            ox=x;
                            oy=y;
                        }
                        else if(ptr2->isIntersection)
                        {
                            if(!ptr2->goingIn)
                            {
                                //qWarning() << "next intersect not going in: " << ptr2->x << "," << ptr2->y;;
                                goto error_end;
                            }
                            /* ajout du point */
                            /*qWarning() << "Isec as a ptr2" << ptr2->x << "," << ptr2->y
                                << " intersect " << ptr2->isIntersection  << ", going in "
                                << ptr2->goingIn << ", visited " << ptr2->visited;*/
                            proj->map2screen(ptr2->x, ptr2->y, &x, &y);
                            if(ox!=x || oy!=y)
                            {
                                pts[j].setX(x);
                                pts[j].setY(y);
                                j++;
                                ox=x;
                                oy=y;
                            }
                            ptr2->visited=true;
                            ptr=ptr2->next; /* �  la prochaine iteration on le prendra en compte*/
                        }
                        else
                        {
                            qWarning() << "We should not be here !!";
                            return;
                        }
                    }
                }
                else
                {
                    //qWarning() << "Adding point " << ptr->x << "," << ptr->y;
                    ptr=ptr->next;
                }
            }
        }
    }

    //qWarning() << "All finished for this polygon";

error_end:
    /*clean border and clipList */
    if(clipList)
    {
        clipList->clearList();
        delete clipList;
    }

    delete A;
    delete B;
    delete C;
    delete D;

    return ;
}

//-----------------------------------------------------------------------
void GshhsReader::GsshDrawPolygons(QPainter &pnt, std::list<GshhsPolygon*> &lst,
                                    Projection *proj,int order)
{
    std::list<GshhsPolygon*>::iterator iter;
    GshhsPolygon *pol;
    QPoint *pts = NULL;
    int i;
    int nbp;

    int nbmax = 10000;
    pts = new QPoint[nbmax];
    assert(pts);

    for  (i=0, iter=lst.begin(); iter!=lst.end(); iter++,i++) {
        pol = *iter;
        if (nbmax < pol->n+2) {
            nbmax = pol->n+2;
            pts = new QPoint[nbmax];
            assert(pts);
        }

        if(mode==1)
        {
            GSHHS_scaleAndClip(pnt, pol, pts, 0, proj,order,FILL_POLY);
            GSHHS_scaleAndClip(pnt, pol, pts, -360, proj,order,FILL_POLY);
        }
        else
        {
            nbp = GSHHS_scaledPoints(pol, pts, 0, proj);
            if (nbp > 3)
                pnt.drawPolygon(pts,nbp);

            nbp = GSHHS_scaledPoints(pol, pts, -360, proj);
            if (nbp > 3)
                pnt.drawPolygon(pts, nbp);
        }
    }

    delete [] pts;
}

//-----------------------------------------------------------------------
void GshhsReader::GsshDrawLines(QPainter &pnt, std::list<GshhsPolygon*> &lst,
                                Projection *proj, bool /*isClosed*/, int order, int mode)
{
    std::list<GshhsPolygon*>::iterator iter;
    GshhsPolygon *pol;
    QPoint *pts = NULL;
    int i;
    int nbp;

    int nbmax = 10000;
    pts = new QPoint[nbmax];
    assert(pts);

    for  (i=0, iter=lst.begin(); iter!=lst.end(); iter++,i++) {
        pol = *iter;

        if (nbmax < pol->n+2) {
            nbmax = pol->n+2;
            pts = new QPoint[nbmax];
            assert(pts);
        }

/*
        //--------------------------------------------------------------
        nbp = GSHHS_scaledPoints(pol, pts, 0, proj);
        if (nbp > 1) {
            if (pol->isAntarctic()) {
                // Ne pas tracer les bords artificiels qui rejoignent le pôle
                // ajoutés lors de la création des polygones (2 au début, 1 �  la fin).
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
                // Ne pas tracer les bords artificiels qui rejoignent le pôle
                // ajoutés lors de la création des polygones (2 au début, 1 �  la fin).
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
        }*/
        if(mode==1)
        {
            GSHHS_scaleAndClip(pnt, pol, pts, 0, proj,order,NO_FILL_POLY);
            GSHHS_scaleAndClip(pnt, pol, pts, -360, proj,order,NO_FILL_POLY);
        }
        else
        {
            nbp = GSHHS_scaledPoints(pol, pts, 0, proj);
            if (nbp > 1)
                pnt.drawPolyline(pts,nbp);

            nbp = GSHHS_scaledPoints(pol, pts, -360, proj);
            if (nbp > 1)
                pnt.drawPolyline(pts, nbp);
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

    // océans bleus (peint toute la zone entre les 2 pôles)
    pnt.setBrush(seaColor);
    pnt.setPen(seaColor);
    int x0,y0,x1,y1;
    proj->map2screen(0,89, &x0,&y0);
    proj->map2screen(0,-89, &x1,&y1);

    pnt.drawRect(0, y0, proj->getW(), y1-y0);
}

//-----------------------------------------------------------------------
void GshhsReader::drawContinents( QPainter &pnt, Projection *proj,
            QColor seaColor, QColor landColor
        )
{
    selectBestQuality(proj);

    mode=0;//(Util::getSetting("manualClipping",0).toInt());

    pnt.setPen(Qt::transparent);

    if (isUsingRangsReader) {
        gshhsRangsReader->drawGshhsRangsMapPlain(pnt, proj, seaColor, landColor);
        return;
    }

    readGshhsFiles();


    // Continents (level 1)
    pnt.setBrush(landColor);
    GsshDrawPolygons(pnt, getList_level(1), proj,0);
    // Grands lacs (level 2)
    pnt.setBrush(seaColor);
    GsshDrawPolygons(pnt, getList_level(2), proj,1);
    // Terres dans les grands lacs (level 3)
    pnt.setBrush(landColor);
    GsshDrawPolygons(pnt, getList_level(3), proj,0);
    // Lacs dans les terres dans les grands lacs (level 4)
    /*pnt.setBrush(seaColor);
    GsshDrawPolygons(pnt, getList_level(4), proj,1);*/
}

//-----------------------------------------------------------------------
void GshhsReader::drawSeaBorders( QPainter &pnt, Projection *proj)
{
    selectBestQuality(proj);

    pnt.setBrush(Qt::transparent);

    mode=0;//(Util::getSetting("manualClipping",0).toInt());

    if (isUsingRangsReader) {
       gshhsRangsReader->drawGshhsRangsMapSeaBorders(pnt, proj);
       return;
    }

    readGshhsFiles();

    // Continents (level 1)
    GsshDrawLines(pnt, getList_level(1), proj, true,0,mode);
    // Grands lacs (level 2)
    GsshDrawLines(pnt, getList_level(2), proj, true,1,mode);
    // Terres dans les grands lacs (level 3)
    GsshDrawLines(pnt, getList_level(3), proj, true,0,mode);
    // Lacs dans les terres dans les grands lacs (level 4)
    //GsshDrawLines(pnt, getList_level(4), proj, true);
}

//-----------------------------------------------------------------------
void GshhsReader::drawBoundaries( QPainter &pnt, Projection *proj)
{
    // Frontières
    GsshDrawLines(pnt, getList_boundaries(), proj, false,0,0);
}

//-----------------------------------------------------------------------
void GshhsReader::drawRivers( QPainter &pnt, Projection *proj)
{
    // Rivières
    GsshDrawLines(pnt, getList_rivers(), proj, false,0,0);
}

//-----------------------------------------------------------------------
void GshhsReader::selectBestQuality(Projection *proj)
{
    isUsingRangsReader = proj->getCoefremp()<10;

    int bestQuality = 0;

    if (proj->getCoefremp() > 50)
        bestQuality = 0;
    else if (proj->getCoefremp() > 5)
        bestQuality = 1;
    else if (proj->getCoefremp() > 0.2)
        bestQuality = 2;
    else if (proj->getCoefremp() > 0.005)
        bestQuality = 3;
    else
        bestQuality = 4;

    if (bestQuality > userPreferredQuality)
        setQuality(userPreferredQuality);
    else
        setQuality(bestQuality);

    //qWarning() << "Quality " << bestQuality;
}


//---------------------------------------------------------
// ClipPoint class

void clipPoint::clearList(void)
{
    clipPoint * ptr = this->next;
    clipPoint * prec;
    this->next=NULL;
    while(ptr!=this)
    {
        prec=ptr;
        ptr=ptr->next;
        delete prec;
    }
}

void clipPoint::append(clipPoint * ptr)
{
    ptr->next=this->next;
    this->next=ptr;
}

void clipPoint::appendBorder(clipPoint * ptr)
{
    ptr->nextBorder=this->nextBorder;
    this->nextBorder=ptr;
}

void clipPoint::init(double x, double y, bool type, bool dir)
{
    this->x=x;
    this->y=y;
    this->isIntersection=type;
    this->goingIn=dir;
    next=this;
    nextBorder=this;
    visited=false;
}

int clipPoint::count(void)
{
    int cnt;
    clipPoint * ptr=this->next;
    for(cnt=1;ptr!=this;ptr=ptr->next) cnt++;
    return cnt;
}

int clipPoint::countBorder(void)
{
    int cnt;
    clipPoint * ptr=this->nextBorder;
    for(cnt=1;ptr!=this;ptr=ptr->nextBorder) cnt++;
    return cnt;
}

QString clipPoint::printBorder(void)
{
    QString res="";
    clipPoint * ptr=this;
    do {
        QTextStream(&res) << "(" << ptr->x << "," << ptr->y << ") ";
        ptr=ptr->nextBorder;
    }
    while(ptr!=this);
    return res;
}
