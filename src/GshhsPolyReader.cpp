/**********************************************************************
zyGrib: meteorological GRIB file viewer
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

#include "GshhsPolyReader.h"
#include "Projection.h"
#include "dataDef.h"


GshhsPolyCell::GshhsPolyCell(FILE *fpoly_, int x0_, int y0_,Projection *proj_,PolygonFileHeader *header_)
{
    proj = proj_;
    header=header_;
    fpoly = fpoly_;
    x0cell = x0_;
    y0cell = y0_;

    ReadPolygonFile(fpoly,x0cell,y0cell,header->pasx,header->pasy,&poly1,&poly2,&poly3,&poly4,&poly5);

#if 0
    int cnt=0;
    for(int i=0;i<poly1.count();++i)
        cnt+=poly1.at(i).count();

    qWarning() << "Cell " << x0cell << "," << y0cell << ": " << poly1.count() << " poly in p1 - total points:"<<cnt;
#endif
}

GshhsPolyCell::~GshhsPolyCell()
{

}

#define READ_POLY(POLY) { \
double X,Y; \
contour tmp_contour; \
int num_vertices,num_contours; \
int value; \
POLY->clear(); \
fread(&(num_contours), sizeof(int), 1, polyfile); \
for (int c= 0; c < num_contours; ++c) \
{ \
    fread(&(value), sizeof(int), 1, polyfile); /* discarding hole value */ \
    fread(&(value), sizeof(int), 1, polyfile); \
    num_vertices=value; \
    tmp_contour.clear(); \
    for (int v= 0; v < num_vertices; ++v) \
    { \
        fread(&(X), sizeof(double), 1, polyfile); \
        fread(&(Y), sizeof(double), 1, polyfile); \
        tmp_contour.append(QPointF(X*GSHHS_SCL,Y*GSHHS_SCL)); \
    } \
    POLY->append(tmp_contour); \
} \
}

void GshhsPolyCell::ReadPolygonFile (FILE *polyfile,
                        int x, int y,
                        int pas_x, int pas_y,
                        contour_list *p1, contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5)
{
    int pos_data;
    int tab_data;

    tab_data = (x/pas_x)*(180/pas_y) + (y+90)/pas_y;
    fseek(polyfile, sizeof(PolygonFileHeader) + tab_data*sizeof(int), SEEK_SET);
    fread(&pos_data, sizeof(int), 1, polyfile);

    fseek(polyfile, pos_data, SEEK_SET);

    READ_POLY(p1)
    READ_POLY(p2)
    READ_POLY(p3)
    READ_POLY(p4)
    READ_POLY(p5)
}

void  GshhsPolyCell::DrawPolygonFilled(QPainter &pnt,contour_list * p,double dx,Projection *proj,QColor color)
{
    double x, y;
    double c, v;
    QPolygonF poly_pt;

    QPen myPen(Qt::NoPen);
    pnt.setPen(myPen);
    pnt.setBrush(color);

    double x_old=0;
    double y_old=0;
    double previousX=0;
    double previousLon=0;

    bool first=true;
    for (c= 0; c < p->count(); ++c)
    {
        if(!p->at(c).count())
            continue;

        for (v= 0; v < p->at(c).count(); ++v)
        {
            if(first)
                proj->map2screenDouble(p->at(c).at(v).x()+dx, p->at(c).at(v).y(), &x, &y);
            else
                proj->map2screenByReference(previousLon,previousX,p->at(c).at(v).x()+dx, p->at(c).at(v).y(), &x, &y);
            first=false;
            previousLon=p->at(c).at(v).x()+dx;
            previousX=x;
            /*if(c==0 && v==0)
            {
                qWarning() << "Cell " << x0cell << "," << y0cell
                        << "Point 0: (" << p->at(c).at(v).x() << "," << p->at(c).at(v).y() << ")" << " - dx=" << dx
                        << "proj: " << x << "," << y;

            }*/

            if(v==0 || x!=x_old || y!=y_old)
            {
                poly_pt.append(QPoint(x,y));
                x_old=x;
                y_old=y;
            }
        }
        pnt.drawPolygon(poly_pt);
        poly_pt.clear();
    }
}

#define DRAW_POLY_FILLED(POLY,COL) if(POLY) DrawPolygonFilled(pnt,POLY,dx,proj,COL);

