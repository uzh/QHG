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
void usage(char *pAppName) {
    printf("%s - extract a region from a QMap\n", pAppName);
    printf("Usage:\n");
    printf("  %s <QMapIn> <Range> <TxtOut>\n", pAppName);
    printf("where\n");
    printf("  QMapIn    Name of input QMap\n"); 
    printf("  Range     Range to extract <dLonMin>:<dLonMax>:<dLatMin>:<dLatMax>\n"); 
    printf("            any of those 4 items can be given as '*' to use\n");
    printf("            the original's item\n");
    printf("  TxtOut   Name of output file f\n"); 
    printf("\n");
    printf("%s lholdag_f.qmap -15:*:20:60\n", pAppName);
    printf("extract a region from lholdag_f.qmap such that its extents are as follows\n");
    printf("  minimum Longitude -15\n");
    printf("  maximum Longitude  maximum longitude of input file\n");
    printf("  minimum Latitude   20\n");
    printf("  maximum Latitude   60\n");
}


//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char sIn[LONG_INPUT];
    char sRange[LONG_INPUT];
    char sOut[LONG_INPUT];

    if (iArgC > 3) {
        char *pDataDir = getenv("DATA_DIR");
        searchFile(apArgV[1], pDataDir, sIn);
        strcpy(sIn, apArgV[1]);

        strcpy(sRange, apArgV[2]);
        strcpy(sOut, apArgV[3]);
        
        int iType = -1;
        
        int iDataOffset  = 0;
        int iRowSize     = 0;
        int iRangeWidth  = 0;
        int iRangeHeight = 0;
        QMapHeader *pQMH = NULL;

        ValReader *pVR = QMapUtils::createValReader(sIn, sRange, true, false, &iType);
        if (pVR != NULL) {
            
            FILE *fOut = fopen(sOut, "wb");
            if (fOut != NULL) {
                for (unsigned int i = 0; i <  pVR->getNRLat(); i++) {
                    double dLat =  pVR->Y2Lat(i);
                    for (unsigned int j = 0; j <  pVR->getNRLon(); j++) {
                        double dLon = pVR-> X2Lon(j);
                        fprintf(fOut, "%f\t%f\t%f\n", dLon, dLat,  pVR->getDValue(j, i)); 
                    }
                }

                fclose(fOut);
            } else {
                printf("Couldn't open [%s] for writing\n", sOut);
            }
        } else {
            printf("Couldn't open [%s] for reading", sIn);
        }
        
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}

