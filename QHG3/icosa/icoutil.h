#ifndef __ICOUTIL_H__
#define __ICOUTIL_H__

#include <math.h>
#include "Vec3D.h"


#define POLY_TYPE_NONE -1
#define POLY_TYPE_ICO 0
#define POLY_TYPE_OCT 1
#define POLY_TYPE_TET 2

#define NOTIFY_LOAD          1
#define NOTIFY_SWITCH        2
#define NOTIFY_ICO_MODE      3
#define NOTIFY_FLAT_PROJ     4
#define NOTIFY_FLAT_LINK     5
#define NOTIFY_TILE_SPLITTER 6
#define NOTIFY_NEW_GP        7
#define NOTIFY_NEW_H         8
#define NOTIFY_CREATED       9
#define NOTIFY_TILED        10
#define NOTIFY_ROT_MODE     11


static const int  MODE_POINTS = 0;
static const int  MODE_LINES  = 1;
static const int  MODE_PLANES = 2;


#define MODE_ICO_FULL  0x00000001
#define MODE_ICO_RECT  0x00000002
#define MODE_ICO_LAND  0x00000003

#define MODE_RECT_QUAD 0x00001000
#define MODE_RECT_HEX  0x00002000

#define STATE_ICO      0x00010000
#define STATE_FLAT     0x00020000

#define MASK_ICO_MODE   0x0000000f
#define MASK_FLAT_PROJ  0x0000000f
#define MASK_FLAT_LINK  0x0000f000


typedef struct box {
    double dLonMin;
    double dLonMax;
    double dLatMin;
    double dLatMax;
     box(double _dLonMin, double _dLonMax, double _dLatMin, double _dLatMax) : dLonMin(_dLonMin), dLonMax(_dLonMax), dLatMin(_dLatMin), dLatMax(_dLatMax){};
     box() : dLonMin(0), dLonMax(0), dLatMin(0), dLatMax(0){};
} tbox;


inline void cart2Sphere(Vec3D *v, double *pdLon, double *pdLat) {
    double r = v->calcNorm();
    *pdLat = asin(v->m_fZ/r);
    *pdLon = atan2(v->m_fY, v->m_fX);
}



#endif
