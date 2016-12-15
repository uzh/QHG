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
void usage(char *pAppName) {
    printf("%s - prune or extend a QMap\n", pAppName);
    printf("Usage:\n");
    printf("  %s <QMapIn> <QMapData> [<QMapOut> [<defval>]]\n", pAppName);
    printf("where\n");
    printf("  QMapIn    Name of input QMap\n"); 
    printf("  QMapData  Data: <dLonMin>:<dLonMax>:<dLatMin>:<dLatMax>[:<undefval>]\n"); 
    printf("            any of those 4 items can be given as '*' to use\n");
    printf("            the original's item\n");
    printf("  QMapOut   Name of output QMap. If omitted, QMapIn is used\n"); 
    printf("\n");
    printf("%s lholdag_f.qmap -15:*:20:60\n", pAppName);
    printf("prune and extend lholdag_f.qmap such that its extents are as follows\n");
    printf("  minimum Longitude -15\n");
    printf("  maximum Longitude  maximum longitude of input file\n");
    printf("  minimum Latitude   20\n");
    printf("  maximum Latitude   60\n");
}

//-------------------------------------------------------------------------------------------------
// splitData
//   split data of the form "lonMin,lonMax,latMin,latMax"
//
int splitData(char *pData, 
              double *pdLonMin, double *pdLonMax,
              double *pdLatMin, double *pdLatMax,
              double *pdUndef) {
    int iResult = -1;
    char sNul[1];
    *sNul = '\0';
    char *pCtx;
    char *p = strtok_r(pData, ",:", &pCtx);
    if (p != NULL) {
        char *pEnd;
        if (*p=='*') {
            pEnd = sNul;
        } else {
            *pdLonMin = strtod(p, &pEnd);
        }
        if (*pEnd == '\0') {
            p = strtok_r(NULL, ",:", &pCtx);
            if (p != NULL) {
                if (*p=='*') {
                    pEnd = sNul;
                } else {
                    *pdLonMax = strtod(p, &pEnd);
                }
                if (*pEnd == '\0') {        
                    p = strtok_r(NULL, ",:", &pCtx);
                    if (p != NULL) {
                        if (*p=='*') {
                            pEnd = sNul;
                        } else {
                            *pdLatMin = strtod(p, &pEnd);
                        }
                        if (*pEnd == '\0') {        
                            p = strtok_r(NULL, ",:", &pCtx);
                            if (p != NULL) {
                                if (*p=='*') {
                                    pEnd = sNul;
                                } else {
                                    *pdLatMax = strtod(p, &pEnd);
                                }
                                if (*pEnd == '\0') {
                                    iResult = 0;
                                    p = strtok_r(NULL, ",:", &pCtx);
                                    if (p != NULL) {
                                        if (*p=='*') {
                                            pEnd = sNul;
                                        } else {
                                            *pdUndef = strtod(p, &pEnd);
                                        } 
                                        if (*pEnd == '\0') {
                                            iResult = 0;
                                        } else {
                                            iResult =-1;
                                            printf("Bad Number format: [%s]\n", p);
                                        }
                                        
                                    }
                                } else {
                                    printf("Bad Number format: [%s]\n", p);
                                }
                            } else {
                                printf("Bad Data Format: expected LatMax\n");
                            }
                        } else {
                            printf("Bad Number format: [%s]\n", p);
                        }
                    } else {
                        printf("Bad Data Format: expected LatMin\n");
                    }
                } else {
                    printf("Bad Number format: [%s]\n", p);
                }
            } else {
                printf("Bad Data Format: expected LonMax\n");
            }
        } else {
            printf("Bad Number format: [%s]\n", p);
        }
    } else {
        printf("Bad Data Format: expected LonMin\n");
    }
        
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// calcExtents
//   calc extents and actual parameters for new QMapfile
//
QMapHeader *calcExtents(ValReader *pVR, int iType,
                        double dLonMinNew, double dLonMaxNew,
                        double dLatMinNew, double dLatMaxNew,
                        int *piWNew, int *piHNew,
                        int *piOffsX, int *piOffsY) {
    /*
    if (dLonMinNew > pVR->getLonMin()) {
        dLonMinNew = pVR->getLonMin();
    }
    if (dLonMaxNew < pVR->getLonMax()) {
        dLonMaxNew = pVR->getLonMax();
    }
    if (dLatMinNew > pVR->getLatMin()) {
        dLatMinNew = pVR->getLatMin();
    }
    if (dLatMaxNew < pVR->getLatMax()) {
        dLatMaxNew = pVR->getLatMax();
    }
    
    printf("New Lon[%f,%f]%f, Lat[%f,%f]%f\n", dLonMinNew, dLonMaxNew, pVR->getDLon(), dLatMinNew, dLatMaxNew, pVR->getDLat());
    */

    // calculate phase-aligned extents of new array
    dLonMinNew = pVR->getNextGridPointLon(dLonMinNew);
    dLonMaxNew = pVR->getNextGridPointLon(dLonMaxNew);

    dLatMinNew = pVR->getNextGridPointLat(dLatMinNew);
    dLatMaxNew = pVR->getNextGridPointLat(dLatMaxNew);

    // coord values of new limits in old map are the negative offsets
    *piOffsX = -(int)(pVR->Lon2X(dLonMinNew));
    *piOffsY = -(int)(pVR->Lat2Y(dLatMinNew));

    double dMaxRLon  = dLonMaxNew - pVR->getDLon();
    double dMaxRLat  = dLatMaxNew - pVR->getDLat();

    // calculate array size
    *piWNew = (int)floor(round(1+(dMaxRLon-dLonMinNew)/pVR->getDLon()));
    *piHNew = (int)floor(round(1+(dMaxRLat-dLatMinNew)/pVR->getDLat()));

    // create & return QMapHeader for new map
    return new QMapHeader(iType,
                          dLonMinNew, dLonMaxNew, pVR->getDLon(),
                          dLatMinNew, dLatMaxNew, pVR->getDLat(),
                          pVR->getVName(), pVR->getXName(), pVR->getYName());
}


//-------------------------------------------------------------------------------------------------
// embed
//   embed data and write QMAP
//
template <class T>
int embed(char *pOut, T **atData, QMapHeader *pQMH, 
          int iWNew, int iHNew, 
          int iW, int iH, 
          int iOffsX, int iOffsY, const T &defval) {

    int iResult = -1;
    T **atDataNew = QMapUtils::createArray(iWNew, iHNew, defval);
    /*
    // create array & fill with default value
    T **atDataNew = new T*[iHNew];
    for (int i = 0; i < iHNew; i++) {
        atDataNew[i] = new T[iWNew];
        for (int j = 0; j < iWNew; j++) {
            atDataNew[i][j] = defval;
        }
    }
    */
    // determine offsets into new array
    int iNOffsX = 0;
    if (iOffsX < 0) {
        iNOffsX =-iOffsX;
        iOffsX = 0;
    } 
    int iNOffsY = 0;
    if (iOffsY < 0) {
        iNOffsY =-iOffsY;
        iOffsY = 0;
    } 

    // determine maximum y- and x- values 
    int iXMax =  iW-iNOffsX;
    if (iXMax > iWNew-iOffsX) {
        iXMax = iWNew-iOffsX;
    }
    int iYMax =  iH-iNOffsY;
    if (iYMax > iHNew-iOffsY) {
        iYMax = iHNew-iOffsY;
    }

    // copy
    for (int i = 0; i < iYMax; i++) {
        for (int j = 0; j < iXMax; j++) {
            atDataNew[i+iOffsY][j+iOffsX] = atData[i+iNOffsY][j+iNOffsX];
        }
    }


    // add QMap header and write to file
    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {
        pQMH->addHeader(fOut);
        bool bOK = QMapUtils::writeArray(fOut, iWNew, iHNew, atDataNew);
              
        fclose(fOut);
        iResult = bOK?0:-1;
    } else {
        printf("Couldn't open %s\n", pOut);
    }
    
    QMapUtils::deleteArray(iWNew, iHNew, atDataNew);
    return iResult;;
}

//-------------------------------------------------------------------------------------------------
// emedding
//   create QMH and do differently typed calls to embed()
//
int embedding(char *pOut, ValReader *pVR, int iType,
              double dLonMinNew, double dLonMaxNew,
              double dLatMinNew, double dLatMaxNew,
              double dUndef) {
    int iResult = -1;
    int iOffsX = 0;
    int iOffsY = 0;
    int iWNew = 0;
    int iHNew = 0;
    int iW = pVR->getNRLon();
    int iH = pVR->getNRLat();
    QMapHeader *pQMH = calcExtents(pVR, iType, dLonMinNew, dLonMaxNew, dLatMinNew, dLatMaxNew, 
                             &iWNew, &iHNew, &iOffsX, &iOffsY);

    printf("New QMH:\n");
    pQMH->display();
    switch (iType) {
    case QMAP_TYPE_UCHAR: {
        QMapReader<uchar> *pQMR =dynamic_cast<QMapReader<uchar> *> (pVR);
        uchar **ppData = pQMR->getData();
        uchar uudef = (isnan(dUndef))?0xff:(uchar) dUndef;
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, uudef);
    }
        break;
    case QMAP_TYPE_SHORT: {
        QMapReader<short int> *pQMR =dynamic_cast<QMapReader<short int> *> (pVR);

        short int **ppData = pQMR->getData();
        short int sudef = (isnan(dUndef))?-1:(short int) dUndef;
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, sudef);
    }
        break;
    case QMAP_TYPE_INT : {
        QMapReader<int> *pQMR =dynamic_cast<QMapReader<int> *> (pVR);      
        int **ppData = pQMR->getData();
        int iudef = (isnan(dUndef))?-1:(int) dUndef;
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, iudef);
    }
        break;
    case QMAP_TYPE_LONG : {
        QMapReader<long> *pQMR =dynamic_cast<QMapReader<long> *> (pVR);      
        long **ppData = pQMR->getData();
        long ludef = (isnan(dUndef))?-1:(long) dUndef;
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, ludef);
    }
        break;
    case QMAP_TYPE_FLOAT: {
        QMapReader<float> *pQMR =dynamic_cast<QMapReader<float> *> (pVR);
        float **ppData = pQMR->getData();
        float fudef = (isnan(dUndef))?fNaN:(float) dUndef;
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, fudef);
    }
        break;
    case QMAP_TYPE_DOUBLE: {
        QMapReader<double> *pQMR =dynamic_cast<QMapReader<double> *> (pVR);
        double **ppData = pQMR->getData();
        iResult = embed(pOut, ppData, pQMH, iWNew, iHNew, iW, iH, iOffsX, iOffsY, dUndef);
    }
        break;
    default:
        iResult = -1;
    }

    delete pVR;
    delete pQMH;

    return iResult;
}
    
