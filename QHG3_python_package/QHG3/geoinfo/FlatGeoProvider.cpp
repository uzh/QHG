#include "utils.h"
#include "FlatGeoProvider.h"



//----------------------------------------------------------------------------
// constructor
//
FlatGeoProvider::FlatGeoProvider(double dAlt)
:   GeoProvider(1, 1, false),
    m_dAlt(dAlt) {

}


//----------------------------------------------------------------------------
// GetInfoReal
//
bool FlatGeoProvider::getWorldCoordsImpl(double dGridX, 
                                         double dGridY, 
                                         double &dLon, 
                                         double &dLat, 
                                         double &dAlt)
{

    bool bOK = false;


    dAlt   = m_dAlt;

    return bOK;
}

