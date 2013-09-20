/*
*  bsb_io.c	- implementation of libbsb reading and writing
*
*  Copyright (C) 2000  Stuart Cunningham <stuart_hc@users.sourceforge.net>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*  $Id: bsb_io.c,v 1.22 2007/02/18 06:12:50 mikrom Exp $
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <bsb.h>
#include <QString>
#include "georef.h"
#include <QDebug>

#ifdef _WIN32
    #define DIR_SEPARATOR '\\'
#else
    #define DIR_SEPARATOR '/'
#endif

/* MSVC doesn't supply a strcasecmp(), so use the MSVC workalike */
#ifdef _MSC_VER
    #define strcasecmp(s1, s2) stricmp(s1, s2)
#endif

/**
 *  bsb_ntohl - portable ntohl
 */
typedef enum OcpnProjTypebra
{
      PROJECTION_UNKNOWN,
      PROJECTION_MERCATOR,
      PROJECTION_TRANSVERSE_MERCATOR,
      PROJECTION_POLYCONIC
}_OcpnProjType;
static uint32_t bsb_ntohl(uint32_t netlong)
{
    static const uint32_t testvalue = 0x12345678;
    uint8_t *p = (uint8_t *)&testvalue;

    if (p[0] == 0x12 && p[3] == 0x78)       /* big endian? */
        return netlong;

    /* for little endian swap bytes */
    return( (netlong & 0xff000000) >> 24
            | (netlong & 0x00ff0000) >> 8
            | (netlong & 0x0000ff00) << 8
            | (netlong & 0x000000ff) << 24 );
}

/**
 * Copies the next newline-delimited line from *pp into line as a NUL
 * terminated string and increments	the pp pointer to point to the start
 * of the next line.
 * Removes \r characters from the line (if any).
 *
 * @param pp pointer to pointer of the the start
 * @param len size of line buffer
 * @param line the buffer to read the line to
 *
 * @return 0 is no NL or line did not fit in the buffer; 1 on success
 */
static int next_line(char **pp, int len, char *line)
{
    char *p = *pp;
    char *q = line;

    while (*p != '\0')
    {
        /* Don't overflow destination buffer */
        if (q - line > len)
            return 0;       /* line didn't fit in buffer */

        if (*p == '\r')
        {
            p++;
            continue;
        }
        if (*p == '\n')
        {
            p++;
            if ( *p == ' ' ) // line continues so glue it together
            {
                while ( *p == ' ' ) p++;
                if( *(q-1) != ',' )
                    *q++ = ','; /* join them with the comma */
            }
            else /* it is trully the end */
            {
                *q = '\0';
                *pp = p;
                return 1;
            }
        }
        *q++ = *p++;
    }
    return 0;               /* didn't find \n line terminator */
}

/**
 * reads the string until comma or max chars
 *
 * @param from source to read from
 * @param to destination to put the chars into
 * @param max numer of chars (sizeo of to)
 */
static void readStrUntilComma( const char* from, char* to, const unsigned max )
{
    unsigned i = 0;
    while ( from[i] && from[i] != ',' && i < max-1 )
    {
        to[i]=from[i];
        i++;
    }
    /* terminate string */
    to[i] = 0;
}

/**
 * reads the comma-separated list of numbers from string
 *
 * @param pc pointer to first character on list
 * @param list[] array of double values to read into
 * @param count number of parameters to read (length of list[]
 *
 * @return count of read numbers
 */
static
int readNumberList( char* pc, double list[], int count )
{
    int i = 0;
    while ( *pc && i < count )
    {
        while ( *pc && *pc != ',' ) pc++;
        if ( *pc )
        {
            pc++;
            if ( *pc ) sscanf( pc, "%lf", &list[i++] );
        }
    }
    return i;
}

/**
 * internal function - reads the raster row index
 *
 * @param p pointer to BSBImage to update
 */
static
int bsb_read_row_index( BSBImage* p )
{
    /* Read start-of-index offset */
    uint32_t st,start_of_index;
    int i,max_row_size;
    if (fseek(p->pFile, -4, SEEK_END) == -1)
        return 0;
    if (fread(&st, 4, 1, p->pFile) != 1)
        return 0;
    start_of_index = bsb_ntohl(st);
    /* Read start-of-rows offset */
    if (fseek(p->pFile, start_of_index, SEEK_SET) == -1)
        return 0;
    /* allocate one more for last row ending */
    p->row_index = (uint32_t*)malloc( (p->height+1)*4 );
    if ( !p->row_index )
        return 0;
    if (fread(p->row_index, p->height*4, 1, p->pFile) != 1)
        return 0;
    /* remember end of last row, which is start of the index */
    p->row_index[p->height] = start_of_index;
    /* convert endiannes */
    for ( i = 0; i < p->height; i++ )
        p->row_index[i] = bsb_ntohl(p->row_index[i]);
    /* convert endiannes */
    max_row_size = 0;
    for ( i = 0; i < p->height; i++ )
    {
        int row_size = p->row_index[i+1]-p->row_index[i];
        max_row_size = max_row_size < row_size ? row_size : max_row_size;
    }
    p->rbuf = (unsigned char*)malloc( max_row_size );
    return p->rbuf != 0;
}

/**
 * computes the BSB header size of the BSB (KAP) file (text part preceding the image)
 *
 * @return the size of the BSB header
 *
 */
extern int bsb_get_header_size(FILE *fp)
{
    int text_size = 0, c;

    /* scan for end-of-text marker and record size of text section */
    while ( (c = fgetc(fp)) != -1 )
    {
        if (c == 0x1a)      /* Control-Z */
            break;
        text_size++;
    }
    return text_size;
}

/**
 *  opens the BSB (KAP or NO1) file and and populated the BSBImage structure
 *  also reads the row index
 *
 * @param filename full path to the file to open
 * @param p pointer to the BSBImage structure
 *
 * @return 0 on failure
 */
