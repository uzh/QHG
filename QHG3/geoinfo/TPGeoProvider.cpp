#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "TPGeoProvider.h"

//----------------------------------------------------------------------------
// constructor
//
TPGeoProvider::TPGeoProvider(int iNumGridX, 
                             int iNumGridY,  
                             double dWidth, 
                             double dHeight, 
                             double dLonCenter, 
                             double dLatCenter, 
                             double dRadius, 
                             bool bDeleteDEM)
:   GeoProvider(iNumGridX, iNumGridY, bDeleteDEM),
    m_dRadius(dRadius)
{
    calcMat(dLonCenter, dLatCenter);
    m_dXMin   = -dWidth/2;
    m_dYMin   = -dHeight/2;
    m_dDeltaX = dWidth/iNumGridX;
    m_dDeltaY = dHeight/iNumGridY;
}

//----------------------------------------------------------------------------
// getWorldCoordsImpl
//
bool TPGeoProvider::getWorldCoordsImpl(double dGridX, 
                                       double dGridY, 
                                       double &dLon, 
                                       double &dLat, 
                                       double &dAlt) {
    bool bOK = false;

    // from grid to vertical tangent plane H = {(u,v,w)| u = r}
    //  x-axis points to east (v = x)
    //  y-axis points north   (w = y)
    double dV = m_dXMin+m_dDeltaX*dGridX;
    double dW = m_dYMin+m_dDeltaY*dGridY;

    // project from tangent plane to sphere (only changes 1st coordinate)
    // actually: find u such that (u, v, w) on sphere with radius r
    double dDet = m_dRadius*m_dRadius - dV*dV - dW*dW;
    if (dDet >= 0) {
        double dU = sqrt(dDet);

        // projected point now (fU, fV, fW)
        // rotate to position
        double dX = m_matTrans[0][0]*dU + m_matTrans[0][1]*dV + m_matTrans[0][2]*dW;
        double dY = m_matTrans[1][0]*dU + m_matTrans[1][1]*dV + m_matTrans[1][2]*dW;
        double dZ = m_matTrans[2][0]*dU + m_matTrans[2][1]*dV + m_matTrans[2][2]*dW;

        // get longitude/latitude in degrees
        dLat = 180*asin(dZ/m_dRadius)/M_PI;
        dLon = 180*atan2(dY, dX)/M_PI;

        double dLatDeg = RAD2DEG(asin(dZ/m_dRadius));
        double dLonDeg = RAD2DEG(atan2(dY, dX));

        // now find indexes for DEM
        if (m_pDEM != NULL) {
            dAlt = m_pDEM->getAltitude(dLonDeg, dLatDeg);
            bOK = true;
        } else {
            dAlt = 0;
            bOK = false;
        }

    } else {
        // no intersection with sphere
        bOK = false;
    }

    return bOK;
}

//----------------------------------------------------------------------------
// calcMat
//
void TPGeoProvider::calcMat(double dLonCenter, double dLatCenter) {
    double dPhi = dLatCenter*M_PI/180;
    double cPhi = cos(dPhi);
    double sPhi = sin(dPhi);

    double dTheta = dLonCenter*M_PI/180;
    double cTheta = cos(dTheta);
    double sTheta = sin(dTheta);

    m_matTrans[0][0] =  cTheta*cPhi;
    m_matTrans[0][1] = -sTheta;
    m_matTrans[0][2] =  cTheta*sPhi;

    m_matTrans[1][0] =  sTheta*cPhi;
    m_matTrans[1][1] =  cTheta;
    m_matTrans[1][2] = -sTheta*sPhi;

    m_matTrans[2][0] =  sPhi;
    m_matTrans[2][1] =  0;
    m_matTrans[2][2] =  cPhi;
}
