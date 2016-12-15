/** ***************************************************************************\
*   \file   FlatGeoProvider.h
*   \author jody
*   \brief  Header file for class FlatGeoProvider
*
*   FlatGeoProvider ignores dem  - same geo data for every cell
*** ***************************************************************************/
#ifndef __FLATGEOPROVIDER_H__
#define __FLATGEOPROVIDER_H__

#include "GeoProvider.h"


/** ***************************************************************************\
*   \class  FlatGeoProvider
*   \brief  A Geoprovider which returns the same altitudes for all locations
*
*** ***************************************************************************/
class FlatGeoProvider : public GeoProvider{
public:
    /** ***************************************************************************\
    *   \fn     FlatGeoProvider(double dAlt);
    *   \brief  constructor
    *
    *   \param  dAlt     the altitude
    *
    *   Initializes membervariables and calculate deltas
    *** ***************************************************************************/
    FlatGeoProvider(double dAlt);

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
    *   Same as getWorldCoords, but corrected with tile offset
    *** ***************************************************************************/
    virtual bool getWorldCoordsImpl(double dGridX, 
                                    double dGridY, 
                                    double &dLon, 
                                    double &dLat, 
                                    double &dAlt);

    double m_dAlt;
};



#endif
