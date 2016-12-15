#include "utils.h"
#include "RectUtils.h"
#include "GeoProvider.h"

//----------------------------------------------------------------------------
// constructor
//
GeoProvider::GeoProvider(int iNumGridX, int iNumGridY, bool bDeleteDEM)
:	m_iNumGridX(iNumGridX),
	m_iNumGridY(iNumGridY),
	m_iTileOffsetX(0),
	m_iTileOffsetY(0),
        m_dXGridSpacing(1),
        m_dYGridSpacing(1),

	m_bDeleteDEM(bDeleteDEM),
	m_pDEM(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
GeoProvider::~GeoProvider() {
    if (m_bDeleteDEM && (m_pDEM !=NULL)) {
        delete m_pDEM;
    }
}



//----------------------------------------------------------------------------
// setGridSpacing
//
void GeoProvider::setGridSpacing(double dXSpacing, double dYSpacing) {
    m_dXGridSpacing = dXSpacing;
    m_dYGridSpacing = dYSpacing;
}


//----------------------------------------------------------------------------
// setTile
//
void GeoProvider::setTile(rect *prTile) {
    m_iTileOffsetX = (int)round(prTile->left*m_dXGridSpacing);
    m_iTileOffsetY = (int)round(prTile->bottom*m_dYGridSpacing);
    printf("GP: offs (%d,%d)\n", m_iTileOffsetX, m_iTileOffsetY);
}


//----------------------------------------------------------------------------
// getWorldCoords
//   returns coordinates (in degerees) and altitude for given grid coords
//
bool GeoProvider::getWorldCoords(double dGridX, 
                                 double dGridY, 
                                 double &dLon, 
                                 double &dLat, 
                                 double &dAlt) {
    return getWorldCoordsImpl(dGridX + m_iTileOffsetX, 
                              dGridY + m_iTileOffsetY, 
                              dLon, 
                              dLat, 
                              dAlt);
}

bool GeoProvider::getWorldCoords(double dGridX, 
                                 double dGridY, 
                                 double *pdGeo) {
    return getWorldCoordsImpl(dGridX + m_iTileOffsetX, 
                              dGridY + m_iTileOffsetY, 
                              pdGeo[0], 
                              pdGeo[1], 
                              pdGeo[2]);
}

//----------------------------------------------------------------------------
// getGridCoords
//   returns grid coordinates for given geo coords (in degerees)
//
bool GeoProvider::getGridCoords(double dLon, 
                                double dLat, 
                                double &dGridX, 
                                double &dGridY) {
    bool bOK = getGridCoordsImpl(dLon, 
                                 dLat, 
                                 dGridX,
                                 dGridY);
                             
    dGridX -=m_iTileOffsetX;
    dGridX -=m_iTileOffsetY; 
    
    return bOK;
}



//----------------------------------------------------------------------------
// getCellArea
// 
bool GeoProvider::getCellArea(double  dGridXE, 
                              double  dGridYN, 
                              double  dGridXW, 
                              double  dGridYS,
                              double &dArea) {
    bool bOK = getCellAreaImpl(dGridXE+m_iTileOffsetX,
                               dGridYN+m_iTileOffsetY,
                               dGridXW+m_iTileOffsetX,
                               dGridYS+m_iTileOffsetY,
                               dArea);
    return bOK;
}
    
