#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "types.h"
#include "strutils.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"


const char *KEY_NAN    = "nan";
const char *KEY_POSINF = "posinf";
const char *KEY_NEGINF = "neginf";
const char *KEY_NUM    = "num";
const char *KEY_INV    = "inv";

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - create a binary mask file for given qmap\n", pApp);
    printf("usage:\n");
    printf("  %s <QMap1> <val> [\"inv\"] <QMapOut>\n", pApp);
    printf("where\n");
    printf("  QMap1    : name of input QMap\n");
    printf("  val      : value of QMap1 for which output is set to 1\n");
    printf("  inv      : flag to signal inversion of mask\n");
    printf("  QMapOut  : name of output QMap\n");
    printf("\n");
    printf("A new QMap is created which has 1 in every position in which the input qmap has the specified value.\n");
    printf("val can be any numeric value includiong the special values \"num\", \"neginf\", \"posinf\", and \"nan\".\n");
    printf("In the second case the operator is applied to each element o QMap1 and the number\n");
    printf("\n");
}


//-------------------------------------------------------------------------------------------------
// maskNumber
//
template<class T>
uchar **maskNumber(T **ppData1, int iW, int iH, T tRefVal, uchar uEqual, uchar uDiff) {
    printf("[maskNumeric] %f %dx%d\n", tRefVal*1.0, iW, iH);
    uchar **ppData3 = new uchar*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new uchar[iW];
        for (int j = 0; j < iW; j++) {
            ppData3[i][j] = (ppData1[i][j] == tRefVal)?uEqual:uDiff;
        }
    }
    return ppData3;
}

//-------------------------------------------------------------------------------------------------
// maskNaN
//
template<class T>
uchar **maskNaN(T **ppData1, int iW, int iH, uchar uEqual, uchar uDiff) {
    printf("[maskNaN] %dx%d\n", iW, iH);
    uchar **ppData3 = new uchar*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new uchar[iW];
        for (int j = 0; j < iW; j++) {
            ppData3[i][j] = isnan(ppData1[i][j])?uEqual:uDiff;
        }
    }
    return ppData3;
}

//-------------------------------------------------------------------------------------------------
// maskInf
//    relying on isinf() to return 1 for posinf and -1 for neginf
//
template<class T>
uchar **maskInf(T **ppData1, int iW, int iH, int iSignum,  uchar uEqual, uchar uDiff) {
    printf("[maskInf] %d %dx%d\n", iSignum, iW, iH);
    uchar **ppData3 = new uchar*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new uchar[iW];
        for (int j = 0; j < iW; j++) {
            ppData3[i][j] = (isinf(ppData1[i][j])==iSignum)?uEqual:uDiff;
        }
    }
    return ppData3;
}


//-------------------------------------------------------------------------------------------------
// maskNormal
//    relying on isinf() to return 1 for posinf and -1 for neginf
//
template<class T>
uchar **maskNormal(T **ppData1, int iW, int iH,  uchar uEqual, uchar uDiff) {
    printf("[maskNormal] %dx%d\n", iW, iH);
    uchar **ppData3 = new uchar*[iH];
    for (int i = 0; i < iH; i++) {
        ppData3[i] = new uchar[iW];
        for (int j = 0; j < iW; j++) {
            ppData3[i][j] = isfinite(ppData1[i][j])?uEqual:uDiff;
        }
    }
    return ppData3;
}

