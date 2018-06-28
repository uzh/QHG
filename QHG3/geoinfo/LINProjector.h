/** ***************************************************************************\
*   \file   EQRProjector.h
*   \author jody
*   \brief  Header file for class EQRProjector
*
*   Implementation of a simple equirectangular Projection
*   (x=longitude, y=latitude)
*** ***************************************************************************/

#ifndef __IDRPROJECTOR_H__
#define __IDRPROJECTOR_H__

#include "Projector.h"

/** ***************************************************************************\
*   \class  EQRProjector.h
*   \brief  Header file for class EQRProjector
*
*   Implementation of a simple equirectangular Projection
*** ***************************************************************************/
class LINProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     LINProjector(double dLambda0,
    *                        double dPhi) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    LINProjector(double dLambda0, double dPhi0);

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

