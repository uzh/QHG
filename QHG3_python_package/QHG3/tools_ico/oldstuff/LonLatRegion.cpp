#include <stdio.h>

#include "utils.h"
#include "strutils.h"
#include "MessLogger.h"

#include "Region.h"
#include "IcoNode.h"

#include "LonLatRegion.h"

#define EPS 1e-16
//----------------------------------------------------------------------------
//  constructor
//   note:  expect radian values
//
LonLatRegion::LonLatRegion(int iID, double dLonMin, double dLonMax, double dLatMin, double dLatMax)
    : Region(iID),
      m_dLonMin(dLonMin),
      m_dLonMax(dLonMax),
      m_dLatMin(dLatMin),
      m_dLatMax(dLatMax) {
}

//----------------------------------------------------------------------------
//  contains
//
bool LonLatRegion::contains(IcoNode *pBC) {
    IcoNode * pIN = dynamic_cast<IcoNode *>(pBC);
    //    LOG_STATUS("Checking if %f,%f is in region %d (%f,%f %f %f): %d\n", pIN->m_dLon, pIN->m_dLat, m_iID, m_dLonMin, m_dLonMax, m_dLatMin, m_dLatMax,(m_dLonMin <= pIN->m_dLon) && (pIN->m_dLon < m_dLonMax) && (m_dLatMin <= pIN->m_dLat) && (pIN->m_dLat < m_dLatMax));
    //display();

    //    printf("Checking if %30.27f,%30.27f is in region %d \n", pIN->m_dLon, pIN->m_dLat, m_iID);
    return ((m_dLonMin-EPS <= pIN->m_dLon) && (pIN->m_dLon <= m_dLonMax) &&
            (m_dLatMin-EPS <= pIN->m_dLat) && (pIN->m_dLat <= m_dLatMax)); 
}


//----------------------------------------------------------------------------
//  display
//
void LonLatRegion::display() {
    printf("Id %d Lon [%30.27f,%30.27f], Lat [%30.27f,%30.27f]\n", m_iID, m_dLonMin-EPS, m_dLonMax, m_dLatMin-EPS, m_dLatMax);
}

//----------------------------------------------------------------------------
//  serialize
//
unsigned char *LonLatRegion::serialize() {
    unsigned char *pS = new unsigned char[dataSize()];
    unsigned char *p = pS;
    p = putMem(p, &m_iID, sizeof(int));
    p = putMem(p, &m_dLonMin, sizeof(double));
    p = putMem(p, &m_dLonMax, sizeof(double));
    p = putMem(p, &m_dLatMin, sizeof(double));
    p = putMem(p, &m_dLatMax, sizeof(double));
    return p;           
}

//----------------------------------------------------------------------------
//  deserialize
//
int LonLatRegion::deserialize(unsigned char *pBuffer) {
    unsigned char *p = pBuffer;
    p = getMem(&m_iID, p, sizeof(int));
    p = getMem(&m_dLonMin, p, sizeof(double));
    p = getMem(&m_dLonMax, p, sizeof(double));
    p = getMem(&m_dLatMin, p, sizeof(double));
    p = getMem(&m_dLatMax, p, sizeof(double));
    return 0;
}