extern int bsb_open_header(char *filename, BSBImage *p)
{
    int text_size = 0, c, depth;
    char *p_ext, *pt, *text_buf, line[1024];
    long pos;

    setlocale( LC_ALL, "C" );

    /* zerofill entire BSB structure - not very strict
       as we would want some 0.0l and 0.0f but this works just the same */
    memset( p, 0, sizeof(*p) );

    /* Look for an extension (if any) to test for '.NO1' files
       which must be treated specially since they are obfuscated */
    if (    (p_ext = strrchr(filename, '.')) != NULL &&
            (p_ext > strrchr(filename, DIR_SEPARATOR)) &&
            (strcasecmp(p_ext, ".NO1") == 0) )
    {
        FILE *inputFile;

        if (! (inputFile = fopen(filename, "rb")))
        {
            qWarning()<<"unable to open"<<filename<<"(1)";
            return 0;
        }

        /* Open temporary file to store unobfuscated file */
        if (! (p->pFile = tmpfile()))
        {
            qWarning()<<"unable to open tmpfile";
            return 0;
        }

        /* .NO1 files are obfuscated using ROT-9 */
        while ((c = fgetc(inputFile)) != EOF)
        {
            int r = (c - 9) & 0xFF;
            fputc(r, p->pFile);
        }
        fflush(p->pFile);
        fseek(p->pFile, 0, SEEK_SET);
    }
    else
    {
        /* Normal unobfuscated BSB/NOS files can be opened straight away */
        if (! (p->pFile = fopen(filename, "rb")))
        {
            qWarning()<<"unable to open"<<filename<<"(2)";
            return 0;
        }
    }

    if ((text_size = bsb_get_header_size(p->pFile)) == 0) {
        qWarning()<<"header or file empty";
        return 0;
    }

    /* allocate space & read in the entire text header */
    text_buf = (char *)malloc(text_size + 1);
    if (text_buf == NULL)
    {
        qWarning()<<"malloc(%d) failed for text header - BSB file possibly corrupt"<<text_size + 1;
        return 0;
    }

    fseek(p->pFile, 0, SEEK_SET);
    if (fread(text_buf, text_size, 1, p->pFile) != 1) {
        qWarning()<<"error reading file";
        return 0;
    }
    text_buf[text_size] = '\0';

    pt = text_buf;
    p->num_colors = 0;
    p->num_refs = 0;
    p->num_plys = 0;
    p->num_wpxs = 0;
    p->num_wpys = 0;
    p->num_pwxs = 0;
    p->num_pwys = 0;
    p->version = -1.0;
    p->width = -1;
    p->height = -1;
    while ( next_line(&pt, sizeof(line), line) )
    {
        char *s;
        int  idx, r, g, b, ifm_depth;

        if (sscanf(line, "RGB/%d,%d,%d,%d", &idx, &r, &g, &b) == 4)
        {
            if ((unsigned)idx < sizeof(p->red)/sizeof(p->red[0]))
            {
                if (idx > 0)
                {
                    p->red[idx-1] = r;
                    p->green[idx-1] = g;
                    p->blue[idx-1] = b;
                    p->num_colors++;
                }
            }
        }
        if (sscanf(line, "REF/%d,%d,%d,%lf,%lf",
                   &p->ref[p->num_refs].id,
                   &p->ref[p->num_refs].x,
                   &p->ref[p->num_refs].y,
                   &p->ref[p->num_refs].lat,
                   &p->ref[p->num_refs].lon) == 5)
        {
            if ( p->num_refs < BSB_MAX_REFS -1 )
            {
                p->num_refs++;
            }
            else
            {
                qWarning()<<"too many reference points (REF)";
            }
        }
        if (sscanf(line, "PLY/%d,%lf,%lf",
                   &p->ply[p->num_plys].id,
                   &p->ply[p->num_plys].lat,
                   &p->ply[p->num_plys].lon) == 3)
        {
            if ( p->num_plys < BSB_MAX_PLYS -1 )
            {
                p->num_plys++;
            }
            else
            {
                qWarning()<<"too many border points (PLY)";
            }

        }
        if (sscanf(line, "WPX/%d,", &p->wpx_level )==1)
        {
            p->num_wpxs = readNumberList( line, p->wpx, sizeof(p->wpx)/sizeof(p->wpx[0]) );
        }
        if (sscanf(line, "WPY/%d,", &p->wpy_level )==1)
        {
            p->num_wpys = readNumberList( line, p->wpy, sizeof(p->wpy)/sizeof(p->wpy[0]) );
        }
        if (sscanf(line, "PWX/%d,", &p->pwx_level )==1)
        {
            p->num_pwxs = readNumberList( line, p->pwx, sizeof(p->pwx)/sizeof(p->pwx[0]) );
        }
        if (sscanf(line, "PWY/%d,", &p->pwy_level )==1)
        {
            p->num_pwys = readNumberList( line, p->pwy, sizeof(p->pwy)/sizeof(p->pwy[0]) );
        }
        if ( (s = strstr(line, "NA=")) )
        {
            readStrUntilComma( s+3, p->name, sizeof(p->name) );
        }
        if ( strstr(line,"KNP/") == line )
        {
            if ( (s = strstr(line, "PR=")) )
            {
                readStrUntilComma( s+3, p->projection, sizeof(p->projection) );
            }
            if ( (s = strstr(line, "GD=")) )
            {
                readStrUntilComma( s+3, p->datum, sizeof(p->datum) );
            }
            if ( (s = strstr(line, "SC=")) )
            {
                sscanf( s+3, "%lf", &p->scale );
            }
            if ( (s = strstr(line, "PP=")) )
            {
                sscanf( s+3, "%lf", &p->projectionparam );
            }

        }
        if ( (s = strstr(line, "RA=")) )
        {
            int x0, y0;
            /* Attempt to read old-style NOS (4 parameter) version of RA= */
            /* then fall back to newer 2-argument version */
            if ((sscanf(s,"RA=%d,%d,%d,%d",&x0,&y0,&p->width,&p->height)!=4) &&
                (sscanf(s,"RA=%d,%d", &p->width, &p->height) != 2))
            {
                qWarning()<<"failed to read width,height from RA";
                return 0;
            }
        }
        if ( (s = strstr(line, "DX=")) )
        {
            if ( sscanf(s, "DX=%lf", &p->xresolution) != 1 )
            {
                qWarning()<<"failed to read x resolution";
                return 0;
            }
        }
        if ( (s = strstr(line, "DY=")) )
        {
            if ( sscanf(s, "DY=%lf", &p->yresolution) != 1 )
            {
                qWarning()<<"failed to read y resolution";
                return 0;
            }
        }
        if (sscanf(line, "IFM/%d", &ifm_depth) == 1)
        {
            p->depth = ifm_depth;
        }
        if (sscanf(line, "VER/%f", &p->version) == 1)
        {
        }
        if (sscanf(line, "CPH/%lf", &p->cph) == 1)
        {
        }

    }
    if (p->width == -1 || p->height == -1)
    {
        qWarning()<<"Error: Could not read RA=<width>,<height>";
        return 0;
    }
    /* done with the header */
    free(text_buf);

    /* Attempt to read depth from binary section, but first skip until NULL */
    while( fgetc(p->pFile) > 0 );

    /* Test depth from bitstream */
    depth = fgetc(p->pFile);
    if (depth != p->depth)
    {
        fprintf(stderr,
                "Warning: depth from IFM tag (%d) != depth from bitstream (%d)\n",
                p->depth, depth);
    }
    pos = ftell(p->pFile);
    if ( !bsb_read_row_index(p) )
    {
       /* printf("Could not read row index\n"); */
       /* restore file position so it starts at the first row again */
       /* TODO: provide a function to recreate the index */
       fseek(p->pFile, pos, SEEK_SET);
    }
    else
    {
       /* position at the first row - it is safer to use index if exists */
       bsb_seek_to_row(p, 0);
    }
    setlocale( LC_ALL, "" );
    int analyze_ret_val = AnalyzeRefpoints(p);
    if(0 != analyze_ret_val)
          return 0;
    return 1;
}


