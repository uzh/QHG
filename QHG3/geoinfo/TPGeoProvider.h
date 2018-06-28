/** ***************************************************************************\
*   \file   TPGeoProvider.h
*   \author jody
*   \brief  Header file for class SimpleGeoProvider
*
*   TPGeoProvider is baed on an orthographic azimuthal projection, i.e.
*   parallel projection onto a tangential plane
*** ***************************************************************************/
#ifndef __TPGEOPROVIDER_H__
#define __TPGEOPROVIDER_H__

#include "DEM.h"
#include "GeoProvider.h"

/** ***************************************************************************\
*   \class  TPGeoProvider
*   \brief  Header file for class SimpleGeoProvider
*
*   TPGeoProvider is baed on an orthographic azimuthal projection, i.e.
*   parallel projection onto a tangential plane with a specifiable contact point.
*   This projection igets more distorted the farther one moves away from the
*   contact point.
*   This implementation assumes the earth's axis to coincide with the global
*   z-axis. Then it calculates the projection of the tangential plane x=r
*   to the sphere along the x axis. This projection is followed by a rotation
*   which moves the contact point (r, 0, 0) to the actual contact point.
*   This rotation works in such a way that the planes local y-axis remains
*   parallel to the meridians (i.e. local north) and the x-axis rmains parallel
*   to the latitude circles (i.e. local east)
*** ***************************************************************************/
class TPGeoProvider : public GeoProvider {
public:
    /** ***********************************************************************\
    *   \fn     TPGeoProvider(int iNumGridX, 
    *                         int iNumGridY, 
    *                         double dWidth, 
    *                         double dHeight, 
    *                         double dLonCenter, 
    *                         double dLatCenter, 
    *                         double dRadius);
    *   \brief  constructor
    *
    *   \param  iNumGridX   grid width  (number of cells in X direction)
    *   \param  iNumGridY   grid height (number of cells in X direction)
    *   \param  dWidth      real width of grid (i.e. of tangential rectangle)
    *   \param  dHeight     real height of grid (i.e. of tangential rectangle)
    *   \param  dLonCenter  longitude of projection center
    *   \param  dLatCenter  latitude of projection center
    *   \param  dRadius     radius of earth (same unit as fWidth & fHeight)
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    TPGeoProvider(int iNumGridX, 
                  int iNumGridY,  
                  double dWidth, 
                  double dHeight, 
                  double dLonCenter, 
                  double dLatCenter, 
                  double dRadius, 
                  bool bDeleteDEM=true);

protected:
    /** ***************************************************************************\
    *   \fn     virtual bool getWorldCoordsImpl(double dGridX, 
    *                                           double dGridY, 
    *                                           double &dLon, 
    *                                           double &dLat, 
    *                                           double &dAlt);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dGridX      grid x coordinate
    *   \param  dGridY      grid y coordinate
    *   \param  dLon        real longitude  (output)
    *   \param  dLat        real latitude   (output)
    *   \param  dAlt        real altitude   (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *   Same as GetInfo, but corrected with tile offset
    *** ***************************************************************************/
    virtual bool getWorldCoordsImpl(double dGridX, 
                                    double dGridY, 
                                    double &dLon, 
                                    double &dLat, 
                                    double &dAlt);


    double m_dXMin;
    double m_dYMin;
    double m_dRadius;
    double m_matTrans[3][3];

    void calcMat(double dLonCenter, double dLatCenter);

    double m_dDeltaX;
    double m_dDeltaY;

};



#endif
