#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "QConverter.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"

const int MASK_ERR     =   -1;
const int MASK_NONE    =    0;
const int MASK_SUM     =    1;
const int MASK_COUNT   =    2;
const int MASK_AVG     =    4;
const int MASK_INF     =    8;
const int MASK_NAN     =   16;
const int MASK_SIZE    =   32;
const int MASK_TOTAL   =   64;
const int MASK_MIN     =  128;
const int MASK_MAX     =  256;
const int MASK_NUMERIC =  512;

const int MASK_ALL_NUM = 1023;
const int MASK_ALL     = MASK_ALL_NUM-MASK_NUMERIC;

const char *OPTS = "scainztmMN";

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char * pApp) {
    printf("%s - get some statistics of a QMap\n", pApp);
    printf("usage:\n");
    printf("  %s [-<opt>]<QMap>\n", pApp);
    printf("where\n");
    printf("  opt is a string composed of any of the letters\n");
    printf("  's' : sum of normal elements\n");
    printf("  'c' : count of normal elements\n");
    printf("  'a' : average of normal elements\n");
    printf("  'i' : count of infinite elements\n");
    printf("  'n' : count of nan elements\n");
    printf("  'z' : size of QMap\n");
    printf("  't' : total number of elements\n");
    printf("  'm' : minimum value\n");
    printf("  'M' : maximum value\n");
    printf("  'N' : show as integer\n");
    printf("or\n");
    printf("  opt is 'A', show all\n");
    printf("\n");
}

//-------------------------------------------------------------------------------------------------
// collectMask
//
int collectMask(char *pOpt) {
    int iMask = MASK_ERR;
    if (*pOpt == '-') {
        pOpt++;
        if (*pOpt == 'A') {
            iMask = MASK_ALL;
            if (strchr(pOpt, 'N')) {
                iMask = MASK_ALL_NUM;
            }
        } else if (*pOpt == 'N') {
            iMask = MASK_ALL_NUM;
        } else {
            iMask = MASK_NONE;
            while (*pOpt != '\0') {
                const char *p = strchr(OPTS, *pOpt);
                if (p!= NULL) {
                    int i = p - OPTS;
                    iMask |= 1 << i; 
                }
                pOpt++;
            }
        }
    }
    return iMask;
}


//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    int iMask = MASK_ALL;
    int iFilePos = 1;
    if (iArgC > 1) {
        if (iArgC > 2) {
            iFilePos = 2;
            iMask = collectMask(apArgV[1]);
        }
        int iType=QMAP_TYPE_NONE;
        ValReader *pVR = QMapUtils::createValReader(apArgV[iFilePos], false, &iType);
        if (pVR != NULL) {
            iResult = 0;
            unsigned int iW = pVR->getNRLon();
            unsigned int iH = pVR->getNRLat();

            double dSum = 0;
            double dMin = dPosInf;
            double dMax = dNegInf;
            int iCount = 0;
            int iNaNCount = 0;
            int iInfCount = 0;
            for (unsigned int i = 0; i < iH; i++) {
                for (unsigned int j = 0; j < iW; j++) {
                    double d = pVR->getDValue(j, i);
                    if (isfinite(d)) {
                        dSum += d;
                        if (d < dMin) {
                            dMin = d;
                        }
                        if (d > dMax) {
                            dMax = d;
                        }

                        iCount++;
                    } else { 
                        if (isnan(d)) {
                            iNaNCount++;
                        } else {
                            iInfCount++;
                        }
                    }
                }
            }
            
            
            if ((iMask & MASK_SIZE) != 0) {
                    printf("Size %dx%d type %s\n", iW, iH, QMapHeader::getTypeName(iType)); 
            }
            if ((iMask & MASK_TOTAL) != 0) {
            printf("Total:  %d Elements\n", iW*iH);
            }
            if ((iMask & MASK_NAN) != 0) {
            printf("NaN:    %d\n", iNaNCount);
            }
            if ((iMask & MASK_INF) != 0) {
            printf("Inf:    %d\n", iInfCount);
            }
            if ((iMask & MASK_COUNT) != 0) {
            printf("Normal: %d\n", iCount);
            }
            if ((iMask & MASK_SUM) != 0) {
                if ((iMask & MASK_NUMERIC) != 0) {
                    printf("Sum: %d\n", (int) dSum);
                } else {
                    printf("Sum: %e\n", dSum);
                }
            }
            if ((iMask & MASK_AVG) != 0) {
            printf("Avg: %e\n", dSum/iCount);
            }
            if ((iMask & MASK_MIN) != 0) {
                if ((iMask & MASK_NUMERIC) != 0) {
                    printf("Min: %d\n", (int) dMin);
                } else {
                    printf("Min: %e\n", dMin);
                }
            }
            if ((iMask & MASK_MAX) != 0) {
                if ((iMask & MASK_NUMERIC) != 0) {
                    printf("Max: %d\n", (int) dMax);
                } else {
                    printf("Max: %e\n", dMax);
                }
            }

        } else {
            printf("Couldn't create ValReader for [%s]\n", apArgV[1]);
            if (iType == QMAP_TYPE_NULL) {
                printf("  File didn't exxist\n");
            } else {
                printf("  Probably not a valid qmap\n");
            }
        }

    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