/**
 * generic polynomial to convert georeferenced lat/lon to char's x/y
 *
 * @param coeff list of polynomial coefficients
 * @param lon longitute or x
 * @param lat latitude or y
 *
 * @return coordinate corresponding to the coeff list
 */
static double polytrans( double* coeff, double lon, double lat )
{
    double ret = coeff[0] + coeff[1]*lon + coeff[2]*lat;
    ret += coeff[3]*lon*lon;
    ret += coeff[4]*lon*lat;
    ret += coeff[5]*lat*lat;
    ret += coeff[6]*lon*lon*lon;
    ret += coeff[7]*lon*lon*lat;
    ret += coeff[8]*lon*lat*lat;
    ret += coeff[9]*lat*lat*lat;
    ret += coeff[10]*lat*lat*lat*lat;
    ret += coeff[11]*lat*lat*lat*lat*lat;
    return ret;
}

/**
 * converts Lon/Lat to chart's X/Y
 *
 * @param p	pointer to a BSBImage structure
 * @param lon longitude (-180.0 to 180.0)
 * @param lat latitude (-180.0 to 180.0)
 * @param x  output chart X coordinate
 * @param y  output chart Y coordinate
 *
 * @return 1 on success and 0 on error
 */
extern int bsb_LLtoXY(BSBImage *p, double lon, double  lat, int* x, int* y)
{

    if(p->num_wpxs!=0 && p->num_wpys!=0) //has polynomials
    {
        double xd,yd;
        /* change longitude phase (CPH) */
        lon = (lon < 0) ? lon + p->cph : lon - p->cph;
        xd = polytrans( p->wpx, lon, lat );
        yd = polytrans( p->wpy, lon, lat );
        *x = (int)(xd + 0.5);
        *y = (int)(yd + 0.5);
        return 1;
    }
#if 1
    double easting, northing;
    lon = (lon < 0) ? lon + p->cph : lon - p->cph;
    double xlon = lon;
    double alat, alon;

    if(p->projection_type == PROJECTION_TRANSVERSE_MERCATOR)
    {
#if 1
          //      Use Projected Polynomial algorithm

          alon = lon /*+ m_lon_datum_adjust*/;
          alat = lat /*+ m_lat_datum_adjust*/;

          //      Get e/n from TM Projection
          toTM(alat, alon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

          //      Apply poly solution to target point
          double xd = polytrans( p->cPoints.wpx, easting, northing );
          double yd = polytrans( p->cPoints.wpy, easting, northing );
          *x = (int)(xd + 0.5);
          *y = (int)(yd + 0.5);
          return 1;
/*
          //      Apply poly solution to vp center point
          toTM(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
          double xc = polytrans( cPoints.wpx, easting, northing );
          double yc = polytrans( cPoints.wpy, easting, northing );

          //      Calculate target point relative to vp center
          double raster_scale = GetPPM() / vp.view_scale_ppm;

          int xs = (int)xc - (int)(vp.pix_width  * raster_scale / 2);
          int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

          int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
          int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

//                printf("  %d  %d  %d  %d\n", pixx, pixx_p, pixy, pixy_p);

          pixx = pixx_p;
          pixy = pixy_p;
*/
#endif
    }
    else if(p->projection_type == PROJECTION_MERCATOR)
    {
#if 1
          //      Use Projected Polynomial algorithm

          alon = lon /*+ m_lon_datum_adjust*/;
          alat = lat /*+ m_lat_datum_adjust*/;

          //      Get e/n from  Projection
          xlon = alon;
          if(p->m_bIDLcross)
          {
                if(xlon < 0.)
                      xlon += 360.;
          }
          toSM_ECC(alat, xlon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

          //      Apply poly solution to target point
          double xd = polytrans( p->cPoints.wpx, easting, northing );
          double yd = polytrans( p->cPoints.wpy, easting, northing );
          *x = (int)(xd + 0.5);
          *y = (int)(yd + 0.5);
          return 1;
/*

          //      Apply poly solution to vp center point
          double xlonc = vp.clon;
          if(p->m_bIDLcross)
          {
                if(xlonc < 0.)
                      xlonc += 360.;
          }

          toSM_ECC(vp.clat + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, p->m_proj_lat, p->m_proj_lon, &easting, &northing);
          double xc = polytrans( p->cPoints.wpx, easting, northing );
          double yc = polytrans( p->cPoints.wpy, easting, northing );
          //      Calculate target point relative to vp center
          double raster_scale = GetPPM() / vp.view_scale_ppm;

          int xs = (int)xc - (int)(vp.pix_width  * raster_scale / 2);
          int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

          int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
          int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

          pixx = pixx_p;
          pixy = pixy_p;
*/
#endif
    }
    else if(p->projection_type == PROJECTION_POLYCONIC)
    {
#if 1
          //      Use Projected Polynomial algorithm

          alon = lon /*+ m_lon_datum_adjust*/;
          alat = lat /*+ m_lat_datum_adjust*/;

          //      Get e/n from  Projection
          xlon = alon;
          if(p->m_bIDLcross)
          {
                if(xlon < 0.)
                      xlon += 360.;
          }
          toPOLY(alat, xlon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

          //      Apply poly solution to target point
          double xd = polytrans( p->cPoints.wpx, easting, northing );
          double yd = polytrans( p->cPoints.wpy, easting, northing );
          *x = (int)(xd + 0.5);
          *y = (int)(yd + 0.5);
          return 1;
/*
          //      Apply poly solution to vp center point
          double xlonc = vp.clon;
          if(m_bIDLcross)
          {
                if(xlonc < 0.)
                      xlonc += 360.;
          }

          toPOLY(vp.clat + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
          double xc = polytrans( cPoints.wpx, easting, northing );
          double yc = polytrans( cPoints.wpy, easting, northing );

          //      Calculate target point relative to vp center
          double raster_scale = GetPPM() / vp.view_scale_ppm;

          int xs = (int)xc - (int)(vp.pix_width  * raster_scale / 2);
          int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

          int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
          int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

          pixx = pixx_p;
          pixy = pixy_p;
*/
#endif
    }
    else
    {
#if 0
          toSM_ECC(lat, xlon, vp.clat, vp.clon, &easting, &northing);

          double epix = easting  * vp.view_scale_ppm;
          double npix = northing * vp.view_scale_ppm;

          double dx = epix * cos ( vp.skew ) + npix * sin ( vp.skew );
          double dy = npix * cos ( vp.skew ) - epix * sin ( vp.skew );

          pixx = ( int ) /*rint*/( ( vp.pix_width  / 2 ) + dx );
          pixy = ( int ) /*rint*/( ( vp.pix_height / 2 ) - dy );
#endif
    }
#endif
          return 0;
}
int   AnalyzeRefpoints(BSBImage *p)
{
    int projection_type=PROJECTION_UNKNOWN;
    QString pr=p->projection;
    pr=pr.toUpper();
    if(pr.contains("MERCATOR") || pr.contains("UNKNOWN"))
        projection_type=PROJECTION_MERCATOR;
    if(pr.contains("TRANSVERSE"))
        projection_type=PROJECTION_TRANSVERSE_MERCATOR;
    if(pr.contains("POLYCONIC"))
        projection_type=PROJECTION_POLYCONIC;
    if(pr.contains("TM"))
        projection_type=PROJECTION_TRANSVERSE_MERCATOR;
    if(projection_type==PROJECTION_UNKNOWN)
        return 1;
    p->projection_type=projection_type;
    p->m_proj_lat = 0.;
    p->m_proj_lon = 0.;
    //    Set up the projection point according to the projection parameter
    if(projection_type == PROJECTION_MERCATOR)
          p->m_proj_lat = p->projectionparam;
    else if(projection_type == PROJECTION_TRANSVERSE_MERCATOR)
          p->m_proj_lon = p->projectionparam;
    else if(projection_type == PROJECTION_POLYCONIC)
          p->m_proj_lon = p->projectionparam;
      int i,n;
      //double elt, elg;

//    Calculate the max/min reference points

      float lonmin = 1000;
      float lonmax = -1000;
      float latmin = 90.;
      float latmax = -90.;

      int plonmin = 100000;
      int plonmax = 0;
      int platmin = 100000;
      int platmax = 0;
      int nlonmin, nlonmax, nlatmax, nlatmin;
      nlonmin =0; nlonmax=0; nlatmax=0; nlatmin=0;

      if(0 == p->num_refs)                  // bad chart georef...
            return (1);

      for(n=0 ; n<p->num_refs ; n++)
      {
            //    Longitude
          if(p->ref[n].lon > lonmax)
            {
                  lonmax = p->ref[n].lon;
                  plonmax = (int)p->ref[n].x;
                  nlonmax = n;
            }
            if(p->ref[n].lon < lonmin)
            {
                  lonmin = p->ref[n].lon;
                  plonmin = (int)p->ref[n].x;
                  nlonmin = n;
            }

            //    Latitude
            if(p->ref[n].lat < latmin)
            {
                  latmin = p->ref[n].lat;
                  platmin = (int)p->ref[n].y;
                  nlatmin = n;
            }
            if(p->ref[n].lat > latmax)
            {
                  latmax = p->ref[n].lat;
                  platmax = (int)p->ref[n].y;
                  nlatmax = n;
            }
      }

      p->m_bIDLcross = false;
      //    Special case for charts which cross the IDL
      if((lonmin * lonmax) < 0)
      {
            if(p->ref[nlonmin].x > p->ref[nlonmax].x)
            {
                  //    walk the reference table and add 360 to any longitude which is < 0
                  for(n=0 ; n<p->num_refs ; n++)
                  {
                        if(p->ref[n].lon < 0.0)
                              p->ref[n].lon += 360.;
                  }

                  //    And recalculate the  min/max
                  lonmin = 1000;
                  lonmax = -1000;

                  for(n=0 ; n<p->num_refs ; n++)
                  {
            //    Longitude
                        if(p->ref[n].lon > lonmax)
                        {
                              lonmax = p->ref[n].lon;
                              plonmax = (int)p->ref[n].x;
                              nlonmax = n;
                        }
                        if(p->ref[n].lon < lonmin)
                        {
                              lonmin = p->ref[n].lon;
                              plonmin = (int)p->ref[n].x;
                              nlonmin = n;
                        }

            //    Latitude
                        if(p->ref[n].lat < latmin)
                        {
                              latmin = p->ref[n].lat;
                              platmin = (int)p->ref[n].y;
                              nlatmin = n;
                        }
                        if(p->ref[n].lat > latmax)
                        {
                              latmax = p->ref[n].lat;
                              platmax = (int)p->ref[n].y;
                              nlatmax = n;
                        }
                  }
                  p->m_bIDLcross = true;
            }
      }


//          Build the Control Point Structure, etc
        p->cPoints.count = p->num_refs;

        p->cPoints.tx  = (double *)malloc(p->num_refs * sizeof(double));
        p->cPoints.ty  = (double *)malloc(p->num_refs * sizeof(double));
        p->cPoints.lon = (double *)malloc(p->num_refs * sizeof(double));
        p->cPoints.lat = (double *)malloc(p->num_refs * sizeof(double));

        p->cPoints.pwx = (double *)malloc(12 * sizeof(double));
        p->cPoints.wpx = (double *)malloc(12 * sizeof(double));
        p->cPoints.pwy = (double *)malloc(12 * sizeof(double));
        p->cPoints.wpy = (double *)malloc(12 * sizeof(double));


        //  Find the two REF points that are farthest apart
        double dist_max = 0.;
        int imax = 0;
        int jmax = 0;

        for(i=0 ; i<p->num_refs ; i++)
        {
              for(int j=i+1 ; j < p->num_refs ; j++)
              {
                    double dx = p->ref[i].x - p->ref[j].x;
                    double dy = p->ref[i].y - p->ref[j].y;
                    double dist = (dx * dx) + (dy * dy);
                    if(dist > dist_max)
                    {
                          dist_max = dist;
                          imax = i;
                          jmax = j;
                    }
              }
        }

        //  Georef solution depends on projection type

        if(p->projection_type == PROJECTION_TRANSVERSE_MERCATOR)
        {
#if 1
              double easting0, easting1, northing0, northing1;
              //  Get the TMerc projection of the two REF points
              toTM(p->ref[imax].lat, p->ref[imax].lon, p->m_proj_lat, p->m_proj_lon, &easting0, &northing0);
              toTM(p->ref[jmax].lat, p->ref[jmax].lon, p->m_proj_lat, p->m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
              double dx2 =  (p->ref[jmax].x - p->ref[imax].x) *  (p->ref[jmax].x - p->ref[imax].x);
              double dy2 =  (p->ref[jmax].y - p->ref[imax].y) *  (p->ref[jmax].y - p->ref[imax].y);
              double dn2 =  (northing1 - northing0) * (northing1 - northing0);
              double de2 =  (easting1 - easting0) * (easting1 - easting0);

              p->m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

              //  Set up and solve polynomial solution for pix<->east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

              for(int n=0 ; n<p->num_refs ; n++)
              {
                    double easting, northing;
                    toTM(p->ref[n].lat, p->ref[n].lon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

                    p->cPoints.tx[n] = p->ref[n].x;
                    p->cPoints.ty[n] = p->ref[n].y;
                    p->cPoints.lon[n] = easting;
                    p->cPoints.lat[n] = northing;
              }

        //      Helper parameters
              p->cPoints.txmax = plonmax;
              p->cPoints.txmin = plonmin;
              p->cPoints.tymax = platmax;
              p->cPoints.tymin = platmin;
              toTM(latmax, lonmax,p-> m_proj_lat, p->m_proj_lon, &p->cPoints.lonmax, &p->cPoints.latmax);
              toTM(latmin, lonmin, p->m_proj_lat, p->m_proj_lon, &p->cPoints.lonmin, &p->cPoints.latmin);

              p->cPoints.status = 1;

              Georef_Calculate_Coefficients_Proj(&p->cPoints);
#endif
       }


       else if(p->projection_type == PROJECTION_MERCATOR)
       {
#if 1


             double easting0, easting1, northing0, northing1;
              //  Get the Merc projection of the two REF points
             toSM_ECC(p->ref[imax].lat, p->ref[imax].lon, p->m_proj_lat, p->m_proj_lon, &easting0, &northing0);
             toSM_ECC(p->ref[jmax].lat, p->ref[jmax].lon, p->m_proj_lat, p->m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
//             double dx =  (pRefTable[jmax].xr - pRefTable[imax].xr);
//             double de =  (easting1 - easting0);
//             m_ppm_avg = fabs(dx / de);

             double dx2 =  (p->ref[jmax].x - p->ref[imax].x) *  (p->ref[jmax].x - p->ref[imax].x);
             double dy2 =  (p->ref[jmax].y - p->ref[imax].y) *  (p->ref[jmax].y - p->ref[imax].y);
             double dn2 =  (northing1 - northing0) * (northing1 - northing0);
             double de2 =  (easting1 - easting0) * (easting1 - easting0);

             p->m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);


              //  Set up and solve polynomial solution for pix<->east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

             for(int n=0 ; n<p->num_refs ; n++)
             {
                   double easting, northing;
                   toSM_ECC(p->ref[n].lat, p->ref[n].lon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

                   p->cPoints.tx[n] = p->ref[n].x;
                   p->cPoints.ty[n] = p->ref[n].y;
                   p->cPoints.lon[n] = easting;
                   p->cPoints.lat[n] = northing;
//                   printf(" x: %g  y: %g  east: %g  north: %g\n",pRefTable[n].xr, pRefTable[n].yr, easting, northing);
             }

        //      Helper parameters
             p->cPoints.txmax = plonmax;
             p->cPoints.txmin = plonmin;
             p->cPoints.tymax = platmax;
             p->cPoints.tymin = platmin;
             toSM_ECC(latmax, lonmax, p->m_proj_lat, p->m_proj_lon, &p->cPoints.lonmax, &p->cPoints.latmax);
             toSM_ECC(latmin, lonmin, p->m_proj_lat, p->m_proj_lon, &p->cPoints.lonmin, &p->cPoints.latmin);

             p->cPoints.status = 1;

             Georef_Calculate_Coefficients_Proj(&p->cPoints);

//              for(int h=0 ; h < 10 ; h++)
//                    printf("pix to east %d  %g\n",  h, cPoints.pwx[h]);          // pix to lon
 //             for(int h=0 ; h < 10 ; h++)
//                    printf("east to pix %d  %g\n",  h, cPoints.wpx[h]);          // lon to pix

/*
             if ((0 != m_Chart_DU ) && (0 != m_Chart_Scale))
             {
                   double m_ppm_avg1 = m_Chart_DU * 39.37 / m_Chart_Scale;
                   m_ppm_avg1 *= cos(m_proj_lat * PI / 180.);                    // correct to chart centroid

                   printf("BSB chart ppm_avg:%g ppm_avg1:%g\n", m_ppm_avg, m_ppm_avg1);
                   m_ppm_avg = m_ppm_avg1;
             }
*/
#endif
       }

       else if(p->projection_type == PROJECTION_POLYCONIC)
       {
#if 1
             //   This is interesting
             //   On some BSB V 1.0 Polyconic charts (e.g. 14500_1, 1995), the projection parameter
             //   Which is taken to be the central meridian of the projection is of the wrong sign....

             //   We check for this case, and make a correction if necessary.....
             //   Obviously, the projection meridian should be on the chart, i.e. between the min and max longitudes....
             double proj_meridian = p->m_proj_lon;

             if((p->ref[nlonmax].lon >= -proj_meridian) && (-proj_meridian >= p->ref[nlonmin].lon))
                   p->m_proj_lon = -p->m_proj_lon;


             double easting0, easting1, northing0, northing1;
             //  Get the Poly projection of the two REF points
             toPOLY(p->ref[imax].lat, p->ref[imax].lon, p->m_proj_lat, p->m_proj_lon, &easting0, &northing0);
             toPOLY(p->ref[jmax].lat, p->ref[jmax].lon, p->m_proj_lat, p->m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
             double dx2 =  (p->ref[jmax].x - p->ref[imax].x) *  (p->ref[jmax].x - p->ref[imax].x);
             double dy2 =  (p->ref[jmax].y - p->ref[imax].y) *  (p->ref[jmax].y - p->ref[imax].y);
             double dn2 =  (northing1 - northing0) * (northing1 - northing0);
             double de2 =  (easting1 - easting0) * (easting1 - easting0);

             p->m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

             // Sanity check
//             double ref_dist = DistGreatCircle(pRefTable[imax].latr, pRefTable[imax].lonr, pRefTable[jmax].latr, pRefTable[jmax].lonr);
//             ref_dist *= 1852;                                    //To Meters
//             double ref_dist_transform = sqrt(dn2 + de2);         //Also meters
//             double error = (ref_dist - ref_dist_transform)/ref_dist;

              //  Set up and solve polynomial solution for pix<->cartesian east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

             for(int n=0 ; n<p->num_refs ; n++)
             {
                   double lata, lona;
                   lata = p->ref[n].lat;
                   lona = p->ref[n].lon;

                   double easting, northing;
                   toPOLY(p->ref[n].lat, p->ref[n].lon, p->m_proj_lat, p->m_proj_lon, &easting, &northing);

                   //   Round trip check for debugging....
//                   double lat, lon;
//                   fromPOLY(easting, northing, m_proj_lat, m_proj_lon, &lat, &lon);

                   p->cPoints.tx[n] = p->ref[n].x;
                   p->cPoints.ty[n] = p->ref[n].y;
                   p->cPoints.lon[n] = easting;
                   p->cPoints.lat[n] = northing;
//                   printf(" x: %g  y: %g  east: %g  north: %g\n",pRefTable[n].xr, pRefTable[n].yr, easting, northing);
             }

                     //      Helper parameters
             p->cPoints.txmax = plonmax;
             p->cPoints.txmin = plonmin;
             p->cPoints.tymax = platmax;
             p->cPoints.tymin = platmin;
             toPOLY(latmax, lonmax, p->m_proj_lat, p->m_proj_lon, &p->cPoints.lonmax, &p->cPoints.latmax);
             toPOLY(latmin, lonmin, p->m_proj_lat, p->m_proj_lon, &p->cPoints.lonmin, &p->cPoints.latmin);

             p->cPoints.status = 1;

             Georef_Calculate_Coefficients_Proj(&p->cPoints);

//              for(int h=0 ; h < 10 ; h++)
//                    printf("pix to east %d  %g\n",  h, cPoints.pwx[h]);          // pix to lon
//              for(int h=0 ; h < 10 ; h++)
//                    printf("east to pix %d  %g\n",  h, cPoints.wpx[h]);          // lon to pix
#endif
       }

       //   Any other projection had better have embedded coefficients
#if 0
       else if(bHaveEmbeddedGeoref)
       {
             //   Use a Mercator Projection to get a rough ppm.
             double easting0, easting1, northing0, northing1;
              //  Get the Merc projection of the two REF points
             toSM_ECC(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
             toSM_ECC(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math in x(longitude) direction

             double dx =  (pRefTable[jmax].xr - pRefTable[imax].xr);
             double de =  (easting1 - easting0);

             m_ppm_avg = fabs(dx / de);

             m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy may be poor. >>>---");
       }
#endif
       else
             p->m_ppm_avg = 1.0;                      // absolute fallback to prevent div-0 errors

#if 0


        // Do a last little test using a synthetic ViewPort of nominal size.....
        ViewPort vp;
        vp.clat = pRefTable[0].latr;
        vp.clon = pRefTable[0].lonr;
        vp.view_scale_ppm = m_ppm_avg;
        vp.skew = 0.;
        vp.pix_width = 1000;
        vp.pix_height = 1000;
//        vp.rv_rect = wxRect(0,0, vp.pix_width, vp.pix_height);
        SetVPRasterParms(vp);


        double xpl_err_max = 0;
        double ypl_err_max = 0;
        double xpl_err_max_meters, ypl_err_max_meters;
        int px, py;

        int pxref, pyref;
        pxref = (int)pRefTable[0].xr;
        pyref = (int)pRefTable[0].yr;

        for(i=0 ; i<nRefpoint ; i++)
        {
              px = (int)(vp.pix_width/2 + pRefTable[i].xr) - pxref;
              py = (int)(vp.pix_height/2 + pRefTable[i].yr) - pyref;

              vp_pix_to_latlong(vp, px, py, &elt, &elg);

              double lat_error  = elt - pRefTable[i].latr;
              pRefTable[i].ypl_error = lat_error;

              double lon_error = elg - pRefTable[i].lonr;

                    //  Longitude errors could be compounded by prior adjustment to ref points
              if(fabs(lon_error) > 180.)
              {
                    if(lon_error > 0.)
                          lon_error -= 360.;
                    else if(lon_error < 0.)
                          lon_error += 360.;
              }
              pRefTable[i].xpl_error = lon_error;

              if(fabs(pRefTable[i].ypl_error) > fabs(ypl_err_max))
                    ypl_err_max = pRefTable[i].ypl_error;
              if(fabs(pRefTable[i].xpl_error) > fabs(xpl_err_max))
                    xpl_err_max = pRefTable[i].xpl_error;

              xpl_err_max_meters = fabs(xpl_err_max * 60 * 1852.0);
              ypl_err_max_meters = fabs(ypl_err_max * 60 * 1852.0);

        }

        Chart_Error_Factor = fmax(fabs(xpl_err_max/(lonmax - lonmin)), fabs(ypl_err_max/(latmax - latmin)));

        //        Good enough for navigation?
        if(Chart_Error_Factor > .02)
        {
                    wxString msg = _("   VP Final Check: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_T(" is %5g"), Chart_Error_Factor);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
        }

        //  Try again with my calculated georef
        //  This problem was found on NOAA 514_1.KAP.  The embedded coefficients are just wrong....
        if((Chart_Error_Factor > .02) && bHaveEmbeddedGeoref)
        {
              wxString msg = _("   Trying again with internally calculated georef solution ");
              wxLogMessage(msg);

              bHaveEmbeddedGeoref = false;
              SetVPRasterParms(vp);

              xpl_err_max = 0;
              ypl_err_max = 0;

              pxref = (int)pRefTable[0].xr;
              pyref = (int)pRefTable[0].yr;

              for(i=0 ; i<nRefpoint ; i++)
              {
                    px = (int)(vp.pix_width/2 + pRefTable[i].xr) - pxref;
                    py = (int)(vp.pix_height/2 + pRefTable[i].yr) - pyref;

                    vp_pix_to_latlong(vp, px, py, &elt, &elg);

                    double lat_error  = elt - pRefTable[i].latr;
                    pRefTable[i].ypl_error = lat_error;

                    double lon_error = elg - pRefTable[i].lonr;

                    //  Longitude errors could be compounded by prior adjustment to ref points
                    if(fabs(lon_error) > 180.)
                    {
                          if(lon_error > 0.)
                                lon_error -= 360.;
                          else if(lon_error < 0.)
                                lon_error += 360.;
                    }
                    pRefTable[i].xpl_error = lon_error;

                    if(fabs(pRefTable[i].ypl_error) > fabs(ypl_err_max))
                          ypl_err_max = pRefTable[i].ypl_error;
                    if(fabs(pRefTable[i].xpl_error) > fabs(xpl_err_max))
                          xpl_err_max = pRefTable[i].xpl_error;

                    xpl_err_max_meters = fabs(xpl_err_max * 60 * 1852.0);
                    ypl_err_max_meters = fabs(ypl_err_max * 60 * 1852.0);

              }

              Chart_Error_Factor = fmax(fabs(xpl_err_max/(lonmax - lonmin)), fabs(ypl_err_max/(latmax - latmin)));

        //        Good enough for navigation?
              if(Chart_Error_Factor > .02)
              {
                    wxString msg = _("   VP Final Check with internal georef: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_(" is %5g"), Chart_Error_Factor);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
              }
              else
              {
                    wxString msg = _("   Result: OK, Internal georef solution used.");

                    wxLogMessage(msg);
              }

        }

#endif
      return(0);
}
/**
 * converts chart's X/Y to Lon/Lat
 *
 * @param p	pointer to a BSBImage structure
 * @param x chart X coordinate
 * @param y chart Y coordinate
 * @param lon output longitude (-180.0 to 180.0)
 * @param lat output latitude (-180.0 to 180.0)
 *
 * @return 1 on success and 0 on error
 */
extern int bsb_XYtoLL(BSBImage *p, int x, int y, double* lonout, double*  latout)
{
    double lon = polytrans( p->pwx, x, y );
    lon = (lon < 0) ? lon + p->cph : lon - p->cph;
    *lonout = lon;
    *latout = polytrans( p->pwy, x, y );
    return 1;
}


/**
 * Seeks the file to the given row so read_row can start reading.
 * Uses the index table at the end of the BSB file to quickly jump to a row.
 *
 * @param p	pointer to a BSBImage
 * @param row row to seek to starting from row 0 (BSB row 1)
 *
 * @returns 1 on success and 0 on error
 *
 */
extern int bsb_seek_to_row(BSBImage *p, int row)
{
    int st, start_of_index, start_of_rows;

    /* in case we did not cahced the index, read it here */
    if ( !p->row_index )
    {
        /* Read start-of-index offset */
        if (fseek(p->pFile, -4, SEEK_END) == -1)
            return 0;
        if (fread(&st, 4, 1, p->pFile) != 1)
            return 0;
        start_of_index = bsb_ntohl(st);
        /* Read start-of-rows offset */
        if (fseek(p->pFile, row*4 + start_of_index, SEEK_SET) == -1)
            return 0;
        if (fread(&st, 4, 1, p->pFile) != 1)
            return 0;
        start_of_rows = bsb_ntohl(st);
    }
    else
    {
        start_of_rows = p->row_index[row];
    }

    /* seek to row offset */
    if (fseek(p->pFile, start_of_rows, SEEK_SET) == -1)
        return 0;
    else
        return 1;
}

/* Table used for computing multiplier in bsb_read_row() */
static char mul_mask[8] = { 0, 63, 31, 15, 7, 3, 1, 0};

/**
 * Read currently seek-to row. Can continue to read more rows.
 *
 * @param p	pointer to a BSBImage containing file pointer at the start of a row
 *			this occurs after bsb_open_header() or bsb_seek_to_row()
 * @oaram buf output buffer for uncompressed pixel data
 *
 * @returns 1 on success and 0 on error
 */
extern int bsb_read_row(BSBImage *p, uint8_t *buf)
{
    int     c, multiplier, row_num = 0;
    int     pixel = 1, written = 0;

    /* The row number is stored in the low 7 bits of each byte.				*/
    /* The 8th bit indicates if row number is continued in the next byte.	*/
    do
    {
        c = fgetc(p->pFile);
        row_num = ((row_num & 0x7f) << 7) + c;
    } while (c >= 0x80);

    /* Rows are terminated by '\0'.  Note that rows can contain a '\0'	*/
    /* as part of the run-length data, so '\0' does not delimit rows.	*/
    /* (This occurs when multiplier is a multiple of 128 - 1)			*/
    while ((c = fgetc(p->pFile)) != '\0')
    {
        if (c == EOF)
        {
            fprintf(stderr, "Warning: EOF reading row %d\n", row_num);
            return 0;
        }

        pixel = (c & 0x7f) >> (7 - p->depth);
        multiplier = c & mul_mask[(int)p->depth];

        while (c >= 0x80)
        {
            c = fgetc(p->pFile);
            multiplier = (multiplier << 7) + (c & 0x7f);
        }
        multiplier++;
        /* limit impact of corrupt BSB data */
        /* For the lower depths, the "grain" of the multiplyer is		*/
        /* course, so don't write past the width of the buffer.			*/
        if ( written + multiplier > p->width )
        {
            multiplier = p->width - written;
        }
        memset( buf+written, pixel-1, multiplier );
        written += multiplier;
    }

    if (written < p->width)
    {
        int short_fall = p->width - written;

        /* It seems valid BSB rows sometimes don't include pixel data for	*/
        /* the very last pixel or two.  Perhaps the decoder is supposed		*/
        /* to merely repeat the last pixel until the width is reached.		*/
        /* A value of 8 was chosen as a guess since the intended behaviour	*/
        /* of a BSB reader in this situation is not known.					*/
        if (short_fall < 8)
        {
            /* Repeat the last pixel value for small short falls */
            while (written < p->width)
                buf[written++] = pixel - 1;
        }
        else
        {
            fprintf(stderr, "Warning: Short row for row %d written=%d width=%d\n", row_num, written, p->width);
            return 0;
        }
    }
    return 1;
}

/**
 * Seeks-to and reads given row.
 * This is performance improved version but it requires that row index is present.
 *
 * @param p	pointer to a BSBImage containing file pointer at the start of a row
 *			this occurs after bsb_open_header() or bsb_seek_to_row()
 * @param buf output buffer for uncompressed pixel data
 *
 * @returns 1 on success and 0 on error
 */
extern int bsb_read_row_at(BSBImage *p, int row, uint8_t *buf)
{
	return bsb_read_row_part(p, row, buf, 0, p->width);
}

/**
 * Seeks-to and reads part of a row.
 * This is performance improved version but it requires that row index is present.
 *
 * @param p	pointer to a BSBImage containing file pointer at the start of a row
 *			this occurs after bsb_open_header() or bsb_seek_to_row()
 * @param buf output buffer for uncompressed pixel data
 *
 * @param xoffset X offset in a row to start reading from
 * @param len number of points to read (length of buf)
 *
 * @returns 1 on success and 0 on error
 */
extern int bsb_read_row_part(BSBImage *p, int row, uint8_t *buf, int xoffset, int buflen)
{
    int len,size,cidx,c,row_num,maxWidth;
    int multiplier, pixel = 1, rowx = 0, bufidx = 0;
    unsigned char* rbuf;
	/* trying to read outside of image? */
	if( row >= p->height )
		return 0;
	if( xoffset >= p->width )
		return 0;
    len = buflen;
	if( xoffset+len > p->width )
	{
		len = p->width-xoffset;
	}
		
    /* if we failed to read row index use old routine */
    if ( !p->rbuf || !p->row_index || !p->row_index[row] )
    {
        bsb_seek_to_row( p, row );
        return bsb_read_row( p, buf );
    }

    if ( fseek( p->pFile, p->row_index[row], SEEK_SET ) == -1 )
        return 0;

    size = p->row_index[row+1]-p->row_index[row];
    rbuf = p->rbuf;
    /* read compressed row in one step */
    if ( fread( rbuf, size, 1, p->pFile ) != 1 )
    {
        return 0;
    }
	
    /* uncompress the row */
	
    /* The row number is stored in the low 7 bits of each byte.		*/
    /* The 8th bit indicates if row number is continued in the next byte.	*/
	
    cidx = 0;
    c=0;
    row_num = 0;
	do
    {
        c = rbuf[cidx++];
        row_num = ((row_num & 0x7f) << 7) + c;
    } while (c >= 0x80);

    maxWidth = xoffset + len;
	
    /* Rows are terminated by '\0'.  Note that rows can contain a '\0'	*/
    /* as part of the run-length data, so '\0' does not delimit rows.	*/
    /* (This occurs when multiplier is a multiple of 128 - 1)		*/
    while ((c = rbuf[cidx++]) != '\0' && bufidx < len)
    {
        pixel = (c & 0x7f) >> (7 - p->depth);
        multiplier = c & mul_mask[(int)p->depth];

        while (c >= 0x80)
        {
            c = rbuf[cidx++];
            multiplier = (multiplier << 7) + (c & 0x7f);
        }
        multiplier++;

		/* limit impact of corrupt BSB data */
        if (multiplier > maxWidth)
            multiplier = maxWidth-rowx;

        /* For the lower depths, the "grain" of the multiplyer is		*/
        /* course, so don't write past the width of the buffer.			*/
        if (rowx + multiplier > maxWidth)
        {
            multiplier = maxWidth-rowx;
        }
		if( rowx+multiplier > xoffset )
		{
			int step = rowx >= xoffset ? multiplier : multiplier-(xoffset-rowx);
			if( bufidx+step > len ) step = len-bufidx;
			memset(buf+bufidx, pixel-1, step);
			bufidx += step;
		}
        rowx += multiplier;
    }

	/* It seems valid BSB rows sometimes don't include pixel data for	*/
	/* the very last pixel or two.  Perhaps the decoder is supposed		*/
	/* to merely repeat the last pixel until the width is reached.		*/
	/* A value of 8 was chosen as a guess since the intended behaviour	*/
	/* of a BSB reader in this situation is not known.					*/
	/* Repeat the last pixel value for small short falls */
	if(bufidx < buflen)
		memset(buf+bufidx, pixel - 1, buflen-bufidx);
    return 1;
}

/**
 * Writes the row index to BSB file
 *
 * @param fp pointer to opened file to write the index to
 * @param height number of rows (the height of the raster/length of index[])
 * @param idx row index table to write
 *
 * @returns 1 on success and 0 on error
 */
extern int bsb_write_index(FILE *fp, int height, int idx[])
{
    int j;

    /* Write index table */
    for (j = 0; j < height + 1; j++)
    {
        /* Indices must be written as big-endian */
        if ((fputc(idx[j] >> 24, fp)) == EOF) return 0;
        if ((fputc((idx[j] & 0x00ff0000) >> 16, fp)) == EOF) return 0;
        if ((fputc((idx[j] & 0x0000ff00) >> 8, fp)) == EOF) return 0;
        if ((fputc(idx[j] & 0x000000ff, fp)) == EOF) return 0;
    }
    return 1;
}

/**
 * Convert an integer into the variable length encoded form used for row
 * numbers in the BSB bitstream.
 * @param row row number to store
 * @param p pointer to the place to store it
 *
 * @return number of bytes used to store it
 */
static int bsb_store_integer(int row, uint8_t *p)
{
    int     req_bytes, i, c;

    /* Using 32 bit integers, row numbers are stored in one to four bytes.	*/
    /* This limits row numbers to 2^28 - 1 (= 268,435,455).					*/
    /* Calculate required bytes to store integer using a table instead of	*/
    /* expensive calls to get log base 2.									*/

    req_bytes = 1;
    if (row > 2097151)                      /* 2^(7*3) - 1 */
        req_bytes = 4;
    else if (row > 16383)               /* 2^(7*2) - 1 */
        req_bytes = 3;
    else if (row > 127)             /* 2^(7*1) - 1 */
        req_bytes = 2;

    for (i = req_bytes - 1; i >= 0; i--)
    {
        c = (row >> i * 7) & 0x7f;

        if (i > 0)
            c |= 0x80;                      /* set sentinel high bit */

        *p++ = c;
    }
    return req_bytes;
}

/**
 * Compress one row of image and store it
 *
 * @param p         - pointer to a BSBImage for the width & depth values
 * @param row	    - row number stating at 0 (row 0 will be stored as BSB row 1)
 * @param aPixel    - array of uncompressed pixels
 * @param buf	    - output buffer for compressed bitstream
 *
 * @returns length of encoded bitstream
 *
 * Use a simple run length encoding, storing the run length in the
 * sentinel bits where possible.  Lower depth images have more room
 * to store this information.
 */
extern int bsb_compress_row(BSBImage *p, int row, const uint8_t *aPixel, uint8_t *buf)
{
    int     width = p->width, depth = p->depth;
    int     run_length, sentinel_bits, max_sentinel_rep, sentinel_mask;
    int     ibuf, ipixel;
    uint8_t last_pix, shifted_pix;

    /* Write the row number (add 1 since BSB rows number from 1 not 0) */
    ibuf = 0;
    ibuf = bsb_store_integer(row+1, buf);


    sentinel_bits = 7 - depth;      /* number of sentinel bits available */
    sentinel_mask = ((1 << depth) - 1) << (7 - depth);
    max_sentinel_rep = 1 << sentinel_bits;  /* max run length that fits	*/
    /* in sentinel bits			*/

    ipixel = 0;
    while ( ipixel < width )
    {
        last_pix = aPixel[ipixel];
        ipixel++;

        /* Count length of pixel 'run' - run length cannot be greater than width */
        run_length = 0;
        while ( (ipixel < width) && (aPixel[ipixel] == last_pix) )
        {
            ipixel++;
            run_length++;
        }
        ipixel--;

        /* BSB colormap never uses index 0, so add 1 to pixel index before use */
        shifted_pix = (last_pix + 1) << sentinel_bits;

        if ( run_length < max_sentinel_rep )
        {
            if ( run_length == 0 )
            {
                if ( (shifted_pix & 0xff) == 0 )
                    shifted_pix = sentinel_mask;
            }
            buf[ibuf++] = shifted_pix | run_length;
        }
        else
        {
            int     i, rc_bytes, rc_bits;
            int     rc = run_length / 2;

            rc_bits = 1;
            while (rc > 0)
            {
                rc = rc / 2;
                rc_bits++;
            }

            rc_bytes = (int)(rc_bits / 7);
            if ( rc_bits - rc_bytes * 7 > sentinel_bits )
                rc_bytes++;

            for ( i = rc_bytes; i > 0; i-- )
            {
                buf[ibuf + i] = run_length & 0x7f;
                run_length = run_length >> 7;
            }

            buf[ibuf] = shifted_pix | run_length;

            for ( i = 0; i < rc_bytes; i++ )
                buf[ibuf + i] |= 0x80;

            ibuf++;
            ibuf = ibuf + rc_bytes;
        }
        ipixel++;
    }
    buf[ibuf] = 0;                      /* terminate row with zero */
    return ibuf + 1;
}

/**
 * Close the BSB file and release all the memory.
 * Zeros all the files of the structure.
 *
 * @param p pointer to BSBImage structure to close
 *
 * @retrun 1 on success; 0 on failure (the structure was not opened)
 */
extern int bsb_close(BSBImage *p)
{
    if (p->pFile)
    {
        fclose(p->pFile);
        free(p->row_index);
        free(p->rbuf);
        p->pFile = 0;
        p->row_index = 0;
        p->rbuf = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

