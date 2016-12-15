#include "utils.h"
#include "SimpleGeoProvider.h"



//----------------------------------------------------------------------------
// constructor
//
SimpleGeoProvider::SimpleGeoProvider(int iGridW, int iGridH, double dMinLon, double dMinLat, double dMaxLon, double dMaxLat, bool bDeleteDEM)
:   GeoProvider(iGridW, iGridH, bDeleteDEM),
    m_dMinLat(dMinLat),
    m_dMinLon(dMinLon) {

    m_dDeltaX = (dMaxLon - dMinLon)/(iGridW-1);
    m_dDeltaY = (dMaxLat - dMinLat)/(iGridH-1);

}


//----------------------------------------------------------------------------
// getWorldCoordsImpl
//
bool SimpleGeoProvider::getWorldCoordsImpl(double dGridX, 
                                           double dGridY, 
                                           double &dLon, 
                                           double &dLat, 
                                           double &dAlt)
{
    bool bOK = true;
    dLat   = m_dMinLat + m_dDeltaY * dGridY;
    dLon   = m_dMinLon + m_dDeltaX * dGridX;

    // here: should get altitude from DEM file or so...
    // if can't be found: bOK = false;
    if (m_pDEM != NULL) {
        dAlt = m_pDEM->getAltitude(dLon, dLat);
        bOK = true;
    } else {
        bOK = false;
        dAlt = 0;
    }

    return bOK;
}

