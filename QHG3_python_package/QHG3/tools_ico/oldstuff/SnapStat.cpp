#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "icoutil.h"
#include "strutils.h"
#include "NodeLister.h"
#include "SnapHeader.h"

const int MASK_ERR   =   -1;
const int MASK_NONE  =    0;
const int MASK_SUM   =    1;
const int MASK_COUNT =    2;
const int MASK_AVG   =    4;
const int MASK_VAR   =    8;
const int MASK_INF   =   16;
const int MASK_NAN   =   32;
const int MASK_TOTAL =   64;
const int MASK_MIN   =  128;
const int MASK_MAX   =  256;

const int MASK_ALL   = 1023;

const char *OPTS = "scavintmM";

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
    printf("  'v' : variance of normal elements\n");
    printf("  'i' : count of infinite elements\n");
    printf("  'n' : count of nan elements\n");
    printf("  't' : total number of elements\n");
    printf("  'm' : minimum value\n");
    printf("  'M' : maximum value\n");
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

        nodelist nlVal;
        double   dMax;
        double   dMin;

        iResult = NodeLister::createList(apArgV[iFilePos], nlVal, &dMin, &dMax);

        if (iResult == 0) {
        
            double dSum  = 0;
            double dSum2 = 0;
            double dMin = dPosInf;
            double dMax = dNegInf;
            int iCount = 0;
            int iNaNCount = 0;
            int iInfCount = 0;

            nodelist::const_iterator it = nlVal.begin();
            
            while (it != nlVal.end()) {
                double d = it->second;
                
                if (isfinite(d)) {
                    dSum += d;
                    dSum2 += d*d;
                    
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
                it++;
            }
            
            double dAvg = dSum/iCount;

            if ((iMask & MASK_TOTAL) != 0) {
                printf("Total:  %zd Elements\n", nlVal.size());
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
                printf("Sum: %e\n", dSum);
            }
            if ((iMask & MASK_AVG) != 0) {
                printf("Avg: %e\n", dAvg);
            }
            if ((iMask & MASK_VAR) != 0) {
                printf("Var: %e\n", dSum2/iCount-dAvg*dAvg);
            }
            if ((iMask & MASK_MIN) != 0) {
                printf("Min: %e\n", dMin);
            }
            if ((iMask & MASK_MAX) != 0) {
                printf("Max: %e\n", dMax);
            }

        } else {
            printf("Couldn't create node list for [%s]\n", apArgV[iFilePos]);
        }

    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
