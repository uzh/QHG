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


//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - dump the QMap data as floats into a file\n", pApp);
    printf("usage:\n");
    printf("  %s <QMap1> <mode> <outputfile>\n", pApp);
    printf("where\n");
    printf("  QMap1      : name of input QMap\n");
    printf("  mode       : 'ascii' or 'bin'\n");
    printf("  outputfile : name of outputfile\n");
    printf("\n");
}
template<class T>
int typedDump( FILE *fOut, ValReader *pVR, T undef) {
    int iResult = 0;
    int iH = pVR->getNRLat();
    int iW = pVR->getNRLon();

    printf("ascdump of %dx%d items of size %zd\n", iW, iH, sizeof(T));
    QMapReader<T> *pQMR =dynamic_cast<QMapReader<T> *> (pVR);
    T **ppData = pQMR->getData();
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            fprintf(fOut, "%f ", (double) ppData[i][j]);
        }
        fprintf(fOut, "\n");
    }
    return iResult;
}

template<class T>
int binDump( FILE *fOut, ValReader *pVR, T undef) {
    int iResult = 0;
    int iH = pVR->getNRLat();
    int iW = pVR->getNRLon();
    printf("bindump of %dx%d items of size %zd\n", iW, iH, sizeof(T));
    QMapReader<T> *pQMR =dynamic_cast<QMapReader<T> *> (pVR);
    T **ppData = pQMR->getData();
    for (int i = 0; i < iH; i++) {
        fwrite(ppData[i], sizeof(T), iW, fOut);
    }
    return iResult;
}


//-------------------------------------------------------------------------------------------------
// doDump
//
int doDump(FILE *fOut, ValReader *pVR, int iType) {
    int iResult = -1; 

    switch (iType) {
    case QMAP_TYPE_UCHAR: {
        uchar uudef = 0xff;
        iResult = typedDump(fOut, pVR, uudef);
    }
        break;
    case QMAP_TYPE_SHORT: {
        short int s = -1;
        iResult = typedDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_INT : {
        int s = -1;
        iResult = typedDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_LONG : {
        long s = -1;
        iResult = typedDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_FLOAT: {
        iResult = typedDump(fOut, pVR, fNaN);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        iResult = typedDump(fOut, pVR, dNaN);
    }
        break;
    default:
        iResult = -1;
    }

    return iResult;
}

int doBinDump(FILE *fOut, ValReader *pVR, int iType) {
    int iResult = -1; 

    switch (iType) {
    case QMAP_TYPE_UCHAR: {
        uchar uudef = 0xff;
        iResult = binDump(fOut, pVR, uudef);
    }
        break;
    case QMAP_TYPE_SHORT: {
        short int s = -1;
        iResult = binDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_INT : {
        int s = -1;
        iResult = binDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_LONG : {
        long s = -1;
        iResult = binDump(fOut, pVR, s);
    }
        break;
    case QMAP_TYPE_FLOAT: {
        iResult = binDump(fOut, pVR, fNaN);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        iResult = binDump(fOut, pVR, dNaN);
    }
        break;
    default:
        iResult = -1;
    }

    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sInput[LONG_INPUT];
    char sOutput[LONG_INPUT];
    
    if (iArgC > 3) {
        bool bBinary = false;

        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sInput);
        searchFile(apArgV[3], pDataDir, sOutput);
        if (strcasecmp(apArgV[2], "ascii")==0) {
            bBinary = false;
            iResult = 0;
        } else if (strcasecmp(apArgV[2], "bin")==0) {
            bBinary = true;
            iResult = 0;
        } 
        
        if (iResult == 0) {
            FILE *fOut = fopen(sOutput,bBinary?"wb":"wt");
            if (fOut != NULL) {
                int iType=-1;
                ValReader *pVR = QMapUtils::createValReader(sInput, false, &iType);
                if (pVR != NULL) {
                    if (bBinary) {
                        iResult = doBinDump(fOut, pVR, iType);
                    } else {
                        iResult = doDump(fOut, pVR, iType);
                    }

                    delete pVR;
                } else {
                    printf("[%s] is not a QMap\n", apArgV[1]);
                }
                fclose(fOut);
            } else {
                printf("Couldn't open [%s] for writing\n", apArgV[3]);
            }
        } else {
            printf("Invalid mode: [%s] -  should be 'ascii' or 'bin'\n", apArgV[2]);
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