void  GshhsPolyCell::drawMapPlain(QPainter &pnt, double dx, Projection *proj,
            QColor seaColor, QColor landColor )
{
    pnt.setRenderHint(QPainter::Antialiasing, true);

    DRAW_POLY_FILLED(&poly1,landColor)
    DRAW_POLY_FILLED(&poly2,seaColor)
    DRAW_POLY_FILLED(&poly3,landColor)
    DRAW_POLY_FILLED(&poly4,seaColor)
    DRAW_POLY_FILLED(&poly5,landColor)
}

void GshhsPolyCell::DrawPolygonContour(QPainter &pnt,contour_list * p, double dx, Projection *proj)
{
    double x1, y1, x2, y2;
    double long_max, lat_max, long_min, lat_min;

    long_min=(double)x0cell;
    lat_min=(double)y0cell;
    long_max=((double)x0cell+(double)header->pasx);
    lat_max=((double)y0cell+(double)header->pasy);

    //qWarning()  << long_min << "," << lat_min << long_max << "," << lat_max;

    for (int i= 0; i < p->count(); ++i)
    {
        if(!p->at(i).count())
            continue;
        int v;
        for (v= 0; v < (p->at(i).count()-1); ++v)
        {
            x1=p->at(i).at(v).x();
            y1=p->at(i).at(v).y();
            x2=p->at(i).at(v+1).x();
            y2=p->at(i).at(v+1).y();

            // Elimination des traits verticaux et horizontaux
            if ((((x1==x2) && ((x1==long_min) || (x1==long_max))) || ((y1==y2) && ((y1==lat_min) || (y1==lat_max))))==0)
            {
                double A,B,C,D;
                proj->map2screenDouble(x1+dx,y1, &A, &B);
                proj->map2screenByReference(x1+dx,A,x2+dx,y2, &C, &D);
                if(qRound(A)!=qRound(C) || qRound(B)!=qRound(D))
                    pnt.drawLine(QPointF(A,B),QPointF(C,D));
                if(A!=C || B!=D)
                {
                    if(proj->isInBounderies(qRound(A),qRound(B)) || proj->isInBounderies(qRound(C),qRound(D)))
                        coasts.append(QLineF(A,B,C,D));
                }
            }
        }

        x1=p->at(i).at(v).x();
        y1=p->at(i).at(v).y();
        x2=p->at(i).at(0).x();
        y2=p->at(i).at(0).y();

        if ((((x1==x2) && ((x1==long_min) || (x1==long_max))) || ((y1==y2) && ((y1==lat_min) || (y1==lat_max))))==0)
        {
            double A,B,C,D;
            proj->map2screenDouble(x1+dx,y1, &A, &B);
            proj->map2screenByReference(x1+dx, A, x2+dx,y2, &C, &D);
            if(qRound(A)!=qRound(C) || qRound(B)!=qRound(D))
                pnt.drawLine(QPointF(A,B),QPointF(C,D));
            if(A!=C || B!=D)
            {
                if(proj->isInBounderies(qRound(A),qRound(B)) || proj->isInBounderies(qRound(C),qRound(D)))
                    coasts.append(QLineF(A,B,C,D));
            }
        }
    }
}

#define DRAW_POLY_CONTOUR(POLY) if(POLY) DrawPolygonContour(pnt,POLY,dx,proj);

void GshhsPolyCell::drawSeaBorderLines(QPainter &pnt, double dx, Projection *proj)
{
    coasts.clear();
    DRAW_POLY_CONTOUR(&poly1)
    DRAW_POLY_CONTOUR(&poly2)
    DRAW_POLY_CONTOUR(&poly3)
    DRAW_POLY_CONTOUR(&poly4)
    DRAW_POLY_CONTOUR(&poly5)
}

//========================================================================
//========================================================================
//========================================================================
//                GshhsPolyReader
//========================================================================
//========================================================================
//========================================================================
GshhsPolyReader::GshhsPolyReader(std::string Polypath):
    path (Polypath + "/")
{
    fpoly = NULL;

    for (int i=0; i<360; ++i) {
        for (int j=0; j<180; ++j) {
            allCells[i][j] = NULL;
        }
    }
    currentQuality = -1;
    this->abortRequested=false;
}


//-------------------------------------------------------------------------
GshhsPolyReader::~GshhsPolyReader()
{
        for (int i=0; i<360; ++i) {
                for (int j=0; j<180; ++j) {
                        if (allCells[i][j] != NULL)
                        {
                                delete allCells[i][j];
                                allCells[i][j] = NULL;
                        }
                }
        }
        if(fpoly)
            fclose(fpoly);
}

