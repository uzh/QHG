#include "ranges.h"
#include "QMapHeader.h"
#include "UDelReader.h"



//------------------------------------------------------------------------------
// constructor
//
UDelReader::UDelReader(char *pFileName, 
                       float fRangeLonMin, float fRangeLonMax,
                       float fRangeLatMin, float fRangeLatMax,
                       int iNumVals)
    : TabDataReader(pFileName, 
                    DEF_MAP_LON_MIN, DEF_MAP_DLON, fRangeLonMin, fRangeLonMax,
                    DEF_MAP_LAT_MIN, DEF_MAP_DLAT, fRangeLatMin, fRangeLatMax,
                    iNumVals) {
}

//------------------------------------------------------------------------------
// destructor
//
UDelReader::~UDelReader() {
}

//------------------------------------------------------------------------------
// extractLocation
//   get Longitude and latitude for files where they are 
//   the first two entries 
//
bool UDelReader::extractLocation(char *pLine, float *pfLon, float *pfLat) {
    bool bOK = false;
    char sLine[MAX_LINE];
    strcpy(sLine, pLine);
    char *pp;
    char *p = strtok_r(pLine, " \t\n", &pp);
    if (p != NULL) {
        bOK = false;
        char *pEnd;
        // longitude
        *pfLon = strtof(p, &pEnd);
        if (*pEnd=='\0') {
            p = strtok_r(NULL, " \t\n", &pp);
            if (p != NULL) {
                // latitude
                *pfLat = strtof(p, &pEnd);
                if (*pEnd=='\0') {
                    bOK = true;
                }
            }
        }
    }
    return bOK;
}


int UDelReader::convert2QMap(char *pUDelFile, char *pQMapFile, int iColumnFrom, int iColumnTo) {
    int iResult = -1;

    UDelReader *pUDR = new UDelReader(pUDelFile, 
                                      DEF_MAP_LON_MIN, DEF_MAP_LON_MAX,
                                      DEF_MAP_LAT_MIN, DEF_MAP_LAT_MAX);
    bool bReady = pUDR->extractColumnRange(iColumnFrom, iColumnTo);
    if (bReady) {


        QMapHeader *pQMH3 = new QMapHeader(QMAP_TYPE_FLOAT,  
                                           pUDR->getLonMin(),  pUDR->getLonMax(),  pUDR->getDLon(), 
                                           pUDR->getLatMin(),  pUDR->getLatMax(),  pUDR->getDLat());
        FILE *fpP = fopen(pQMapFile, "wb");
        pQMH3->addHeader(fpP);
  
        // line buffer
        float *adBufP = new float[pQMH3->m_iWidth];
    
        // loop through UDR data
        for (double dLat = pUDR->getLatMin(); dLat <  pUDR->getLatMax(); dLat += pUDR->getDLat()) {
            int iCC = 0;
            for (double dLon =  pUDR->getLonMin(); dLon <  pUDR->getLonMax(); dLon += pUDR->getDLon()) {
                float dV =  pUDR->getValue(dLon, dLat, 0);
                if (!isfinite(dV)) {
                    dV = UDEL_BAD_VAL;
                }
                adBufP[iCC++] =  dV;
            }
            printf("\rRow %12.4f: %d", dLat, iCC);
            iResult = fwrite(adBufP, sizeof(float), iCC, fpP);
        }
        iResult = 0;
        printf("\n");
        fclose(fpP);
        delete pQMH3;
    }
    delete pUDR;
    return iResult;
}
