#ifndef BSB_INCLUDED
#define BSB_INCLUDED
/*
 *  bsb.h	- libbsb types and functions
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
 *  $Id: bsb.h,v 1.14 2007/02/18 06:12:50 mikrom Exp $
 *
 */

#include <stdio.h>
#include <qglobal.h>
#ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
#else
// not ISO C99 - use best guess instead
typedef quint8 uint8_t;
typedef quint16 uint16_t;
typedef quint32 uint32_t;
#endif

#define BSB_MAX_REFS 512
#define BSB_MAX_PLYS 200
#define BSB_MAX_AFTS 200

typedef struct BSBImage
{
    char    name[200];
    char    projection[50];
    char    datum[50];
    char    depth;
    float   version;
    int     width;
    int     height;
    double  xresolution;
    double  yresolution;
    double  scale;
    /* usually 'scale given at latitude' with mercator projection */
    double  projectionparam;

    uint8_t red[256];
    uint8_t green[256];
    uint8_t blue[256];
    char    num_colors;

    /* geo reference points */
    struct REF
    {
        int id;
        int x;
        int y;
        double lon;
        double lat;
    } ref[BSB_MAX_REFS];
    int num_refs;

    /* chart border points */
    struct PLY
    {
        int id;
        double lat;
        double lon;
    } ply[BSB_MAX_PLYS];
    int num_plys;

    /* geotransforms from/to lat/lon & X,Y are polynomials */

    /* wpx,wpy - world to pixel */
    double wpx[BSB_MAX_AFTS];
    int num_wpxs;
    int wpx_level;
    double wpy[BSB_MAX_AFTS];
    int num_wpys;
    int wpy_level;
    /* pwx,pwy - pixel to world */
    double pwx[BSB_MAX_AFTS];
    int num_pwxs;
    int pwx_level;
    double pwy[BSB_MAX_AFTS];
    int num_pwys;
    int pwy_level;
    /* phase change for charts crossing 180 longitude */
    double cph; 

    /* private: */
    FILE* pFile;
    uint32_t* row_index;
    unsigned char* rbuf;
} BSBImage;

#ifdef __cplusplus
extern "C" {
#endif

/* See comments in bsb_io.c for documentation on these functions */

extern int bsb_get_header_size(FILE *fp);
extern int bsb_open_header(char *filename, BSBImage *p);
extern int bsb_seek_to_row(BSBImage *p, int row);
extern int bsb_read_row(BSBImage *p, uint8_t *buf);
extern int bsb_read_row_at(BSBImage *p, int row, uint8_t *buf);
extern int bsb_read_row_part(BSBImage *p, int row, uint8_t *buf, int xoffset, int len);
extern int bsb_LLtoXY(BSBImage *p, double lon, double  lat, int* x, int* y);
extern int bsb_XYtoLL(BSBImage *p, int x, int y, double* lon, double*  lat);
extern int bsb_compress_row(BSBImage *p, int row, const uint8_t *pixel, uint8_t *buf);
extern int bsb_write_index(FILE *fp, int height, int index[]);
extern int bsb_close(BSBImage *p);

#ifdef __cplusplus
}
#endif

#endif /* BSB_INCLUDED */
