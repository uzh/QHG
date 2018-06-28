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


const double DEF_RATIO = 0.25;
const double DEF_TEMP  = 15.0;
const double K         = (log(2)/10);


//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char * pApp) {
    printf("%s - calculate growth qmap for given capacity, temp-map and growth data\n", pApp);
    printf("usage:\n");
    printf("  %s <cap> <temp> <output> <standardTime> [<standardRatio> <standardTemp>]\n", pApp);
    printf("where:\n");
    printf("  cap           a QMap of capacities\n");
    printf("  temp          a QMap of temperatures\n");
    printf("  output        name of output qmap\n");
    printf("  standardTime  time needed for species to increase by \n");
    printf("                standardRatio*cap at temperature standardTemp\n");
    printf("  standardRatio standard growth ratio (%f)\n", DEF_RATIO);
    printf("  standardTemp  standard growth temp (%f)\n", DEF_TEMP);
    printf("\n");
}



//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    
    double dStdR = DEF_RATIO;
    double dStdTemp = DEF_TEMP;
    

    if ((iArgC == 5) || (iArgC == 7)) {
        int iType = QMAP_TYPE_NONE;
        ValReader *pVRCap = QMapUtils::createValReader(apArgV[1], true, &iType);
        if (pVRCap != NULL) {
            ValReader *pVRTemp = QMapUtils::createValReader(apArgV[2], true, &iType);
            if (pVRTemp != NULL) {
               
                char *pEnd;
                double dStdTime = strtod(apArgV[4], &pEnd);
                if (*pEnd == '\0') {
                    if (iArgC == 5) {
                        iResult = 0;
                    } else if (iArgC == 7) {
                        dStdR = strtod(apArgV[5], &pEnd);
                        if (*pEnd == '\0') {
                            dStdTemp = strtod(apArgV[6], &pEnd);
                            if (*pEnd == '\0') {
                                iResult = 0;
                            } else {
                                printf("invalid standard temp: [%s]\n", apArgV[6]);
                            }
                        } else {
                            printf("invalid standard ratio: [%s]\n", apArgV[5]);
                        }
                    } 
                } else {
                    printf("invalid standard time: [%s]\n", apArgV[4]);
                }

                if (iResult == 0) {
                    printf("Creating output [%s]\n", apArgV[3]);
                    double dFactor = dStdR/(dStdTime*(exp(dStdTemp*K)-1));
                    unsigned int iW = pVRCap->getNRLon();
                    unsigned int iH = pVRCap->getNRLat();
                    unsigned int iWT = pVRTemp->getNRLon();
                    unsigned int iHT = pVRTemp->getNRLat();
                    

                    if ((iW == iWT) && (iH == iHT)) {
                        double **adData = QMapUtils::createArray(iW, iH, dNaN);
                        double **adDataA = QMapUtils::createArray(iW, iH, dNaN);

                        double dDeltaLonRad    = pVRCap->getDLon()*M_PI/180; printf("DeltaLon: %f\n", dDeltaLonRad);
                        double dDeltaLatRad2   = pVRCap->getDLat()*M_PI/360; // half of delta lat;
                        for (unsigned int i = 0; i < iH; i++) {
                            double dLat = pVRCap->Y2Lat(i);
                            double dLatRad = dLat*M_PI/180;
                            double dPhi1 = dLatRad - dDeltaLatRad2;
                            double dPhi2 = dLatRad + dDeltaLatRad2;
                            double dDeltaSinLat = fabs(sin(dPhi1) - sin(dPhi2));

                            for (unsigned int j = 0; j < iW; j++) {
                                double dCap = pVRCap->getDValue(j, i);
                                double dTemp = pVRTemp->getDValue(j, i);
                                if (dTemp < 0) {
                                    dTemp = 0;
                                }
                                if (isfinite(dCap) && isfinite(dTemp)) {
                                    adData[i][j] = dFactor*dCap*(exp(K*dTemp)-1);
                                    adDataA[i][j] = dDeltaSinLat*dDeltaLonRad*RADIUS_EARTH2;
                                } else {
                                    adData[i][j] = dNaN;
                                    adDataA[i][j] = dNaN;
                                }

                            }
                        }
                        
                        bool bOK = true;
                        QMapHeader *pQMH = new QMapHeader();
                        iResult = pQMH->readHeader(apArgV[1]);
                        
                        strcpy(pQMH->m_sVName, "Growth");
                        
                        FILE *fOut = fopen(apArgV[3], "wb");
                        bOK = pQMH->addHeader(fOut);
                        if (bOK) {
                            bOK = QMapUtils::writeArray(fOut, iW, iH, adData);
                            iResult = bOK?0:-3;
                        }
                        fclose(fOut);    
                        
                        strcpy(pQMH->m_sVName, "Area");
                        
                        char *p = strrchr(apArgV[1], '/');
                        if (p != NULL) {
                            p++;
                            *p = '\0';
                        }
                        char sAreaName[MAX_PATH];
                        sprintf(sAreaName, "%sarea.qmap", (p!= NULL)?apArgV[1]:""); 
                        FILE *fOutA = fopen(sAreaName, "wb");
                        bOK = pQMH->addHeader(fOutA);
                        if (bOK) {
                            bOK = QMapUtils::writeArray(fOutA, iW, iH, adDataA);
                            iResult = bOK?0:-3;
                        }
                        fclose(fOutA);    

                        QMapUtils::deleteArray(iW, iH, adData);
                        QMapUtils::deleteArray(iW, iH, adDataA);
                        printf("+++success+++\n");
                    } else {
                        printf("sizes mismatch: cap: %dx%d, Temp %dx%d\n", iW, iH, iWT, iHT);
                        iResult = -2;
                    }
                }
                
                delete pVRTemp;
            } else {
                printf("couldn't create val reader for temperatures [%s]\n", apArgV[2]);
                if (iType == QMAP_TYPE_NULL) {
                    printf("  File didn't exxist\n");
                } else {
                    printf("  Probably not a valid qmap\n");
                }
            }
            delete pVRCap;
        } else {
            printf("couldn't create val reader for capacities [%s]\n", apArgV[1]);
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
