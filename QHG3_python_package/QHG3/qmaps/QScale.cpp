#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "QMapHeader.h"
#include "ValReader.h"


void usage(char *pName) {
    printf("%s - change range and resolution of qmap\n", pName);
    printf("usage:\n");
    printf("  %s  <qmap_in> <lon_data> <latdata> <qmap_out>\n", pName);
    printf("or\n");
    printf("  %s  <qmap_in> <qmap_templ> <qmap_out>\n", pName);
    printf("where\n");
    printf("  qmap_in      name of input qmap-file\n");
    printf("  lon_data     <lon_min>,<lon_max>,<d_lon>\n");
    printf("  lat_data     <lat_min>,<lat_max>,<d_lat>\n");
    printf("  qmap_templ   name of qmap-file with desired size\n");
    printf("  qmap_out     name of output qmap-file\n");
    printf("\n");
    printf("Creates a qmap from a portion of original qmap with given resolutions\n");
    printf("If new range exceeds original range, 'new' cells are filled with default values:\n");
    printf(" 0xff for uchar, -1 for integer types, and NaN for float types\n");
    printf("\n");
}



int splitData(char *pCoordData, double *pdMin, double *pdMax, double *pdD) {
    int iResult = -1;
    char *pCtx;
    char *p = strtok_r(pCoordData, ",:", &pCtx);
    if (p != NULL) {
        char *pEnd;
        *pdMin = strtod(p, &pEnd);
        if (*pEnd == '\0') {
            p = strtok_r(NULL, ",:", &pCtx);
            if (p != NULL) {
                char *pEnd;
                *pdMax = strtod(p, &pEnd);
                if (*pEnd == '\0') {
                    p = strtok_r(NULL, ",:", &pCtx);
                    if (p != NULL) {
                        char *pEnd;
                        *pdD = strtod(p, &pEnd);
                        if (*pEnd == '\0') {
                            iResult = 0;
                        } else {
                            printf("Bad Format for D [%s]\n", p);
                        }
                    } else {
                        printf("Bad Format [%s]\n", p);
                    }
                } else {
                    printf("Bad Format for max [%s]\n", p);
                }
            } else {
                printf("Bad Format [%s]\n", p);
            }
        } else {
            printf("Bad Format for min [%s]\n", p);
        }
    } else {
        printf("Bad Format [%s]\n", pCoordData);
    }
    return iResult;
}