//-------------------------------------------------------------------------------------------------
// maskAll
//  
//
template<class T>
    int  maskAll(ValReader *pVR1, T tDummy, char *pVal, bool bInverse, char *pOut, QMapHeader *pQMH) {
    int iResult = 0;
    int iH = pVR1->getNRLat();
    int iW = pVR1->getNRLon();
    
    QMapReader<T> *pQMR1 =dynamic_cast<QMapReader<T> *> (pVR1);

    uchar uEqual = bInverse?0:1;
    uchar uDiff  = bInverse?1:0;
    
    T **ppData1 = pQMR1->getData();

    uchar **ppData3=NULL;
    
    if (strcasecmp(KEY_NAN, pVal)==0){
        ppData3 = maskNaN(ppData1, iW, iH, uEqual, uDiff);
    } else if (strcasecmp(KEY_POSINF, pVal)==0) {
        ppData3 = maskInf(ppData1, iW, iH, 1, uEqual, uDiff);
    } else if (strcasecmp(KEY_NEGINF, pVal)==0) {
        ppData3 = maskInf(ppData1, iW, iH, -1, uEqual, uDiff);
    } else if (strcasecmp(KEY_NUM, pVal)==0) {
        ppData3 = maskNormal(ppData1, iW, iH, uEqual, uDiff);
    } else {
        char *pEnd;
        double dVal = strtod(pVal, &pEnd);
        if (*pEnd == '\0') {
            T tRefVal = (T)dVal;
            ppData3 = maskNumber(ppData1, iW, iH, tRefVal, uEqual, uDiff);
        } else {
            printf("Couldn't convert [%s] to a number\n",pVal);
            iResult = -1;
        }
    }

    if (iResult == 0) {
    
        // write output

        FILE *fOut = fopen(pOut, "wb");
        if (fOut != NULL) {
            
            pQMH->addHeader(fOut);
            
            for (int i = 0; i < iH; i++) {
                fwrite(ppData3[i], sizeof(uchar), iW, fOut);
            }
            fclose(fOut);
            iResult = 0;
        } else {
            printf("Couldn't open %s\n", pOut);
        }
    }

    return iResult;
}




//-------------------------------------------------------------------------------------------------
// operate
//  call the correct binCombine depending on the type
//
int operate(ValReader *pVR1, int iType, char *pVal, bool bInverse, char *pOut, QMapHeader *pQMH) {
    int iResult = -1; 

    switch (iType) {
    case QMAP_TYPE_UCHAR: {
        uchar uudef = 0xff;
        iResult = maskAll(pVR1, uudef, pVal, bInverse, pOut, pQMH);
    }
        break;
    case QMAP_TYPE_SHORT: {
        short int s = -1;
        iResult = maskAll(pVR1, s, pVal, bInverse, pOut, pQMH);
    }
        break;
    case QMAP_TYPE_INT : {
        int s = -1;
        iResult = maskAll(pVR1, s, pVal, bInverse, pOut, pQMH);
    }
        break;
    case QMAP_TYPE_LONG : {
        long s = -1;
        iResult = maskAll(pVR1, s, pVal, bInverse, pOut, pQMH);
    }
        break;
    case QMAP_TYPE_FLOAT: {
        iResult = maskAll(pVR1, fNaN, pVal, bInverse, pOut, pQMH);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        iResult = maskAll(pVR1, dNaN, pVal, bInverse, pOut, pQMH);
    }
        break;
    default:
        iResult = -1;
    }

    return iResult;
}


//-------------------------------------------------------------------------------------------------
// main
//  arguments: 
//    <QMap1> <OpName> <QMap2> <QmapOut>
//  or
//    <QMap1> <OpName> <number> <QmapOut>
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sInput[LONG_INPUT];
    char sOutput[LONG_INPUT];
    bool bInverse = false;

    if (iArgC > 3) {
        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sInput);
        iResult = 0;
        if (iArgC > 4) {
            strcpy(sOutput, apArgV[4]);
            if (strcasecmp(KEY_INV, apArgV[3]) == 0) {
                bInverse = true;
            } else {
                printf("unknown argument [%s]\n", apArgV[3]);
                iResult = -1;
            }
        } else {
            strcpy(sOutput, apArgV[3]);
        }
        if (iResult == 0) {
            int iType;
            ValReader *pVR1 = QMapUtils::createValReader(sInput, false, &iType);
            if (pVR1 != NULL) {
                printf("Masking %s in [%s]%s, output [%s]\n", apArgV[2], sInput, bInverse?" inverted":"",sOutput);
                // QMHeader for output : same data as pVR1
                QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_UCHAR,
                                                  pVR1->getLonMin(), pVR1->getLonMax(), pVR1->getDLon(),
                                                  pVR1->getLatMin(), pVR1->getLatMax(), pVR1->getDLat(),
                                                  pVR1->getVName(),  pVR1->getXName(),  pVR1->getYName());
                
                iResult = operate(pVR1, iType, apArgV[2], bInverse, sOutput, pQMH);
                
                if (iResult == 0) {
                    printf("+++ success +++\n");
                }
                delete pQMH;
                
            } else {
                printf("%s is not a QMap\n", apArgV[1]);
            }
        } 
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
