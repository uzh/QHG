#include <stdio.h>
#include <hdf5.h>

#include "Geography.h"
#include "QDFUtils.h"
#include "GeoReader.h"

//----------------------------------------------------------------------------
// constructor
//
GeoReader::GeoReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hGeoGroup(H5P_DEFAULT) {
}

//----------------------------------------------------------------------------
// destructor
//
GeoReader::~GeoReader() {
}

//----------------------------------------------------------------------------
// createGeoReader
//
GeoReader *GeoReader::createGeoReader(const char *pFileName) {
    GeoReader *pGR = new GeoReader();
    int iResult = pGR->init(pFileName);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createGeoReader
//
GeoReader *GeoReader::createGeoReader(hid_t hFile) {
    GeoReader *pGR = new GeoReader();
    int iResult = pGR->init(hFile);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// init
//
int GeoReader::init(const char *pFileName) {
    int iResult = -1;
    
    hid_t hFile = qdf_openFile(pFileName);
    if (hFile > 0) {
        iResult = init(hFile);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// init
//
int GeoReader::init(hid_t hFile) {
    int iResult = -1;
    m_hGeoGroup = qdf_openGroup(hFile, GEOGROUP_NAME);
    if (m_hGeoGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}

//----------------------------------------------------------------------------
// readAttributes
//
int GeoReader::readAttributes(uint *piNumCells, int *piMaxNeighbors, double *pdRadius, double *pdSeaLevel) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGeoGroup, GEO_ATTR_NUM_CELLS, 1, piNumCells); 
        m_iNumCells = *piNumCells;
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGeoGroup, GEO_ATTR_MAX_NEIGH, 1, piMaxNeighbors); 
        m_iMaxNeighbors = *piMaxNeighbors;
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGeoGroup, GEO_ATTR_RADIUS, 1, pdRadius); 
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGeoGroup, GEO_ATTR_SEALEVEL, 1, pdSeaLevel); 
    }                
    return iResult;
}

//----------------------------------------------------------------------------
// readData
//
int GeoReader::readData(Geography *pGG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_iNumCells == pGG->m_iNumCells) && (m_iMaxNeighbors == pGG->m_iMaxNeighbors)) {
        iResult = 0;
    } else {
        iResult = -1;
        printf("Number of cells or max neighbors do not correspond:\n");
        if (m_iNumCells != pGG->m_iNumCells) {
            printf("  GeoReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_iNumCells,  pGG->m_iNumCells);
        }
        if (m_iMaxNeighbors != pGG->m_iMaxNeighbors) {
            printf("  GeoReader::m_iMaxNeighbors: %d; Geography::m_iMaxNeighbors: %d\n", m_iMaxNeighbors,  pGG->m_iMaxNeighbors);
        }
    }


    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LONGITUDE, pGG->m_iNumCells, pGG->m_adLongitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LATITUDE, pGG->m_iNumCells, pGG->m_adLatitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_ALTITUDE, pGG->m_iNumCells, pGG->m_adAltitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_AREA,     pGG->m_iNumCells, pGG->m_adArea);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_DISTANCES, pGG->m_iNumCells*pGG->m_iMaxNeighbors, pGG->m_adDistances);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_ICE_COVER, pGG->m_iNumCells, (char *)pGG->m_abIce);
    }
    if (iResult == 0) {
        if (H5Lexists(m_hGeoGroup, GEO_DS_WATER, H5P_DEFAULT)) {
            iResult = qdf_readArray(m_hGeoGroup, GEO_DS_WATER, pGG->m_iNumCells, pGG->m_adWater);
        }
    }

    pGG->m_bUpdated = true;

    return iResult;
}
