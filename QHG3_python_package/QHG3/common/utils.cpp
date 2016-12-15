#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "utils.h"
#include "types.h"



//-----------------------------------------------------------------------------
// createGrid
//
double **createGrid(int iW, int iH, bool bInit) {
    double **aadData = new double *[iH];
    for (int i = 0; i < iH; ++i) {
        aadData[i] = new double[iW];
        if (bInit) {
            memset(aadData[i], 0, iW*sizeof(double));
        }
    }
    return aadData;

}

//-----------------------------------------------------------------------------
// createGrid
//
double **createGrid(int iW, int iH, double dBGVal) {
    double **aadData = new double *[iH];
    for (int i = 0; i < iH; ++i) {
        aadData[i] = new double[iW];
        for (int j = 0; j < iW; j++) {
            aadData[i][j] = dBGVal;
        }
    }
    return aadData;

}

//-----------------------------------------------------------------------------
// destroyGrid
//
void destroyGrid(double **ppGrid, int iW, int iH) {
    if (ppGrid != NULL) {
        for (int i = 0; i < iH; ++i) {
            if (ppGrid[i] != NULL) {
                delete[] ppGrid[i];
            }
        }
        delete[] ppGrid;
    }
}

//-----------------------------------------------------------------------------
// setGrid
//
void setGrid(double **ppGrid, int iW, int iH, double dVal) {
    if (ppGrid != NULL) {
        for (int i = 0; i < iH; ++i) {
            for (int j = 0; j < iW; ++j) {
                ppGrid[i][j] =  dVal;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// clearGrid
//
void clearGrid(double **ppGrid, int iW, int iH) {
    setGrid(ppGrid, iW, iH, 0);
    if (ppGrid != NULL) {
        for (int i = 0; i < iH; ++i) {
            memset(ppGrid[i], 0, iW*sizeof(double));
        }
    }
}

//-----------------------------------------------------------------------------
// nanGrid
//
void nanGrid(double **ppGrid, int iW, int iH) {
    setGrid(ppGrid, iW, iH, 0);
    if (ppGrid != NULL) {
        for (int i = 0; i < iH; ++i) {
            for (int j = 0; j < iW; ++j) {
                ppGrid[i][j] = dNaN;
            }
        }
    }
}




unsigned short crc_tab[16] = { 
  0x0000, 0x1081, 0x2102, 0x3183, 
  0x4204, 0x5285, 0x6306, 0x7387, 
  0x8408, 0x9489, 0xA50A, 0xB58B, 
  0xC60C, 0xD68D, 0xE70E, 0xF78F
};  

//-----------------------------------------------------------------------------
// CheckSum
// CRC16 implementation acording to CCITT standards 
//
unsigned short crc_update(unsigned short crc, unsigned char c) {
    crc = (((crc >> 4) & 0x0FFF) ^ crc_tab[((crc ^ c)      & 0x000F)]);
    crc = (((crc >> 4) & 0x0FFF) ^ crc_tab[((crc ^ (c>>4)) & 0x000F)]);
    return crc;
}


unsigned short crc_add(unsigned short crc, float *pfData, int iNumFloats) {
    unsigned char *p = (unsigned char*) pfData;
    for (unsigned int i = 0; i < sizeof(float)*iNumFloats; ++i, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}

unsigned short crc_add(unsigned short crc, int i) {
    unsigned char *p = (unsigned char*) (&i);
    for (unsigned int k = 0; k < sizeof(int); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}


unsigned short crc_add(unsigned short crc, float f) {
    unsigned char *p = (unsigned char*) (&f);
    for (unsigned int k = 0; k < sizeof(float); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}
unsigned short crc_add(unsigned short crc, double d) {
    unsigned char *p = (unsigned char*) (&d);
    for (unsigned int k = 0; k < sizeof(double); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}
unsigned short crc_add(unsigned short crc, spcid i) {
    unsigned char *p = (unsigned char*) (&i);
    for (unsigned int k = 0; k < sizeof(double); ++k, ++p) {
        crc = crc_update(crc, *p);
    }
    return  crc;
}

//-----------------------------------------------------------------------------
// niceNum
//
char *niceNum(char *pNum, double dNum) {
    if (fabs(dNum) > 1e5) {
        sprintf(pNum, "%e", dNum);
    } else {
        sprintf(pNum, "%f", dNum);
    }
    return pNum;
}
