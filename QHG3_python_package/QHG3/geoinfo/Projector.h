/** ***************************************************************************\
*   \file   Projector.h
*   \author jody
*   \brief  Header file for class Projector
*
*   Abstract base class for all Projector classes.
*   Projector classes provide transformation from the unit sphere to the plane
*   and vice versa.
*** ***************************************************************************/
#ifndef __PROJECTOR_H__
#define __PROJECTOR_H__

const int NAME_LEN = 128;

/** ***************************************************************************\
*   \class  Projector.h
*   \brief  Header file for class Projector
*
*   Abstract base class for all Projector classes
*** ***************************************************************************/
class Projector {

protected:
    int    m_iID;
    char   m_sName[NAME_LEN];
    double m_dLambda0; // reference longitude
    double m_dPhi0;    // reference latitude
    
public:
    virtual ~Projector() {};

    /** ***************************************************************************\
    *   \fn    Projector(const char *pName,
    *                    double dLambda0,
    *                    double dPhi0) 
    *   \brief  constructor
    *
    *   \param  pNAme      name of projection
    *   \param  dLambda0   longitude of projection center (in radians)
    *   \param  dPhi0      latitude  of projection center (in radians)
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    Projector(int iID,
              const char *pName, 
              double dLambda0, 
              double dPhi0);

    /** ***************************************************************************\
    *   \fn     void sphere2Plane(double dLambda, 
    *                             double dPhi, 
    *                             double &dX, 
    *                             double &dY)=0;
    *   \brief  convert spherical coordinates to cartesian
    *
    *   \param  dLambda longitude in radians (input)
    *   \param  dPhi    latitude  in radians (input)
    *   \param  dX      X coordinate  (output)
    *   \param  dY      Y coordinate  (output)
    *
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    virtual void sphere2Plane(double dLambda, 
                              double dPhi, 
                              double &dX, 
                              double &dY) = 0;

    /** ***************************************************************************\
    *   \fn     void plane2Sphere(double dX, 
    *                             double dY, 
    *                             double &dLambda, 
    *                             double &dPhi)=0;
    *   \brief  convert cartesian coordinates to spherical
    *
    *   \param  dX      X coordinate  (input)
    *   \param  dY      Y coordinate  (input)
    *   \param  dLambda longitude in radians (output)
    *   \param  dPhi    latitude  in radians (output)
    *
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    virtual void plane2Sphere(double dX,
                              double dY,
                              double &dLambda,
                              double &dPhi) = 0;

    /** ***************************************************************************\
    *   \fn     void setAdditional(int iNumAdd, double *pdAdd);
    *   \brief  set additional projection parameters
    *
    *   \param  iNumAdd number of additional parameters
    *   \param  pdAdd   array containing additional parameters
    *
    *   This method does nothing.
    *   Must be implemented in classes needing additional parameters
    *** ***************************************************************************/
    virtual void setAdditional(int iNumAdd, const double *pdAdd);

    const char *getName() const     { return m_sName;};
    
    int    getID() const      { return m_iID;};
    
    double getLambda0() const { return m_dLambda0;};
    
    double getPhi0() const    { return m_dPhi0;};


};

#endif

