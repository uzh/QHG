#include <math.h>
#include "GeoInfo.h"
#include "TCEAProjector.h"



//----------------------------------------------------------------------------
// constructor
//
TransverseCylindricalEqualAreaProjector:: TransverseCylindricalEqualAreaProjector(double dLambda0, double dPhi0)
    :   Projector(PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA,
                  GeoInfo::getName(PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA), 
                  dLambda0, 
                  dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void TransverseCylindricalEqualAreaProjector::sphere2Plane(double dLambda, 
                                                           double dPhi, 
                                                           double &dX, 
                                                           double &dY) {

    double dDeltaL = dLambda - m_dLambda0;
    dX = cos(dPhi)*sin(dDeltaL);
    // atan or atan2 - doesn't seem to matter
    dY = atan(tan(dPhi)/cos(dDeltaL)) - m_dPhi0;
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void TransverseCylindricalEqualAreaProjector::plane2Sphere(double dX,
                                                           double dY,
                                                           double &dLambda,
                                                           double &dPhi) {

    double dX1 = sqrt(1-dX*dX);
    double dY1 = dY + m_dPhi0;
    
    dPhi    = asin(dX1*sin(dY1));
    // atan or atan2 - doesn't seem to matter
    dLambda = m_dLambda0+atan(dX*cos(dY1)/dX1);
}

