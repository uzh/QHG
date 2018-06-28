#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "utils.h"
#include "types.h"


const float  fNaN = __nan_valf.__f;
const double dNaN = __nan_val.__d;
/*
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


*/



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
