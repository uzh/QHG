#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QMapHeader.h"
#include "UDelReader.h"


void makeMap( UDelReader *pUDR, char *pName) {
    QMapHeader *pQMH3 = new QMapHeader(QMAP_TYPE_DOUBLE,  pUDR->getLonMin(),  pUDR->getLonMax(),  pUDR->getDLon(), 
                                       pUDR->getLatMin(),  pUDR->getLatMax(),  pUDR->getDLat());
    FILE *fpP = fopen(pName, "wb");
    pQMH3->addHeader(fpP);
   
    double *adBufP = new double[pQMH3->m_iWidth];
    
    for (double dLat = pUDR->getLatMin(); dLat <  pUDR->getLatMax(); dLat += pUDR->getDLat()) {
        int iCC = 0;
        for (double dLon =  pUDR->getLonMin(); dLon <  pUDR->getLonMax(); dLon += pUDR->getDLon()) {
            adBufP[iCC++] =  pUDR->getValue(dLon, dLat, 0);
        }
        printf("Row %f: %d\n", dLat, iCC);
        fwrite(adBufP, sizeof(double), iCC, fpP);
    }
    fclose(fpP);
    delete pQMH3;
}


int main(int iArgC, char *apArgV[]) {
    if (iArgC > 1) {
        UDelReader *pUDR = new UDelReader(apArgV[1], -179.75, 180, -89.75, 90);
        bool bReady = pUDR->extractColumnRange(UDEL_ANN_START, UDEL_ANN_END);
        if (bReady) {
            char sInput[128];
            *sInput = ' ';
            while (*sInput != '\n') {
                printf("enter longitude,latitude (or CR to quit, ? for help) : ");
                fgets(sInput, 128, stdin); 
                if (*sInput != '\n') {
                    if (*sInput == 'q') {
                        *sInput = '\n';
                    } else if (*sInput == '?') {
                        printf(" q, CR  - quit\n");
                        printf(" ?      - help\n");
                        printf(" m      - create mapfile\n");
                    } else if (*sInput == 'm') {
                        printf("enter filename : ");
                        fgets(sInput, 128, stdin);
                        makeMap(pUDR, sInput);
                    } else {
                        bool bInterp = false;
                        if (*sInput == 'i') {
                            bInterp = true;
                            *sInput = ' ';
                        }
                        char *p2 = strchr(sInput, ',');
                        if (p2 != NULL) {
                            *p2 = '\0';
                            p2++;
                            double dLon = atof(sInput);
                            double dLat = atof(p2);
                            double dVal;
                            if (bInterp) {
                                dVal = pUDR->getValueBiLin(dLon, dLat, 0);
                            } else {
                                dVal = pUDR->getValue(dLon, dLat, 0);
                            }
                            printf("  -> %f\n", dVal);
                        }
                    }
                }
            }

        } else {
            printf("Couldn't extract data\n");
        }
        delete pUDR;
    } else {
        printf("%s - testing a map UDEL file.\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <udelfile>\n", apArgV[0]);
        printf("where\n");
        printf("  mapfile : a UDEL file\n");
    }
    return 0; 
}
