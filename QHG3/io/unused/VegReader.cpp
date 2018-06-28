#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "NPPVeg.h"
#include "QDFUtils.h"
#include "VegReader.h"

//----------------------------------------------------------------------------
// constructor
//
VegReader::VegReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hVegGroup(H5P_DEFAULT) {
}


//----------------------------------------------------------------------------
// destructor
//
VegReader::~VegReader() {
}


//----------------------------------------------------------------------------
// createVegReader
//
VegReader *VegReader::createVegReader(const char *pFileName) {
    VegReader *pVR = new VegReader();
    int iResult = pVR->init(pFileName);
    if (iResult != 0) {
        delete pVR;
        pVR = NULL;
    }
    return pVR;
}


//----------------------------------------------------------------------------
// createVegReader
//
VegReader *VegReader::createVegReader(hid_t hFile) {
    VegReader *pVR = new VegReader();
    int iResult = pVR->init(hFile);
    if (iResult != 0) {
        delete pVR;
        pVR = NULL;
    }
    return pVR;
}


//----------------------------------------------------------------------------
// init
//
int VegReader::init(const char *pFileName) {
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
int VegReader::init(hid_t hFile) {
    int iResult = -1;
    m_hVegGroup = qdf_openGroup(hFile, VEGGROUP_NAME);
    if (m_hVegGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}


//----------------------------------------------------------------------------
// readAttributes
//
int VegReader::readAttributes(uint *piNumCells, int *piNumVegSpc, bool *pbDynamic) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hVegGroup, VEG_ATTR_NUM_CELLS, 1, piNumCells); 
        m_iNumCells = *piNumCells;
    }    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hVegGroup, VEG_ATTR_NUM_SPECIES, 1, piNumVegSpc); 
        m_iNumVegSpc = *piNumVegSpc;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// readArray
//
int VegReader::readArray(Vegetation *pV, const char *pArrayName) {
    // the caller must make sure that the nmber of cells in the CellGrid
    // and in the Vegetation are identical

    int iResult = -1;
    if (strcmp(pArrayName, VEG_DS_BASE_NPP) == 0) {
        iResult = qdf_readArray(m_hVegGroup, VEG_DS_BASE_NPP, pV->m_iNumCells, pV->m_adBaseANPP);
    } else if (strcmp(pArrayName, VEG_DS_NPP) == 0) {
        iResult = qdf_readArray(m_hVegGroup, VEG_DS_NPP, pV->m_iNumCells, pV->m_adTotalANPP);
    } else {
        printf("Unknown array [%s]\n", pArrayName);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// readData
//
int VegReader::readData(Vegetation *pV) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_iNumCells == pV->m_iNumCells) /*&& (m_iNumVegSpc == pV->m_iNumVegSpecies)*/){
        pV->m_iNumVegSpecies = m_iNumVegSpc;
        iResult = 0;
    } else {
        iResult = -1;
        printf("Number of cells or number pf species do not correspond:\n");
        printf("  VegReader::m_iNumCells: %d; Vegetation::m_iNumCells: %d\n", m_iNumCells,  pV->m_iNumCells);
        printf("  VegReader::m_iNumVegSpc: %d; Vegetation::m_iNumVegSpc: %d\n", m_iNumVegSpc,  pV->m_iNumVegSpecies);
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hVegGroup, VEG_DS_BASE_NPP, m_iNumCells, pV->m_adBaseANPP);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hVegGroup, VEG_DS_NPP, m_iNumCells, pV->m_adTotalANPP);
    }


    return iResult;
}
