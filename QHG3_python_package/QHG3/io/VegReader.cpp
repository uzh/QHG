#include <stdio.h>
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
	    iResult = qdf_extractAttribute(m_hVegGroup, VEG_ATTR_DYNAMIC, m_iNumVegSpc, (char *)pV->m_abDynamic); 
    }

	if (H5Aexists(m_hVegGroup, VEG_ATTR_VARIANCE)) {
		iResult = qdf_extractAttribute(m_hVegGroup, VEG_ATTR_VARIANCE, m_iNumVegSpc, pV->m_adVariance);
	} else {
		iResult = 0;
		for (int i=0; i<m_iNumVegSpc; i++) {
			pV->m_adVariance[i] = 0;
		}
	}
    if (iResult == 0) {
        iResult = qdf_readArrays(m_hVegGroup, VEG_DS_MASS, m_iNumVegSpc, m_iNumCells, pV->m_adMass);
    }
    if (iResult == 0) {
        iResult = qdf_readArrays(m_hVegGroup, VEG_DS_NPP,  m_iNumVegSpc, m_iNumCells, pV->m_adANPP);
    }

 
    /*
    if (iResult == 0) {
        hsize_t dim = 1;
        hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
        hid_t hDataSet   = H5Dcreate2(m_hVegGroup, VEG_DS_PREV_TIME, H5T_NATIVE_FLOAT, hDataSpace, 
                                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        herr_t status = H5Dread(hDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &pV->m_iCurSeason);
        iResult = (status >= 0)?0:-1;
    }
    */

    pV->m_bUpdated = true;

    return iResult;
}
