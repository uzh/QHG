#ifndef __SHPUTILS_H__
#define __SHPUTILS_H__

#ifndef uchar
typedef unsigned char uchar;
#endif
#ifndef uint
typedef unsigned int  uint;
#endif

#define BIGENDIAN true
#define LITTLEENDIAN false


#define SHP_NULL 0
#define SHP_POINT 1
#define SHP_POLYLINE 3
#define SHP_POLYGON 5
#define SHP_MULTIPOINT 8
#define SHP_POINTZ 11
#define SHP_POLYLINEZ 13
#define SHP_POLYGONZ 15
#define SHP_MULTIPOINTZ 18
#define SHP_POINTM 21
#define SHP_POLYLINEM 23
#define SHP_POLYGONM 25
#define SHP_MULTIPOINTM 28
#define SHP_MULTIPATCH 31

typedef struct numname {
    int         iNum;
    const char *pName;
} numname;

typedef struct mbr {
    double dXmin;
    double dYmin;
    double dXmax;
    double dYmax;
} mbr;

class shpUtils {
public:
    static uchar *getNum(uchar *p, short *piNum, bool bBigEndian);
    static uchar *getNum(uchar *p, int *piNum, bool bBigEndian);
    static uchar *getNum(uchar *p, double *pdNum);
    static int toLittleEndian(short iNum);
    static int toLittleEndian(int iNum);
    static const char *getShapeName(int iShapeNum);
   
    static uchar *getMBR(uchar *p, mbr mMBR);
 
};

#endif
