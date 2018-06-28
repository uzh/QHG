/** ***************************************************************************\
*   \file   TCEAProjector.h
*   \author jody
*   \brief  Header file for class TransverseCylindricalEqualAreaProjector
*
*   Implementation of the Transverse Cylindrical Equal Area Projection
*   (http://mathworld.wolfram.com/CylindricalEqual-AreaProjection.html)
*** ***************************************************************************/
#ifndef __TCEAPROJECTOR_H__
#define __TCEAPROJECTOR_H__

#include "Projector.h"

/** ***************************************************************************\
*   \class  TCEAProjector.h
*   \brief  Header file for class LCCTransverseCylindricalEqualAreaProjector
*
*   Implementation of the Transverse Cylindrical Equal Area Projection
*** ***************************************************************************/
class TransverseCylindricalEqualAreaProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     TransverseCylindricalEqualAreaProjector(double dLambda0,
    *                                                   double dPhi0) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    TransverseCylindricalEqualAreaProjector(double dLambda0, double dPhi0);
    virtual ~TransverseCylindricalEqualAreaProjector() {};

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

