#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Vegetation.h"
#include "QDFUtils.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "VegGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
VegGroupReader::VegGroupReader() {
}


//----------------------------------------------------------------------------
// createVegGroupReader
//
VegGroupReader *VegGroupReader::createVegGroupReader(const char *pFileName) {
    VegGroupReader *pGR = new VegGroupReader();
    int iResult = pGR->init(pFileName, VEGGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createVegGroupReader
//
VegGroupReader *VegGroupReader::createVegGroupReader(hid_t hFile) {
    VegGroupReader *pGR = new VegGroupReader();
    int iResult = pGR->init(hFile, VEGGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int VegGroupReader::tryReadAttributes(VegAttributes *pAttributes) {
    int iResult = GroupReader<Vegetation, VegAttributes>::tryReadAttributes(pAttributes);


    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, VEG_ATTR_NUM_SPECIES, 1,  &(pAttributes->m_iNumVegSpc)); 
    }


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int VegGroupReader::readArray(Vegetation *pVG, const char *pArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pVG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells does not correspond:\n");
            printf("  VegGroupReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pVG->m_iNumCells);
        }
    }
    if (iResult == 0) {
        if (strcmp(pArrayName, VEG_DS_BASE_NPP) == 0) {
            iResult = qdf_readArray(m_hGroup, VEG_DS_BASE_NPP, pVG->m_iNumCells, pVG->m_adBaseANPP);
        } else if (strcmp(pArrayName, VEG_DS_NPP) == 0) {
            iResult = qdf_readArray(m_hGroup, VEG_DS_NPP, pVG->m_iNumCells, pVG->m_adTotalANPP);
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
int VegGroupReader::readData(Vegetation *pVG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pVG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells or max neighbors do not correspond:\n");
            printf("  VegGroupReader::m_iNumCells: %d; Vegetation::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pVG->m_iNumCells);
        }
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, VEG_DS_BASE_NPP, m_iNumCells, pVG->m_adBaseANPP);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, VEG_DS_NPP, m_iNumCells, pVG->m_adTotalANPP);
    }

    return iResult;
}
