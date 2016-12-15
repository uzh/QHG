#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "Projector.h"
#include "PrGeoProvider.h"
#include "GridProjection.h"

//----------------------------------------------------------------------------
// constructor
//
PrGeoProvider::PrGeoProvider(int iGridW,
                             int iGridH,
                             double dWidth,
                             double dHeight,
                             double dRadius,
                             Projector *pProj,
                             bool bDeleteProjector,
                             bool bDeleteDEM)
    :   GeoProvider(iGridW, iGridH, bDeleteDEM),
        m_pProj(pProj), m_dDefAlt(100) {

    m_pGP = new GridProjection(iGridW, iGridH, dWidth, dHeight, dRadius, pProj, bDeleteProjector);
}

//----------------------------------------------------------------------------
// constructor
//
PrGeoProvider::PrGeoProvider(const ProjGrid *pPG,
                             Projector *pProj,
                             bool bDeleteProjector,
                             bool bDeleteDEM) 
    : GeoProvider(pPG->m_iGridW, pPG->m_iGridH, bDeleteDEM), 
      m_pProj(pProj), m_dDefAlt(100) {

    m_pGP = new GridProjection(pPG, pProj, false, bDeleteProjector);
}

//----------------------------------------------------------------------------
// destructor
//
PrGeoProvider::~PrGeoProvider() {
    if (m_pGP != NULL) {
        printf("[PrGeoProvider]---------------- deleting  GridProjection(%p)\n", m_pGP);

        delete m_pGP;
    }
}

//----------------------------------------------------------------------------
// getWorldCoordsImpl
//  returns lon, lat in degrees
//
bool PrGeoProvider::getWorldCoordsImpl(double dGridX, 
                                       double dGridY, 
                                       double &dLon, 
                                       double &dLat, 
                                       double &dAlt) {
    bool bOK = false;


    bOK = m_pGP->gridToSphere(dGridX, dGridY, dLon, dLat);
    //        printf("PrGeoProvider: G(%f,%f) -> S(%f,%f,%f)\n", dGridX, dGridY, dLon, dLat, dAlt);
    

    dLon = RAD2DEG(dLon);
    dLat = RAD2DEG(dLat);
    

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
bool PrGeoProvider::getGridCoordsImpl(double dLon, 
                                      double dLat, 
                                      double &dGridX, 
                                      double &dGridY) {
    bool bOK = false;

    dLon = DEG2RAD(dLon);
    dLat = DEG2RAD(dLat);
    

    bOK = m_pGP->sphereToGrid(dLon, dLat, dGridX, dGridY);

    return bOK;
}

//-----------------------------------------------------------------------------
// getCellArea
//
bool PrGeoProvider::getCellAreaImpl(double  dGridXE, 
                                    double  dGridYN, 
                                    double  dGridXW, 
                                    double  dGridYS,
                                    double &dArea) {
    return m_pGP->getCellArea(dGridXE, dGridYN, dGridXW, dGridYS, dArea);

}
