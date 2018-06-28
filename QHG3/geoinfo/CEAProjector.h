/** ***************************************************************************\
*   \file   CEAProjector.h
*   \author jody
*   \brief  Header file for class CylindricalEqualAreaProjector
*
*   Implementation of the Cylindrical Equal Area Projection
*   (http://mathworld.wolfram.com/CylindricalEqual-AreaProjection.html)
*** ***************************************************************************/
#ifndef __CEAPROJECTOR_H__
#define __CEAPROJECTOR_H__

#include "Projector.h"

/** ***************************************************************************\
*   \class  CEAProjector.h
*   \brief  Header file for class LCCTransverseCylindricalEqualAreaProjector
*
*   Implementation of the Transverse Cylindrical Equal Area Projection
*** ***************************************************************************/
class CylindricalEqualAreaProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     CylindricalEqualAreaProjector(double dLambda0,
    *                                                   double dPhi0) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    CylindricalEqualAreaProjector(double dLambda0, double dPhi0);

    /** ***************************************************************************\
    *   \fn     void sphere2Plane(double dLambda, 
    *                             double dPhi, 
    *                             double &dX, 
    *                             double &dY);
    *   \brief  convert spherical coordinates to cartesian
    *
    *   \param  dLambda longitude in radians (input)
    *   \param  dPhi    latitude  in radians (input)
    *   \param  dX      X coordinate  (output)
    *   \param  dY      Y coordinate  (output)
    *
    *** ***************************************************************************/
    virtual void sphere2Plane(double dLambda, 
                              double dPhi, 
                              double &dX, 
                              double &dY);

    /** ***************************************************************************\
    *   \fn     void plane2Sphere(double dX, 
    *                             double dY, 
    *                             double &dLambda, 
    *                             double &dPhi);
    *   \brief  convert cartesian coordinates to spherical
    *
    *   \param  dX      X coordinate  (input)
    *   \param  dY      Y coordinate  (input)
    *   \param  dLambda longitude in radians (output)
    *   \param  dPhi    latitude  in radians (output)
    *
    *** ***************************************************************************/
    virtual void plane2Sphere(double dX,
                              double dY,
                              double &dLambda,
                              double &dPhi);



};

#endif

