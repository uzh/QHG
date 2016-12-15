#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "Projector.h"
#include "GeoInfo.h"
#include "PlaneGeoProvider.h"

//----------------------------------------------------------------------------
// constructor
//
PlaneGeoProvider::PlaneGeoProvider(const ProjGrid *pPG) 
    : GeoProvider(pPG->m_iGridW, pPG->m_iGridH, true),
      m_dDefAlt(100) {
}

//----------------------------------------------------------------------------
// destructor
//
PlaneGeoProvider::~PlaneGeoProvider() {
}

//----------------------------------------------------------------------------
// getWorldCoordsImpl
//  returns lon, lat in degrees
//
bool PlaneGeoProvider::getWorldCoordsImpl(double dGridX, 
                                          double dGridY, 
                                          double &dLon, 
                                          double &dLat, 
                                          double &dAlt) {
    bool bOK = true;


 
    dLon = dGridX;
    dLat = dGridY;
    

    if (bOK) {
        // now find indexes for DEM
   
        if (m_pDEM != NULL) {
            dAlt = m_pDEM->getAltitude(dLon, dLat);
            
            if (isnan(dAlt)) {
                //                printf("PrGeoProvider: G(%f,%f) -> S(%f,%f)\n", dGridX, dGridY, dLon, dLat);
            }

            bOK = true;
        } else {
            dAlt = m_dDefAlt;
            bOK = true;
        }
    }
    

    return bOK;
}

//-----------------------------------------------------------------------------
// getWorldCoordsImpl
//
bool PlaneGeoProvider::getGridCoordsImpl(double dLon, 
                                      double dLat, 
                                      double &dGridX, 
                                      double &dGridY) {
    bool bOK = true;

    dGridX = dLon;
    dGridY = dLat;
    return bOK;
}

//-----------------------------------------------------------------------------
// getCellArea
//
bool PlaneGeoProvider::getCellAreaImpl(double  dGridXE, 
                                       double  dGridYN, 
                                       double  dGridXW, 
                                       double  dGridYS,
                                       double &dArea) {
    dArea = (dGridXE-dGridXW)*(dGridYN-dGridYS);
    //    printf("calculating area (%f-%f)*(%f,%f)=%f!!!\n",dGridXE,dGridXW,dGridYN,dGridYS,dArea);
    return true;

}
