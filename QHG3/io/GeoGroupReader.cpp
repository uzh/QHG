#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Geography.h"
#include "QDFUtils.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "GeoGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
GeoGroupReader::GeoGroupReader() {
}


//----------------------------------------------------------------------------
// createGeoGroupReader
//
GeoGroupReader *GeoGroupReader::createGeoGroupReader(const char *pFileName) {
    GeoGroupReader *pGR = new GeoGroupReader();
    int iResult = pGR->init(pFileName, GEOGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createGeoGroupReader
//
GeoGroupReader *GeoGroupReader::createGeoGroupReader(hid_t hFile) {
    GeoGroupReader *pGR = new GeoGroupReader();
    int iResult = pGR->init(hFile, GEOGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int GeoGroupReader::tryReadAttributes(GeoAttributes *pAttributes) {
    int iResult = GroupReader<Geography, GeoAttributes>::tryReadAttributes(pAttributes);

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_MAX_NEIGH, 1, &(pAttributes->m_iMaxNeighbors)); 
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_RADIUS, 1, &(pAttributes->m_dRadius)); 
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_SEALEVEL, 1, &(pAttributes->m_dSeaLevel)); 
    }                


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int GeoGroupReader::readArray(Geography *pGG, const char *pArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        ((m_pAttributes->m_iNumCells == pGG->m_iNumCells) && 
         (m_pAttributes->m_iMaxNeighbors == pGG->m_iMaxNeighbors))) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells or max neighbors do not correspond:\n");
            if (m_pAttributes->m_iNumCells != pGG->m_iNumCells) {
                printf("  GeoGroupReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pGG->m_iNumCells);
            }
            if (m_pAttributes->m_iMaxNeighbors != pGG->m_iMaxNeighbors) {
                printf("  GeoGroupReader::m_iMaxNeighbors: %d; Geography::m_iMaxNeighbors: %d\n", m_pAttributes->m_iMaxNeighbors,  pGG->m_iMaxNeighbors);
            }
        }
    }

    if (iResult == 0) {
        if (strcmp(pArrayName, GEO_DS_LONGITUDE) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_LONGITUDE, pGG->m_iNumCells, pGG->m_adLongitude);
        } else if (strcmp(pArrayName, GEO_DS_LATITUDE) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_LATITUDE,  pGG->m_iNumCells, pGG->m_adLatitude);
        } else if (strcmp(pArrayName, GEO_DS_ALTITUDE) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_ALTITUDE,  pGG->m_iNumCells, pGG->m_adAltitude);
        } else if (strcmp(pArrayName, GEO_DS_AREA) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_AREA,      pGG->m_iNumCells, pGG->m_adArea);
        } else if (strcmp(pArrayName, GEO_DS_DISTANCES) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_DISTANCES, pGG->m_iNumCells*pGG->m_iMaxNeighbors, pGG->m_adDistances);
        } else if (strcmp(pArrayName, GEO_DS_ICE_COVER) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_ICE_COVER, pGG->m_iNumCells, (char *)pGG->m_abIce);
        } else if (strcmp(pArrayName, GEO_DS_WATER) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_WATER,     pGG->m_iNumCells, pGG->m_adWater);
        } else if (strcmp(pArrayName, GEO_DS_COASTAL) == 0) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_COASTAL,   pGG->m_iNumCells, (char *)pGG->m_abCoastal);
        } else {
            printf("Unknown array [%s]\n", pArrayName);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int GeoGroupReader::readData(Geography *pGG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        ((m_pAttributes->m_iNumCells == pGG->m_iNumCells) && 
         (m_pAttributes->m_iMaxNeighbors == pGG->m_iMaxNeighbors))) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells or max neighbors do not correspond:\n");
            if (m_pAttributes->m_iNumCells != pGG->m_iNumCells) {
                printf("  GeoGroupReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pGG->m_iNumCells);
            }
            if (m_pAttributes->m_iMaxNeighbors != pGG->m_iMaxNeighbors) {
                printf("  GeoGroupReader::m_iMaxNeighbors: %d; Geography::m_iMaxNeighbors: %d\n", m_pAttributes->m_iMaxNeighbors,  pGG->m_iMaxNeighbors);
            }
        }
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_LONGITUDE, pGG->m_iNumCells, pGG->m_adLongitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_LATITUDE, pGG->m_iNumCells, pGG->m_adLatitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_ALTITUDE, pGG->m_iNumCells, pGG->m_adAltitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_AREA,     pGG->m_iNumCells, pGG->m_adArea);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_DISTANCES, pGG->m_iNumCells*pGG->m_iMaxNeighbors, pGG->m_adDistances);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_ICE_COVER, pGG->m_iNumCells, (char *)pGG->m_abIce);
        /*
            printf("Read ice    : ");
            for (int i = 0; i < 32; i++) {
                printf(" %d", pGG->m_abIce[i]);
            }
            printf(" ...\n");
        */
    }
    if (iResult == 0) {
        if (H5Lexists(m_hGroup, GEO_DS_WATER, H5P_DEFAULT)) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_WATER, pGG->m_iNumCells, pGG->m_adWater);
        }
    }
    if (iResult == 0) {
        if (H5Lexists(m_hGroup, GEO_DS_COASTAL, H5P_DEFAULT)) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_COASTAL, pGG->m_iNumCells,  (char *)pGG->m_abCoastal);
            /*
            printf("Read coastal: ");
            for (int i = 0; i < 32; i++) {
                printf(" %d", pGG->m_abCoastal[i]);
            }
            printf(" ...\n");
            */
        }
    }

    pGG->m_bUpdated = true;

    return iResult;


}
