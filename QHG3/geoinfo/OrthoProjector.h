/** ***************************************************************************\
*   \file   OrthoProjector.h
*   \author jody
*   \brief  Header file for class OrthographicProjector
*
*   Implementation of the Orthographic Projection
*   (http://mathworld.wolfram.com/OrthographicProjection.html)
*** ***************************************************************************/
#ifndef __ORTHOPROJECTOR_H__
#define __ORTHOPROJECTOR_H__

#include "Projector.h"

/** ***************************************************************************\
*   \class  OrthoProjector.h
*   \brief  Header file for class OrthographicProjector
*
*   Implementation of the Orthographic Projection
*** ***************************************************************************/
class OrthographicProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     OrthographicProjector(double dLambda0,
    *                                              double dPhi0) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    OrthographicProjector(double dLambda0, double dPhi0);
    virtual ~OrthographicProjector() {};

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