template <class T>
bool doScale(ValReader *pVR, QMapHeader *pQMHOut, char *pOut, T defVal) {
    int iResult = -1;
    // create array
    T **aadData = QMapUtils::createArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, defVal);
    // fill it
    for (double dLat = pQMHOut->m_dDataLatMin; dLat < pQMHOut->m_dDataLatMax; dLat += pQMHOut->m_dDLat) {
        int i = (int)round(pQMHOut->Lat2Y(dLat));
        printf("row %4d ...\r", i);
        for (double dLon = pQMHOut->m_dDataLonMin; dLon < pQMHOut->m_dDataLonMax; dLon += pQMHOut->m_dDLon) {
            int j = (int)round(pQMHOut->Lon2X(dLon));
            double dVal = pVR->getDValue(dLon, dLat);
            aadData[i][j] = (T) dVal;
        }
    }
              
    // save the array
    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {
        bool bOK = pQMHOut->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, pQMHOut->m_iWidth, pQMHOut->m_iHeight, aadData);
            if (bOK) {
                fclose(fOut);
                QMapUtils::deleteArray(pQMHOut->m_iWidth, pQMHOut->m_iHeight, aadData);
                printf("Success!\n");
                iResult = 0;
            } else {
                printf("Couldn't write data to output file [%s]\n", pOut);
                iResult = -4;
            }
            
        } else {
            printf("Couldn't write header to output file [%s]\n", pOut);
            iResult = -3;
        }
        
    } else {
        printf("Couldn't open output file [%s]\n", pOut);
        iResult = -2;
    }
    return iResult;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 3) {
        char sIn[LONG_INPUT];
        char sOut[LONG_INPUT];
        char sLonData[SHORT_INPUT];
        char sLatData[SHORT_INPUT];
        char sQMAPTemplate[LONG_INPUT];
        double dLonMin;
        double dLonMax;
        double dDLon;
        double dLatMin;
        double dLatMax;
        double dDLat;

        char *pDataDir = getenv("DATA_DIR");
        if (iArgC > 4) {

            searchFile(apArgV[1], pDataDir, sIn);
            
            strcpy(sLonData, apArgV[2]);
            strcpy(sLatData, apArgV[3]);
            strcpy(sOut,     apArgV[4]);

            iResult = splitData(sLonData, &dLonMin, &dLonMax, &dDLon);
            if (iResult == 0) {
                iResult = splitData(sLatData, &dLatMin, &dLatMax, &dDLat);
                if (iResult == 0) {
                } else {
                    printf("Couldn't split lat data [%s]\n", apArgV[3]);
                }
            } else {
                printf("Couldn't split lon data [%s]\n", apArgV[2]);
            }
        } else {
            searchFile(apArgV[1], pDataDir, sIn);
            searchFile(apArgV[2], pDataDir, sQMAPTemplate);
            
            strcpy(sOut,     apArgV[3]);
            
            QMapHeader *pQMHTempl = new QMapHeader();
            iResult = pQMHTempl->readHeader(sQMAPTemplate);
            if (iResult == 0) {
                dLonMin = pQMHTempl->m_dDataLonMin;
                dLonMax = pQMHTempl->m_dDataLonMax;
                dDLon   = pQMHTempl->m_dDLon;
                dLatMin = pQMHTempl->m_dDataLatMin;
                dLatMax = pQMHTempl->m_dDataLatMax;
                dDLat   = pQMHTempl->m_dDLat;

            } else {
                printf("Couldn't read header of teplate [%s]\n", sQMAPTemplate);
            }
            delete pQMHTempl;
        }

        
        if (iResult == 0) {
            int iType = QMapHeader::getQMapType(sIn);
            if (QMAP_TYPE_OK(iType)) {
                QMapHeader *pQMHOut = new QMapHeader(iType,
                                                     dLonMin, dLonMax+dDLon/2, dDLon,
                                                     dLatMin, dLatMax+dDLat/2, dDLat);
                printf("Output:\n");
                printf("  Lon [%f,%f]:%f (%d)\n", dLonMin, dLonMax, dDLon, pQMHOut->m_iWidth); 
                printf("  Lat [%f,%f]:%f (%d)\n", dLatMin, dLatMax, dDLat, pQMHOut->m_iHeight); 
                ValReader *pVR = QMapUtils::createValReader(sIn, true);
                if (pVR != NULL) {
                    switch (iType) {
                    case QMAP_TYPE_UCHAR: 
                        iResult = doScale(pVR, pQMHOut, sOut, (uchar) 0xff);
                        break;
                    case QMAP_TYPE_SHORT: 
                        iResult = doScale(pVR, pQMHOut, sOut, (short int) -1);
                        break;
                    case QMAP_TYPE_INT : 
                        iResult = doScale(pVR, pQMHOut, sOut, (int) -1);
                        break;
                    case QMAP_TYPE_LONG : 
                        iResult = doScale(pVR, pQMHOut, sOut, (long) -1);
                        break;
                    case QMAP_TYPE_FLOAT: 
                        iResult = doScale(pVR, pQMHOut, sOut, fNaN);
                        break;
                    case QMAP_TYPE_DOUBLE: 
                        iResult = doScale(pVR, pQMHOut, sOut, dNaN);
                        break;
                    default:
                        iResult = -1;
                    }
                    
                    if (iResult == 0) {
                        printf("+++ success +++\n");
                    }
                } else {
                    printf("Couldn't read input file [%s]\n", apArgV[1]);
                }
                delete pVR;
                delete pQMHOut;
            } else {
                printf("Input file is not a qmap [%s]\n", apArgV[1]);
            }
        } // error already reported
 
    } else {
        usage(apArgV[0]);
    }
    return iResult;

}







