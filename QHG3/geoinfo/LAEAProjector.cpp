#include <math.h>
#include "GeoInfo.h"
#include "LAEAProjector.h"



//----------------------------------------------------------------------------
// constructor
//
LambertAzimuthalEqualAreaProjector:: LambertAzimuthalEqualAreaProjector(double dLambda0, double dPhi0)
    : Projector(PR_LAMBERT_AZIMUTHAL_EQUAL_AREA,
                GeoInfo::getName(PR_LAMBERT_AZIMUTHAL_EQUAL_AREA), 
                dLambda0, 
                dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void LambertAzimuthalEqualAreaProjector::sphere2Plane(double dLambda, 
                                                      double dPhi, 
                                                      double &dX, 
                                                      double &dY) {
    
    double dL = dLambda - m_dLambda0;
    double dK = sqrt(2/(1+sin(m_dPhi0)*sin(dPhi)+cos(m_dPhi0)*cos(dPhi)*cos(dL)));
    dX = dK*cos(dPhi)*sin(dL);
    dY = dK*(cos(m_dPhi0)*sin(dPhi)-sin(m_dPhi0)*cos(dPhi)*cos(dL));
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void LambertAzimuthalEqualAreaProjector::plane2Sphere(double dX,
                                                      double dY,
                                                      double &dLambda,
                                                      double &dPhi) {
         

    double dRho = sqrt(dX*dX+dY*dY);
    double dC = 2*asin(dRho/2);
    if (dRho == 0) {
        dPhi    = m_dPhi0;
        dLambda = m_dLambda0;
    } else {
        dPhi = asin(cos(dC)*sin(m_dPhi0)+dY*sin(dC)*cos(m_dPhi0)/dRho);
        // atan2 must be used!
        dLambda = m_dLambda0+atan2(dX*sin(dC), dRho*cos(m_dPhi0)*cos(dC) - dY*sin(m_dPhi0)*sin(dC));
    }    
}

