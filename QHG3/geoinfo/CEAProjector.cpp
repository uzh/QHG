#include <math.h>
#include "GeoInfo.h"
#include "CEAProjector.h"



//----------------------------------------------------------------------------
// constructor
//
CylindricalEqualAreaProjector:: CylindricalEqualAreaProjector(double dLambda0, double dPhi0)
    :   Projector(PR_CYLINDRICAL_EQUAL_AREA,
                  GeoInfo::getName(PR_CYLINDRICAL_EQUAL_AREA), 
                  dLambda0, 
                  dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void CylindricalEqualAreaProjector::sphere2Plane(double dLambda, 
                                                 double dPhi, 
                                                 double &dX, 
                                                 double &dY) {

    double dDeltaL = dLambda - m_dLambda0;
    dX = dDeltaL*cos(m_dPhi0);
    dY = sin(dPhi)/cos(m_dPhi0);
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void CylindricalEqualAreaProjector::plane2Sphere(double dX,
                                                 double dY,
                                                 double &dLambda,
                                                 double &dPhi) {

    
    dPhi    = asin(dY*cos(m_dPhi0));
    dLambda = m_dLambda0+dX/cos(m_dPhi0);
}