//-------------------------------------------------------------------------
int GshhsPolyReader::getPolyVersion()
{
    char txtn='c';
    QString fname=QString().fromStdString(path)+QString().sprintf("poly-%c-1.dat",txtn);
    if (fpoly)
        fclose(fpoly);
    fpoly = fopen( fname.toStdString().c_str(), "rb");

    /* init header */
    if(!fpoly) return -1;

    readPolygonFileHeader(fpoly,&polyHeader);
    //fclose(fpoly);
    currentQuality=0;
    return polyHeader.version;
}

void GshhsPolyReader::setQuality(int quality)  // 5 levels: 0=low ... 4=full
{
    if (currentQuality != quality || fpoly==NULL)
    {
        currentQuality = quality;

        char txtn;
        int qual = 4-quality;   // Fichier .dat : 0=meilleure ... 4=grossière
        if (qual < 0)   qual = 0;
        if (qual > 4)   qual = 4;

        switch(qual) {
            case 0:
                txtn='f';
                break;
            case 1:
                txtn='h';
                break;
            case 2:
                txtn='i';
                break;
            case 3:
                txtn='l';
                break;
            case 4:
                txtn='c';
                break;
        }


        QString fname=QString().fromStdString(path)+QString().sprintf("poly-%c-1.dat",txtn);

        //qWarning() << "New quality: " << qual << ", fname=" << fname;

        if (fpoly)
            fclose(fpoly);

        fpoly = fopen( fname.toStdString().c_str(), "rb");

        /* init header */
        if(fpoly)
            readPolygonFileHeader(fpoly,&polyHeader);

        for (int i=0; i<360; ++i) {
            for (int j=0; j<180; ++j) {
                if (allCells[i][j] != NULL)
                {
                    delete allCells[i][j];
                    allCells[i][j] = NULL;
                }
            }
        }
    }
}

void GshhsPolyReader::readPolygonFileHeader(FILE *polyfile, PolygonFileHeader *header)
{
//    int FReadResult = 0;

    fseek(polyfile, 0 , SEEK_SET);
    //FReadResult = (int)fread(header, sizeof(PolygonFileHeader), 1, polyfile);
    fread(header, sizeof(PolygonFileHeader), 1, polyfile);
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapPlain( QPainter &pnt, Projection *proj,
                    QColor seaColor, QColor landColor )
{
    if (!fpoly)
        return;

    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (proj->getXmin());
    cxmax = (int) ceil  (proj->getXmax());
    cymin = (int) floor (proj->getYmin());
    cymax = (int) ceil  (proj->getYmax());
    int dx, cx, cxx, cy;
    GshhsPolyCell *cel;

//printf("cxmin=%d cxmax=%d    cymin=%d cymax=%d\n", cxmin,cxmax, cymin,cymax);
    for (cx=cxmin; cx<cxmax; ++cx) {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;

        for (cy=cymin; cy<cymax; ++cy) {
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
                if (allCells[cxx][cy+90] == NULL) {
                    cel = new GshhsPolyCell(fpoly,cxx, cy,proj,&polyHeader);
                    assert(cel);
                    allCells[cxx][cy+90] = cel;
                }
                else {
                    cel = allCells[cxx][cy+90];
                }
                dx = cx-cxx;
                cel -> drawMapPlain(pnt, dx, proj, seaColor, landColor);
            }
        }
    }
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapSeaBorders( QPainter &pnt, Projection *proj)
{
    if (!fpoly)
        return;
    this->abortRequested=true;
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (proj->getXmin());
    cxmax = (int) ceil  (proj->getXmax());
    cymin = (int) floor (proj->getYmin());
    cymax = (int) ceil  (proj->getYmax());
    int dx, cx, cxx, cy;
    GshhsPolyCell *cel;

    for (cx=cxmin; cx<cxmax; ++cx) {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;

        for (cy=cymin; cy<cymax; ++cy) {
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
                if (allCells[cxx][cy+90] == NULL) {
                    cel = new GshhsPolyCell(fpoly,cxx, cy,proj,&polyHeader);
                    assert(cel);
                    allCells[cxx][cy+90] = cel;
                }
                else {
                    cel = allCells[cxx][cy+90];
                }
                dx = cx-cxx;
                cel->drawSeaBorderLines(pnt, dx, proj);
            }
        }
    }
    this->abortRequested=false;
}
