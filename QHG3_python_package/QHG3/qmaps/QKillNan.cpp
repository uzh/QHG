#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"


//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - remove nan values from a QMap\n", pApp);
    printf("usage:\n");
    printf("  %s <QMap1> <outputfile>\n", pApp);
    printf("where\n");
    printf("  QMap1      : name of input QMap\n");
    //    printf("  mode       : 'ascii' or 'bin'\n");
    printf("  outputfile : name of outputfile\n");
    printf("\n");
}

template<class T>
void fix(T **ppData, int iW, int iH, int iX, int iY) {
    T dSum = 0;
    int iC = 0;
    if (iX > 1) {
        if (!isnan(ppData[iY][iX-1])) {
            dSum += ppData[iY][iX-1];
            iC++;
        }
    }
    if (iX < iW-1) {
        if (!isnan(ppData[iY][iX+1])) {
            dSum += ppData[iY][iX+1];
            iC++;
        }
    }
    if (iY > 1) {
        if (!isnan(ppData[iY-1][iX])) {
            dSum += ppData[iY-1][iX];
            iC++;
        }
    }
    if (iY < iH-1) {
        if (!isnan(ppData[iY+1][iX])) {
            dSum += ppData[iY+1][iX];
            iC++;
        }
    }
    if (iC > 0) {
        ppData[iY][iX] = dSum/iC;
    }
}

template<class T>
int removeNaNs(FILE *fOut, ValReader *pVR, T undef) {
    int iResult = 0;
    int iH = pVR->getNRLat();
    int iW = pVR->getNRLon();

    printf("ascdump of %dx%d items of size %zd\n", iW, iH, sizeof(T));
    QMapReader<T> *pQMR =dynamic_cast<QMapReader<T> *> (pVR);
    T **ppData = pQMR->getData();
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            if (isnan(ppData[i][j])) {
                fix(ppData, iW, iH, j, i);
            }
        }
    }
    QMapUtils::writeArray(fOut, iW, iH, ppData);
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// doRemoveNaNs
//
int doRemoveNaNs(FILE *fOut, ValReader *pVR, int iType) {
    int iResult = -1; 

    switch (iType) {
    case QMAP_TYPE_FLOAT: {
        iResult = removeNaNs(fOut, pVR, fNaN);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        iResult = removeNaNs(fOut, pVR, dNaN);
    }
        break;
    default:
        iResult = -1;
    }

    return iResult;
}



int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char sInput[LONG_INPUT];
    char sOutput[LONG_INPUT];
    
    if (iArgC > 2) {

        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sInput);
        searchFile(apArgV[2], pDataDir, sOutput);
        
        QMapHeader *pQMH = new QMapHeader();
        iResult = pQMH->readHeader(sInput);

        if (iResult == 0) {
            FILE *fOut = fopen(sOutput,"wb");
            if (fOut != NULL) {
                pQMH->addHeader(fOut);
                int iType=-1;
                ValReader *pVR = QMapUtils::createValReader(sInput, false, &iType);
                if (pVR != NULL) {
                    iResult = doRemoveNaNs(fOut, pVR, iType);
                    

                    delete pVR;
                } else {
                    printf("[%s] is not a QMap\n", apArgV[1]);
                }
            } else {
                printf("Couldn't open [%s] for writing\n", apArgV[3]);
            }
        } else {
            printf("COuldn't read header from [%s]\n", apArgV[1]);
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}

