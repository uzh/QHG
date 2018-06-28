#include <stdio.h>
#include "types.h"
#include "StatDEM.h"




//----------------------------------------------------------------------------
// constructor
//
StatDEM::StatDEM(int iNumLonVals, int iNumLatVals) 
:   DEM(iNumLonVals, iNumLatVals),
    m_matDEM(NULL) {

    m_matDEM = new double *[m_iNumLatVals];
    for (int i1 = 0; i1 < m_iNumLatVals; i1++) {
        m_matDEM[i1] = new double[m_iNumLonVals];
    }
printf("[StatDEM]Memory allocated:#lon %d, #lat %d\n", m_iNumLonVals, m_iNumLatVals);    
}



//----------------------------------------------------------------------------
// load
//
bool StatDEM::load(char *pFileName) {
    bool bOK = false;
    FILE *fIn = fopen(pFileName, "rt");
    if (fIn != NULL) {
        char sLine[256];
        int iLat = 0;
        int iLon = 0;
printf("Loading geo data...\n");
printf("CurLat: %4d", 0);        
        while (!feof(fIn) && (iLat < m_iNumLatVals)) {
            char *p = fgets(sLine, 256, fIn);
            if (p != NULL) {
                m_matDEM[iLat][iLon] = splitLine(p);
            }
            ++iLon;
            if (iLon >= m_iNumLonVals) {
                if (iLat%10 == 0) {
                    printf("\b\b\b\b%4d", iLat);fflush(stdout);
                }
                iLon = 0;
                ++iLat;
            }
        }
printf("\b\b\b\b%4d\n", iLat);fflush(stdout);
printf("Done\n");
//        printf("[StatDEM]#Lat:  %d\n", m_iNumLatVals);
//        printf("[StatDEM]#Lon:  %d\n", m_iNumLonVals);

        m_dDeltaLat = (m_dMaxLat-m_dMinLat)/(m_iNumLatVals-1);
        m_dDeltaLon = (m_dMaxLon-m_dMinLon)/(m_iNumLonVals-1);

//        printf("[StatDEM]Lat:  [%f, %f], D:%f\n", m_dMinLat, m_dMaxLat, m_dDeltaLat);
//        printf("[StatDEM]Lon: [%f, %f], D:%f\n", m_dMinLon, m_dMaxLon, m_dDeltaLon);
        bOK = true;
    } else {
        bOK = false;
    }
    return bOK;
}

//----------------------------------------------------------------------------
// destructor
//
StatDEM::~StatDEM() {
    if (m_matDEM != NULL) {
        for (int i = 0;i < m_iNumLatVals; i++) {
            if (m_matDEM[i] != NULL) {
                delete[] m_matDEM[i];
            }
        }
        delete[] m_matDEM;
    }
}

//----------------------------------------------------------------------------
// getAltitude
//
double StatDEM::getAltitude(double dLon, double dLat) {
    double d = NO_VAL;
    int iLatIndex=-1;
    int iLonIndex=-1;
    bool bOK = findIndex(dLon, dLat, iLonIndex, iLatIndex);
    if (bOK) {
        printf("getting value for %d, %d\n", iLonIndex, iLatIndex);
        d = m_matDEM[iLatIndex][iLonIndex];
    }
    return d;
}

