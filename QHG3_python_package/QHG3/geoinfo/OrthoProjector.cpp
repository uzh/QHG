#include <math.h>
#include "GeoInfo.h"
#include "OrthoProjector.h"



//----------------------------------------------------------------------------
// constructor
//
OrthographicProjector:: OrthographicProjector(double dLambda0, double dPhi0)
    : Projector(PR_ORTHOGRAPHIC, 
                GeoInfo::getName(PR_ORTHOGRAPHIC), 
                dLambda0, 
                dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void OrthographicProjector::sphere2Plane(double dLambda, 
                                         double dPhi, 
                                         double &dX, 
                                         double &dY) {
    
    dX = cos(dPhi)*sin(dLambda - m_dLambda0);
    dY = cos(m_dPhi0)*sin(dPhi)-sin(m_dPhi0)*cos(dPhi)*cos(dLambda-m_dLambda0);
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void OrthographicProjector::plane2Sphere(double dX,
                                         double dY,
                                         double &dLambda,
                                         double &dPhi) {
                                  
    double dRho = sqrt(dX*dX+dY*dY);
    double dC = asin(dRho);
    
    if (dRho == 0) {
        dPhi = m_dPhi0;
        dLambda = m_dLambda0;
    } else {
        // sin(dC) = sin(asin(rho)) = rho
        //        dPhi = asin(cos(dC)*sin(m_dPhi0)+dY*sin(dC)*cos(m_dPhi0)/dRho);
        dPhi = asin(cos(dC)*sin(m_dPhi0)+dY*cos(m_dPhi0));
        //        dLambda = m_dLambda0 + atan(dX*sin(dC)/(dRho*cos(m_dPhi0)*cos(dC) - dY*sin(m_dPhi0)*sin(dC)));
        dLambda = m_dLambda0 + atan2(dX, (cos(m_dPhi0)*cos(dC) - dY*sin(m_dPhi0)));
    }
}

