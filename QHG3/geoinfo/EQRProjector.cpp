#include "GeoInfo.h"
#include "EQRProjector.h"



//----------------------------------------------------------------------------
// constructor
//
EQRProjector:: EQRProjector(double dLambda0, double dPhi0)
    : Projector(PR_EQUIRECTANGULAR,
                GeoInfo::getName(PR_EQUIRECTANGULAR), 
                dLambda0, 
                dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void EQRProjector::sphere2Plane(double dLambda, 
                                   double dPhi, 
                                   double &dX, 
                                   double &dY) {

    dX = dLambda - m_dLambda0;
    dY = dPhi    - m_dPhi0;

}

//----------------------------------------------------------------------------
// plane2Sphere
//
void EQRProjector::plane2Sphere(double dX,
                                   double dY,
                                   double &dLambda,
                                   double &dPhi) {
                                  
    dPhi    = dY + m_dPhi0;
    dLambda = dX + m_dLambda0;
   
}

