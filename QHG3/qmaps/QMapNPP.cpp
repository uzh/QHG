#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    printf("%s - calc npp from land-sea-mask, rain and temperature\n", pApp);
    printf("usage:\n");
    printf("  %s <QMapMask> <QMaskRain> <QMapTemp> <QMapOut>\n", pApp);
    printf("where\n");
    printf("  QMapMask    qmap containing land-sea mask (land:1, sea:0)\n"); 
    printf("  QMapRain    qmap containing annual rainfall (mm)\n"); 
    printf("  QMapMask    qmap containing mean annual Temperature (kelvin)\n"); 
    printf("  QMapOut    name for output QMap\n");
}

//-------------------------------------------------------------------------------------------------
// calcNPP
//
int calcNPP(ValReader *pVRMask, ValReader *pVRRain, ValReader *pVRTemp, const char *pOut) {
    int iResult = -1;

    uint iH = pVRMask->getNRLat();
    uint iW = pVRMask->getNRLon();

    double **ppNPP = new double *[iH];

    double dLat = pVRMask->getLatMin();
    for (uint i = 0; i < iH; i++) {
        ppNPP[i] = new double[iW];

        double dLon = pVRMask->getLonMin();
        for (uint j = 0; j < iW; j++) {
            if (pVRMask->getDValue(j, i) > 0) {
                double dLon2 = (dLon>180)?dLon-180:dLon;
                double dR = pVRRain->getDValue(dLon2, dLat);
                double dT = pVRTemp->getDValue(dLon2, dLat);
                double dNPPT = 3000/(1 + exp(1.315 - 0.119*dT));
                double dNPPR = 3000*(1-exp(-0.000664*dR));
                //                printf("%5d %5d (%5.2f, %5.2f): %8.4f %8.4f\n", j, i, dLon, dLat, dR, dT);
                ppNPP[i][j] = cos((dLat+pVRMask->getDLat()/2)*M_PI/180.0) * ((dNPPT < dNPPR)?dNPPT:dNPPR);
                if (ppNPP[i][j] < 0) {
                    printf("%5d %5d (%5.2f, %5.2f): t %8.4f r %8.4f-> nppt %f, nppr%f, npp %f\n", j, i, dLon, dLat, dT, dR, dNPPT, dNPPT, ppNPP[i][j]);
                }
            } else {
                ppNPP[i][j] = 0.0;
            }
            dLon +=  pVRMask->getDLon();
        }
        dLat +=  pVRMask->getDLat();
    }

       // write output

    QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                       pVRMask->getLonMin(), pVRMask->getLonMax(), pVRMask->getDLon(),
                                      pVRMask->getLatMin(), pVRMask->getLatMax(), pVRMask->getDLat(),
                                      "NPP",  pVRMask->getXName(),  pVRMask->getYName());
    
    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {

        bool bOK = pQMH->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, iW, iH, ppNPP);
        }
        fclose(fOut);
        iResult = bOK?0:-1;
    } else {
        printf("Couldn't open %s\n", pOut);
    }
    
    QMapUtils::deleteArray(iW, iH, ppNPP);
    delete pQMH;

    return iResult;
};

//-------------------------------------------------------------------------------------------------
// main
//  arguments: 
//    <QMap1> <OpName> <QMap2> <QmapOut>
//  or
//    <QMap1> <OpName> <number> <QmapOut>
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sMask[LONG_INPUT];
    char sRain[LONG_INPUT];
    char sTemp[LONG_INPUT];
    char sOut[LONG_INPUT];
    *sMask = '\0';
    *sRain = '\0';
    *sTemp = '\0';
    *sOut  = '\0';

    if (iArgC > 4) {

        char *pDataDir = getenv("DATA_DIR");
        char *pFinal = NULL;
        
        searchFile(apArgV[1], pDataDir, sMask);
        searchFile(apArgV[2], pDataDir, sRain);
        searchFile(apArgV[3], pDataDir, sTemp);
        strcpy(sOut, apArgV[4]);

        if ((*sMask != '\0') && (*sRain != '\0') && (*sTemp != '\0')) {
            int iType1;
            ValReader *pVRMask = QMapUtils::createValReader(sMask, false, &iType1);
            if (pVRMask != NULL) {
                QMapHeader *pQMH = new QMapHeader(iType1,
                                                  pVRMask->getLonMin(), pVRMask->getLonMax(), pVRMask->getDLon(),
                                                  pVRMask->getLatMin(), pVRMask->getLatMax(), pVRMask->getDLat(),
                                                  pVRMask->getVName(),  pVRMask->getXName(),  pVRMask->getYName());

                int iType2;
                ValReader *pVRRain = QMapUtils::createValReader(sRain, true, &iType2);
                if (pVRRain != NULL) {
                    iResult = 0;
                    int iType3;
                    ValReader *pVRTemp = QMapUtils::createValReader(sTemp, true, &iType3);
                    if (pVRTemp != NULL) {
                        iResult = calcNPP(pVRMask, pVRRain, pVRTemp, sOut);
                        if (iResult == 0) {
                            printf("+++ success +++\n");
                        } else {
                            printf("npp claculation failed\n");
                        }
                        delete pVRTemp;
                    } else {
                        printf("Couldn't create val reader for [%s]\n", sTemp);
                    }
                    delete pVRRain;
                } else {
                    printf("Couldn't create val reader for [%s]\n", sRain);
                }
                delete pVRMask;
            } else {
                printf("Couldn't create val reader for [%s]\n", sMask);
            }
        } else {
            if (*sMask == '\0') {
                printf("Mask file [%s] does not exist\n", sMask);
            }
            if (*sRain == '\0') {
                printf("Rain file [%s] does not exist\n", sRain);
            }
            if (*sTemp == '\0') {
                printf("Temp file [%s] does not exist\n", sTemp);
            }
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
