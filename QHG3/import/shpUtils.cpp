#include <stdio.h>
#include <string.h>

#include "shpUtils.h"

numname s_list[14] = {
    {SHP_NULL,        "Null shape"},
    {SHP_POINT,       "Point"},
    {SHP_POLYLINE,    "Polyline"},	
    {SHP_POLYGON,     "Polygon"},
    {SHP_MULTIPOINT,  "MultiPoint"},
    {SHP_POINTZ,      "PointZ"},
    {SHP_POLYLINEZ,   "PolylineZ"},
    {SHP_POLYGONZ,    "PolygonZ"},
    {SHP_MULTIPOINTZ, "MultiPointZ"},
    {SHP_POINTM,      "PointM"},
    {SHP_POLYLINEM,   "PolylineM"},
    {SHP_POLYGONM,    "PolygonM"},
    {SHP_MULTIPOINTM, "MultiPointM"},
    {SHP_MULTIPATCH,  "MultiPatch"},      
};


//----------------------------------------------------------------------------
// getNum
//
uchar *shpUtils::getNum(uchar *p, short *piNum, bool bBigEndian) {
    *piNum =  *((short*)p);
    if (bBigEndian) {
        *piNum = toLittleEndian(*piNum);
    }
    p += sizeof(short);
    return p;
}

//----------------------------------------------------------------------------
// getNum
//
uchar *shpUtils::getNum(uchar *p, int *piNum, bool bBigEndian) {
    *piNum =  *((int*)p);
    if (bBigEndian) {
        *piNum = toLittleEndian(*piNum);
    }
    p += sizeof(int);
    return p;
}


//----------------------------------------------------------------------------
// getNum
//
uchar *shpUtils::getNum(uchar *p, double *pdNum) {
    *pdNum =  *((double*)p);
    p += sizeof(double);
    return p;
}


//----------------------------------------------------------------------------
// getShapeName
//
const char *shpUtils::getShapeName(int iShapeNum) {
    const char *p = NULL;
    for (uint i = 0; (p == NULL) && (i < sizeof(s_list)/sizeof(numname)); i++) {
        if (s_list[i].iNum == iShapeNum) {
            p = s_list[i].pName;
        }
    }
    return p;
}


//----------------------------------------------------------------------------
// getMBR
//
uchar *shpUtils::getMBR(uchar *p, mbr mMBR) {
    memcpy(&mMBR, p, sizeof(mbr));
    p += sizeof(mbr);
    return p;
}


//----------------------------------------------------------------------------
// toLittleEndian
//
int shpUtils::toLittleEndian(int iNum) {
    uchar p[4];
    uchar *p0 = (uchar *)(&iNum);
    for (int i = 0; i < 4; i++) {
        p[i] = p0[3-i];
    }
    int iRes;
    memcpy(&iRes, p, 4*sizeof(uchar));
    return iRes;
}

//----------------------------------------------------------------------------
// toLittleEndian
//
int shpUtils::toLittleEndian(short iNum) {
    uchar p[2];
    uchar *p0 = (uchar *)(&iNum);
    p[0] = p0[1];
    p[1] = p0[0];
    short sRes;
    memcpy(&sRes, p, 2*sizeof(uchar));
    return sRes;
}
