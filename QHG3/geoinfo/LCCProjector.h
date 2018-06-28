/** ***************************************************************************\
*   \file   LCCProjector.h
*   \author jody
*   \brief  Header file for class LambertConformalConicalProjector
*
*   Implementation of the Lambert Conformal Conical Projection
*   (http://mathworld.wolfram.com/LambertConformalConicProjection.html)
*** ***************************************************************************/

#ifndef __LCCPROJECTOR_H__
#define __LCCPROJECTOR_H__

#include <math.h>
#include "utils.h"
#include "Projector.h"

/** ***************************************************************************\
*   \class  LambertConformalConicalProjector
*   \brief  Implementation  of the Lambert Conformal Conical Projection
*
*   Implementation of the Lambert Conformal Conical Projector
*** ***************************************************************************/
class LambertConformalConicalProjector : public Projector {
    
public:
    /** ***************************************************************************\
    *   \fn     LambertConformalConicalProjector(double dLambda0,
    *                                            double dPhi0,
    *                                            double dPhi1,
    *                                            double dPhi2) 
    *   \brief  constructor
    *
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   \param  dPhi1      latitude  of first  standard parallel (in radians)
    *   \param  dPhi2      latitude  of second standard parallel (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    LambertConformalConicalProjector(double dLambda0, double dPhi0, double dPhi1=0, double dPhi2=M_PI/6);

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

    /** ***************************************************************************\
    *   \fn     void setAdditional(int iNumAdd, double *pdAdd);
    *   \brief  set additional projection parameters
    *
    *   \param  iNumAdd number of additional parameters
    *   \param  pdAdd   array containing additional parameters
    *
    *   The Lambert Conformal Conical Projection needs two additional parameters:
    *   the latitudes of the two standard parallels (in radians)
    *** ***************************************************************************/
    virtual void setAdditional(int iNumAdd, const double *pdAdd);

    /** ***************************************************************************\
    *   \fn     void SetStandardLatitudes(double dPhi1, double dPhi2);
    *   \brief  set the two standard parallels
    *
    *   \param  dPhi1   latitude of first  standard parallel (in radians)
    *   \param  dPhi2   latitude of second standard parallel (in radians)
    *
    *   The Lambert Conformal Conical Projection needs two additional parameters:
    *   the latitudes of the two standard parallels
    *** ***************************************************************************/
    void setStandardLatitudes(double dPhi1, double dPhi2);
     
protected:
    /** ***************************************************************************\
    *   \fn     void CalcConsts();
    *   \brief  calculate some projection constants
    *
    *** ***************************************************************************/
    void calcConsts();
    double m_dPhi1;
    double m_dPhi2;
	
    double m_dN;
    double m_dF;
    double m_dRho0;


};

#endif

