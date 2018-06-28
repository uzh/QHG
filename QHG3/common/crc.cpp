/*============================================================================
| crc
|
|  CRC16 implementation acording to CCITT standards 
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
#include "types.h"

ushort crc_tab[16] = { 
  0x0000, 0x1081, 0x2102, 0x3183, 
  0x4204, 0x5285, 0x6306, 0x7387, 
  0x8408, 0x9489, 0xA50A, 0xB58B, 
  0xC60C, 0xD68D, 0xE70E, 0xF78F
};  


//-----------------------------------------------------------------------------
// CheckSum
// CRC16 implementation acording to CCITT standards 
//
ushort crc_update(ushort crc, uchar c) {
    crc = (((crc >> 4) & 0x0FFF) ^ crc_tab[((crc ^ c)      & 0x000F)]);
    crc = (((crc >> 4) & 0x0FFF) ^ crc_tab[((crc ^ (c>>4)) & 0x000F)]);
    return crc;
}


ushort crc_add(ushort crc, float *pfData, int iNumFloats) {
    uchar *p = (uchar*) pfData;
    for (uint i = 0; i < sizeof(float)*iNumFloats; ++i, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}


ushort crc_add(ushort crc, int i) {
    uchar *p = (uchar*) (&i);
    for (uint k = 0; k < sizeof(int); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}


ushort crc_add(ushort crc, float f) {
    uchar *p = (uchar*) (&f);
    for (uint k = 0; k < sizeof(float); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}


ushort crc_add(ushort crc, double d) {
    uchar *p = (uchar*) (&d);
    for (uint k = 0; k < sizeof(double); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}


ushort crc_add(ushort crc, spcid i) {
    uchar *p = (uchar*) (&i);
    for (uint k = 0; k < sizeof(double); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}
