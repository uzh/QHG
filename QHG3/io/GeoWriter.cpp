#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Geography.h"
#include "QDFUtils.h"
#include "GeoWriter.h"

//----------------------------------------------------------------------------
// constructor
//
GeoWriter::GeoWriter(Geography *pGG) 
    : m_pGG(pGG) {

    //    m_hCellDataType = createCellDataType();
}

//----------------------------------------------------------------------------
// writeToHDF
//
int GeoWriter::writeToQDF(const char *pFileName, float fTime) {
    int iResult = -1;
    hid_t hFile = qdf_opencreateFile(pFileName, fTime);
    if (hFile > 0) {
        iResult = write(hFile);
        qdf_closeFile(hFile);  
    }
    return iResult;
}

//----------------------------------------------------------------------------
// write
//
int GeoWriter::write(hid_t hFile) {

    int iResult = -1;
    hid_t hGeoGroup = qdf_opencreateGroup(hFile, GEOGROUP_NAME);
    if (hGeoGroup > 0) {
        iResult = writeGeoAttributes(hGeoGroup);
        
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_LONGITUDE, m_pGG->m_iNumCells, m_pGG->m_adLongitude);
        }
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_LATITUDE,  m_pGG->m_iNumCells, m_pGG->m_adLatitude);
        }
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_ALTITUDE,  m_pGG->m_iNumCells, m_pGG->m_adAltitude);
        }
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_AREA,      m_pGG->m_iNumCells, m_pGG->m_adArea);
        }
        if (iResult == 0) {
            
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_DISTANCES, m_pGG->m_iNumCells*m_pGG->m_iMaxNeighbors, m_pGG->m_adDistances);
            // save the 1-dimensional boolean array abIce
        }
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_ICE_COVER, m_pGG->m_iNumCells, (char *)m_pGG->m_abIce);
        }
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_WATER,     m_pGG->m_iNumCells, m_pGG->m_adWater);
        }        
        if (iResult == 0) {
            iResult = qdf_writeArray(hGeoGroup, GEO_DS_COASTAL,   m_pGG->m_iNumCells,  (char *)m_pGG->m_abCoastal);
        }        
        qdf_closeGroup(hGeoGroup);
        
    } else {
        // couldn't open group
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeGeoData
//
int GeoWriter::writeGeoAttributes(hid_t hGeoGroup) {

    int iResult = qdf_insertAttribute(hGeoGroup, GEO_ATTR_NUM_CELLS,  1, &m_pGG->m_iNumCells);
    
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hGeoGroup, GEO_ATTR_MAX_NEIGH, 1, &m_pGG->m_iMaxNeighbors);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hGeoGroup, GEO_ATTR_RADIUS, 1, &m_pGG->m_dRadius);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hGeoGroup, GEO_ATTR_SEALEVEL, 1, &m_pGG->m_dSeaLevel);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// replace
//
int GeoWriter::replace(hid_t hFile) {

    int iResult = -1;
    hid_t hGeoGroup = qdf_openGroup(hFile, GEOGROUP_NAME);
    if (hGeoGroup > 0) {
        iResult = 0;

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_LONGITUDE, m_pGG->m_iNumCells, m_pGG->m_adLongitude);
        }

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_LATITUDE,  m_pGG->m_iNumCells, m_pGG->m_adLatitude);
        }

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_ALTITUDE,  m_pGG->m_iNumCells, m_pGG->m_adAltitude);
        }

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_AREA,      m_pGG->m_iNumCells, m_pGG->m_adArea);
        }            

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_DISTANCES, m_pGG->m_iNumCells*m_pGG->m_iMaxNeighbors, m_pGG->m_adDistances);
        }
        // save the 1-dimensional boolean array abIce
        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_ICE_COVER, m_pGG->m_iNumCells, (char *)m_pGG->m_abIce);
        }

        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_WATER,     m_pGG->m_iNumCells, m_pGG->m_adWater);
        }
        if (iResult == 0) {
            iResult = qdf_replaceArray(hGeoGroup, GEO_DS_COASTAL,     m_pGG->m_iNumCells, (char *)m_pGG->m_abCoastal);
        }
        qdf_closeGroup(hGeoGroup);
        
    } else {
        // couldn't open group
    }
    return iResult;
}

