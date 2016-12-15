#include <stdio.h>
#include <string.h>

#include "LineReader.h"
#include "strutils.h"
#include "QMapHeader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"


#define KEY_LON "longitude = "
#define KEY_LAT "latitude = "


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - convert an nc dump for a variable to a QMap\n",pApp);
    printf("Usage:\n");
    printf("  %s <nc-dump> [-f] <lon-name> <lat-name> <var-name> <out-file>\n", pApp);
    printf("whereæ\n");
    printf("  nc-dump   output of ncdump for a particular variable\n");
    printf("  var-name  NetCDF variable for which the dump has been made\n");
    printf("  out-file  output QMap fgile\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// collectLonLat
//   find number of lon and lat values and create array
//
double **collectLonLat(LineReader *pLR, char*pLonKey, char *pLatKey, int *piLonMax,  int *piLatMax) {
    int iResult = 0;
    double **ppData = NULL;
    char *p0 = pLR->getCurLine();
    
    
    // collect lon and lat extents
    bool bLonSearch = true;
    bool bLatSearch = true;

    while ((p0 != NULL) && (bLonSearch || bLatSearch) && (iResult == 0)) {
        char *q = strrchr(p0, ';');
        if (q != 0) {
            *q=' ';
        }
        p0 = trim(p0);
      
        if (bLonSearch && (strstr(p0, pLonKey) == p0)) {
            char *p = p0 + strlen(pLonKey);
            if (strToNum(p, piLonMax)) {
                bLonSearch = false;
            } else {
                printf("Bad value for [%s]: [%s]\n", pLonKey, p);
                iResult = -1;
            }
        } else if (bLatSearch && (strstr(p0, pLatKey) == p0)) {
            char *p = p0 + strlen(pLatKey);
            if (strToNum(p, piLatMax)) {
                bLatSearch = false;
            } else {
                printf("Bad value for [%s]: [%s]\n", pLatKey, p);
                iResult = -1;
            }
        }
        p0 = pLR->getNextLine();
    }

    if (!bLonSearch && !bLatSearch) {
        // allocate array

        ppData = QMapUtils::createArray(*piLonMax, *piLatMax, 0.0);
    }

    return ppData;
}

//----------------------------------------------------------------------------
// searchDataStart
//
int searchDataStart(LineReader *pLR, char *pVarKey) {
    int iResult = 0;
    char *p0 = pLR->getCurLine();
    bool bSearchStart = true;
    while (bSearchStart && (iResult == 0)) {
        if (p0 != NULL) {
            if (strstr(p0, pVarKey) == p0) {
                bSearchStart = false;
            }
            p0 = pLR->getNextLine();
        } else {
            iResult = -1;
            printf("No entry [%s] found\n", pVarKey);
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// readData
//
int readData(LineReader *pLR, int iLonMax, int iLatMax, double **ppData, bool bFlip) {
    int iResult = 0;
    char *p0 = pLR->getCurLine();
    // read data
    int iLon=0;
    int iLat0=0;
    int iLat = bFlip?(iLatMax - 1):iLat0;
    
    bool bGoOn = true;

    while (bGoOn && (p0 != NULL) && (iResult == 0)) {
        if (strchr(p0, ';') != NULL) {
            bGoOn = false;
        }

        char *p = strtok(p0, ",; \t\n");
        while ((iResult == 0) && (p != NULL)) {
            if (iLat0 < iLatMax) {
                if (strToNum(p, &(ppData[iLat][iLon]))) {
                    iLon++;
                    if (iLon >= iLonMax) {
                        iLon = 0;
                        iLat0++;
                        iLat = bFlip?(iLatMax - iLat0 - 1):iLat0;
                    }
                }
                p = strtok(NULL, ",; \t\n");
            } else {
                printf("Too much data: [%s]\n", p);
                iResult = -1;
            }
        }
        
        p0 = pLR->getNextLine();
    }
    if (p0 == NULL) {
        printf("end of line before all data read\n");
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// writeData
//
int writeData(const char *pName, int iLonMax, int iLatMax, double **ppData) {
    int iResult = -1;
    FILE *fOut = fopen(pName, "wb");
    if (fOut != NULL) {
        iResult = 0;
        QMapHeader *pQMH =  new QMapHeader(QMAP_TYPE_DOUBLE,
                                           0, iLonMax, 1.0,
                                           0, iLatMax, 1.0);
        pQMH->addHeader(fOut);
        
        bool bOK = QMapUtils::writeArray(fOut, iLonMax, iLatMax, ppData);
        
        fclose(fOut);
        delete pQMH;
        if (bOK) {
            iResult = 0;
        }
    } else {
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 5) {
        bool bFlip = false;
        int iOffs = 0;
        if (strcmp(apArgV[1], "-f") == 0) {
            bFlip = true;
            iOffs = 1;
        }
        
        LineReader *pLR = LineReader_std::createInstance(apArgV[iOffs+1], "rt");
        if (pLR != NULL) {
            char sLonKey[256];
            char sLatKey[256];
            char sVarKey[256];
            sprintf(sLonKey, "%s =", apArgV[iOffs+2]);
            sprintf(sLatKey, "%s =", apArgV[iOffs+3]);
            sprintf(sVarKey, "%s =", apArgV[iOffs+4]);

            iResult = 0;

            int iLonMax=0;
            int iLatMax=0;

            // collect array sizes, create array
            double **ppData = collectLonLat(pLR, sLonKey, sLatKey, &iLonMax, &iLatMax);
            if (ppData != NULL) {
                printf("Found #Lon:%d, #Lat:%d\n", iLonMax, iLatMax);
            } else {
                iResult = -1;
                printf("Failed to find [%s], [%s] in [%s]\n", sLonKey, sLatKey, apArgV[1]);
            }

            // go to beginning of data
            if (iResult == 0) {
                iResult = searchDataStart(pLR, sVarKey);
                if (iResult == 0) {
                    printf("Found start of data: [%s]\n", pLR->getCurLine());
                } else {
                    printf("Didn't find start of data: [%s]\n", sVarKey);
                }
            }

            //read data
            if (iResult == 0) {
                iResult = readData(pLR, iLonMax, iLatMax, ppData, bFlip);
                if (iResult == 0) {
                    printf("successfully read data\n");
                }
            }

            // write data
            if (iResult == 0) {
                iResult = writeData(apArgV[iOffs+5], iLonMax, iLatMax, ppData);
                if(iResult == 0) {
                    printf("wrote data to [%s]\n", apArgV[iOffs+5]);
                    printf("+++ success +++\n");
                }
            }



            if (ppData != NULL) {
                QMapUtils::deleteArray(iLonMax, iLatMax,ppData);
            }



            delete pLR;
        } else {
            printf("Couldn't open [%s] for reading\n", apArgV[1]);
        }
    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
