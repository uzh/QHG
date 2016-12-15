/*****************************************************************************************\
| Azimuthal Equidistant Projector
|
| http://mathworld.wolfram.com/AzimuthalEquidistantProjection.html
\*****************************************************************************************/

#include <math.h>
#include "utils.h"
#include "GeoInfo.h"
#include "AEProjector.h"



//----------------------------------------------------------------------------
// constructor
//
AzimuthalEquidistantProjector:: AzimuthalEquidistantProjector(double dLambda0, double dPhi0)
    : Projector(PR_AZIMUTHAL_EQUIDISTANT, 
                GeoInfo::getName(PR_AZIMUTHAL_EQUIDISTANT), 
                dLambda0, 
                dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void AzimuthalEquidistantProjector::sphere2Plane(double dLambda, 
                                                 double dPhi, 
                                                 double &dX, 
                                                 double &dY) {

    double dDeltaL = dLambda - m_dLambda0;
    double dcc = sin(m_dPhi0)*sin(dPhi)+cos(m_dPhi0)*cos(dPhi)*cos(dDeltaL);
    double dc  = acos(dcc);
    double dK  = (dc==0)?1:dc/sin(dc);

    // had to insert the "-" to avoid mirror image
    dX = dK*cos(dPhi)*sin(dDeltaL);
    dY = dK*(cos(m_dPhi0)*sin(dPhi)-sin(m_dPhi0)*cos(dPhi)*cos(dDeltaL));
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void AzimuthalEquidistantProjector::plane2Sphere(double dX,
                                                 double dY,
                                                 double &dLambda,
                                                 double &dPhi) {
                                  
    double dc = sqrt(dX*dX+dY*dY);
    double ds = (dc==0)?1:sin(dc)/dc;
    dPhi = asin(cos(dc)*sin(m_dPhi0)+dY*cos(m_dPhi0)*ds);
    //    dPhi = asin(cos(dc)*sin(m_dPhi0)+dY*sin(dc)*cos(m_dPhi0)/dc);
    
    double dL;
                
    if (m_dPhi0 == M_PI/2) {
        // must be atan
        dL = atan(-dX/dY);
    } else if (m_dPhi0 == -M_PI/2) {
        // must be atan
        dL = atan(dX/dY);
    } else {
        // must be atan2
        dL = atan2(dX*sin(dc), dc*cos(m_dPhi0)*cos(dc) - dY*sin(m_dPhi0)*sin(dc));
    }
    
    
    dLambda = m_dLambda0 + dL; 
}

