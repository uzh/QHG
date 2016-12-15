#include "GeoInfo.h"
#include "LINProjector.h"



//----------------------------------------------------------------------------
// constructor
//
LINProjector:: LINProjector(double dLambda0, double dPhi0)
    : Projector(PR_LINEAR,
                GeoInfo::getName(PR_LINEAR), 
                dLambda0, 
                dPhi0) {
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void LINProjector::sphere2Plane(double dLambda, 
                                   double dPhi, 
                                   double &dX, 
                                   double &dY) {

    dX = dLambda;
    dY = dPhi;

}

//----------------------------------------------------------------------------
// plane2Sphere
//
void LINProjector::plane2Sphere(double dX,
                                   double dY,
                                   double &dLambda,
                                   double &dPhi) {
                                  
    dPhi    = dY;
    dLambda = dX;
   
}

