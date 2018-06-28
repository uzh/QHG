
#include "GridProjection.h"
#include "IcoLoc.h"
#include "RectLoc.h"

RectLoc::RectLoc(GridProjection *pGrPr) :
    m_pGrPr(pGrPr),
    m_iW(pGrPr->getProjGrid()->m_iGridW) {
}

gridtype RectLoc::findNode(double dLon, double dLat) {
    double dX;
    double dY;
    m_pGrPr->sphereToGrid(dLon, dLat, dX, dY);
    //    printf("spher %f,%f -> grid %f,%f\n", dLon, dLat, dX, dY);
    gridtype iN = m_iW*(int)dY + (int)dX;
    return iN;
}


bool RectLoc::findCoords(int iNodeID, double *pdLon, double *pdLat) {
    double dX = iNodeID%m_iW;
    double dY = iNodeID/m_iW;
    m_pGrPr->gridToSphere(dX, dY, *pdLon, *pdLat);
    return true;
}
