#include <stdio.h>

#include "strutils.h"
#include "GridProjection.h"

#include "IcoNode.h"
#include "Region.h"
#include "RectRegion.h"

#define EPS 1e-16

//----------------------------------------------------------------------------
//  constructor
//
RectRegion::RectRegion(int iID, GridProjection *pGP, double dH, double dXMin, double dXMax, double dYMin, double dYMax)
    : Region(iID),
      m_pGP(pGP),
      m_dH(dH),
      m_iGridWidth(0),
      m_dXMin(dXMin),
      m_dXMax(dXMax),
      m_dYMin(dYMin),
      m_dYMax(dYMax) {
    if (m_pGP != NULL) {
        m_iGridWidth = m_pGP->getProjGrid()->m_iGridW+1; 
    }
}

//----------------------------------------------------------------------------
//  contains
//
bool RectRegion::contains(IcoNode *pBC) {
    
    double dX;
    double dY;

    /*
    dX = pBC->m_lID%m_iGridWidth;
    dY = pBC->m_lID/m_iGridWidth;
    */
    m_pGP->sphereToGrid(pBC->m_dLon, pBC->m_dLat, dX, dY);
    dY = dY/m_dH;
    if ((m_dH < 1) && (((int)dY)%2 == 0)) {
        dX -= 0.5;
    }
    printf("ID %d -> %f, %f\n", pBC->m_lID, dX, dY);
    return ((m_dXMin-EPS <= dX) && (dX < m_dXMax) &&
            (m_dYMin-EPS <= dY) && (dY < m_dYMax)); 
}

//----------------------------------------------------------------------------
//  display
//
void RectRegion::display() {
    //    printf("Id %d X [%21.18f,%21.18f], Y [%21.18f,%21.18f]\n", m_iID, m_dXMin-EPS, m_dXMax, m_dYMin-EPS, m_dYMax);
    printf("Id %d X [%f,%f], Y [%f,%f]\n", m_iID, m_dXMin-EPS, m_dXMax, m_dYMin-EPS, m_dYMax);
}

//----------------------------------------------------------------------------
//  serialize
//TODO: GP?
unsigned char *RectRegion::serialize() {
    unsigned char *pS = new unsigned char[dataSize()];
    unsigned char *p = pS;
    p = putMem(p, &m_iID, sizeof(int));
    p = putMem(p, &m_dXMin, sizeof(double));
    p = putMem(p, &m_dXMax, sizeof(double));
    p = putMem(p, &m_dYMin, sizeof(double));
    p = putMem(p, &m_dYMax, sizeof(double));
    return p;           
}

//----------------------------------------------------------------------------
//  deserialize
//
int RectRegion::deserialize(unsigned char *pBuffer) {
    unsigned char *p = pBuffer;
    p = getMem(&m_iID, p, sizeof(int));
    p = getMem(&m_dXMin, p, sizeof(double));
    p = getMem(&m_dXMax, p, sizeof(double));
    p = getMem(&m_dYMin, p, sizeof(double));
    p = getMem(&m_dYMax, p, sizeof(double));
    return 0;
}
