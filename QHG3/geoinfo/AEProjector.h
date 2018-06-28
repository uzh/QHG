/** ***************************************************************************\
*   \file   AEProjector.h
*   \author jody
*   \brief  Header file for class AzimuthalEquidistantProjector
*
*   Implementation of the Azimuthal Equidistant Projection
*   (http://mathworld.wolfram.com/AzimuthalEquidistantProjection.html)
*** ***************************************************************************/

#ifndef __AEPROJECTOR_H__
#define __AEPROJECTOR_H__

#include "Projector.h"

/** ***************************************************************************\
*   \class  AEProjector.h
*   \brief  Header file for class AzimuthalEquidistantProjector
*
*   Implementation of the Azimuthal Equidistant Projection
*** ***************************************************************************/
class AzimuthalEquidistantProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     AzimuthalEquidistantProjector(double dLambda0,
    *                                         double dPhi0) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    AzimuthalEquidistantProjector(double dLambda0, double dPhi0);
    virtual ~AzimuthalEquidistantProjector() {};

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
    
    /** *************************************************************************** \
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

