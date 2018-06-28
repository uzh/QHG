#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DEM.h"


//----------------------------------------------------------------------------
// constructor
//
DEM::DEM(int iNumLonVals, int iNumLatVals)
:   m_dMinLat(9999),
    m_dMaxLat(-9999),
    m_dMinLon(9999),
    m_dMaxLon(-9999),
    m_dDeltaLat(0),
    m_dDeltaLon(0),
    m_iNumLatVals(iNumLatVals),
    m_iNumLonVals(iNumLonVals),
    m_bBadIndex(false) {

}

//----------------------------------------------------------------------------
// splitLine
//
double DEM::splitLine(char *pLine) {
    double dAlt = NO_VAL;
    char *pBuf;       
    char  *q = strtok_r(pLine, ", ", &pBuf);
    if (q != NULL) {
        double dLon = atof(q);
        if (dLon < m_dMinLon) {
            m_dMinLon = dLon;
        }
        if (dLon > m_dMaxLon) {
            m_dMaxLon = dLon;
        }

        q = strtok_r(NULL, ", ", &pBuf);
        if (q != NULL) {
            double dLat = atof(q);
            if (dLat < m_dMinLat) {
                m_dMinLat = dLat;
            }
            if (dLat > m_dMaxLat) {
                m_dMaxLat = dLat;
            }
            q = strtok_r(NULL, ", ", &pBuf);
            if (q != NULL) {

                dAlt = atof(q);
            }

        }
    }
    return dAlt;
}

//----------------------------------------------------------------------------
// findIndex
//   longitude, latitude in degrees
//
bool DEM::findIndex(double dLon, double dLat, int &iLonIndex, int &iLatIndex) {
    bool bOK = false;
//    printf("lat: min %f, max %f, delta %f, cur %f\n", m_dMinLat, m_dMaxLat, m_dDeltaLat, dLat);
    iLatIndex  = m_iNumLatVals-1-(int)(0.5+(dLat  - m_dMinLat)/m_dDeltaLat);
    iLonIndex = (int)(0.5+(dLon - m_dMinLon)/m_dDeltaLon);
    if ((iLatIndex >= 0) && (iLatIndex < m_iNumLatVals) &&
        (iLonIndex >= 0) && (iLonIndex < m_iNumLonVals)) {
//          printf("good index for lon %f, lat %f : lonidx %d, latidx %d\n", dLon, dLat, iLonIndex, iLatIndex);
          bOK = true;
    } else {
          printf("bad index for lon %f, lat %f : lonidx %d, latidx %d (maxlon %d, maxlat %d)\n", dLon, dLat, iLonIndex, iLatIndex, m_iNumLonVals, m_iNumLatVals);
          printf("              minLon %f, deltaLon %f, minLat %f, deltaLat %f\n", m_dMinLon, m_dDeltaLon, m_dMinLat, m_dDeltaLat);
          m_bBadIndex = true;
          bOK = false;
    }
    return bOK;
}
