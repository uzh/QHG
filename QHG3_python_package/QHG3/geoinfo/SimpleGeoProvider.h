/** ***************************************************************************\
*   \file   SimpleGeoProvider.h
*   \author jody
*   \brief  Header file for class SimpleGeoProvider
*
*   SimpleGeoProvider is based on an equidistant cylindrical projection, i.e.
*   x coordinate=longitude, y coordinate=latitude.
*** ***************************************************************************/
#ifndef __SIMPLEGEOPROVIDER_H__
#define __SIMPLEGEOPROVIDER_H__

#include "GeoProvider.h"

// forward

class DEM;


/** ***************************************************************************\
*   \class  SimpleGeoProvider
*   \brief  A GeoProvider baed on an equidistant cylindrical projection
*
*   SimpleGeoProvider is based on an equidistant cylindrical projection whose
*   origin corresponds to the crossing of the greenwich meridian and the equator., i.e.
*   This projection gets more distorted the closer one moves towards the poles.
*** ***************************************************************************/
class SimpleGeoProvider : public GeoProvider{
public:
    /** ***************************************************************************\
    *   \fn     SimpleGeoProvider(int iNumGridX, 
    *                             int iNumGridY,  
    *                             double dMinLon, 
    *                             double dMinLat, 
    *                             double dMaxLon, 
    *                             double dMaxLat
    *                             bool   bDeleteDEM=true);
    *   \brief  constructor
    *
    *   \param  iNumGridX   grid width  (number of cells in X direction)
    *   \param  iNumGridY   grid height (number of cells in X direction)
    *   \param  dMinLon     minimum longitude (left edge)
    *   \param  dMinLat     minimum latitude  (lower edge)
    *   \param  dMaxLon     maximum longitude (right edge)
    *   \param  dMaxLat     maximum latitude  (upper edge)
    *   \param  bDeleteDEM  delete DEM in destructor
    *
    *   Initializes membervariables and calculate deltas
    *** ***************************************************************************/
    SimpleGeoProvider(int iNumGridX, 
                      int iNumGridY,  
                      double dMinLon, 
                      double dMinLat, 
                      double dMaxLon, 
                      double dMaxLat, 
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
    double m_dMinLat;
    double m_dMinLon;

    double m_dDeltaX;
    double m_dDeltaY;
};



#endif