//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char sIn[LONG_INPUT];
    char sOut[LONG_INPUT];
    double dDefVal = dNaN;

    if (iArgC > 2) {
        if (iArgC > 3) {
            strcpy(sOut, apArgV[3]);
            if (iArgC > 4) {
                char *pEnd;
                double dD = strtod(apArgV[4], &pEnd);
                if (*pEnd == '\0') {
                    dDefVal = dD;
                }
            }
        } else {
            strcpy(sOut, apArgV[1]);
        }
        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sIn);
        strcpy(sIn, apArgV[1]);
        
        int iType = -1;
        ValReader *pVR = QMapUtils::createValReader(sIn, false, &iType);
        if (pVR != NULL) {
            double dLonMin=pVR->getLonMin();
            double dLonMax=pVR->getLonMax();
            double dLatMin=pVR->getLatMin();
            double dLatMax=pVR->getLatMax();
            double dUndef = dDefVal;

            printf("Type : [%s](%d)\n", QMapHeader::getTypeName(iType), iType);
            printf("Size : %dx%d\n", pVR->getNRLon(), pVR->getNRLat());
            printf("Lon  : [%-4.4f, %-4.4f] D:%-4.4f\n", dLonMin, dLonMax, pVR->getDLon());
            printf("Lat  : [%-4.4f, %-4.4f] D:%-4.4f\n", dLatMin, dLatMax, pVR->getDLat());
            
            iResult = splitData(apArgV[2], &dLonMin, &dLonMax, &dLatMin, &dLatMax, &dUndef);
            if (iResult == 0) {

                iResult = embedding(sOut, pVR, iType, dLonMin, dLonMax, dLatMin, dLatMax, dUndef);
                if (iResult == 0) {
                    printf("+++ success +++\n");
                }
             }
        } else {
            printf("%s is not a QMapFile\n", sIn);
        }
        
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}

