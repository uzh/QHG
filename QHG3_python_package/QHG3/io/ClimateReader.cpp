#include <stdio.h>
#include <hdf5.h>

#include "Climate.h"
#include "QDFUtils.h"
#include "ClimateReader.h"

//----------------------------------------------------------------------------
// constructor
//
ClimateReader::ClimateReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hClimateGroup(H5P_DEFAULT),
       m_iNumCells(0) {
}

//----------------------------------------------------------------------------
// destructor
//
ClimateReader::~ClimateReader() {
}

//----------------------------------------------------------------------------
// createClimateReader
//
ClimateReader *ClimateReader::createClimateReader(const char *pFileName) {
    ClimateReader *pCR = new ClimateReader();
    int iResult = pCR->init(pFileName);
    if (iResult != 0) {
        delete pCR;
        pCR = NULL;
    }
    return pCR;
}

//----------------------------------------------------------------------------
// createClimateReader
//
ClimateReader *ClimateReader::createClimateReader(hid_t hFile) {
    ClimateReader *pCR = new ClimateReader();
    int iResult = pCR->init(hFile);
    if (iResult != 0) {
        delete pCR;
        pCR = NULL;
    }
    return pCR;
}

//----------------------------------------------------------------------------
// init
//
int ClimateReader::init(const char *pFileName) {
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
int ClimateReader::init(hid_t hFile) {
    int iResult = -1;
    m_hClimateGroup = qdf_openGroup(hFile, CLIGROUP_NAME);
    if (m_hClimateGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}

//----------------------------------------------------------------------------
// readAttributes
//
int ClimateReader::readAttributes(uint *piNumCells, int *piNumSeasons, bool *pbDynamic) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hClimateGroup, CLI_ATTR_NUM_CELLS, 1, piNumCells); 
        m_iNumCells = *piNumCells;
    }    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hClimateGroup, CLI_ATTR_NUM_SEASONS, 1, piNumSeasons); 
    }
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hClimateGroup, CLI_ATTR_DYNAMIC, 1, (char *)pbDynamic); 
    }
    return iResult;
}

//----------------------------------------------------------------------------
// readData
//
int ClimateReader::readData(Climate *pC) {
    int iResult = 0;
    // ther caller must make sure that the nmber of cells in the CellGrid
    // and in the Climate are identical
 
    if (iResult == 0) {
        iResult = qdf_readArray(m_hClimateGroup, CLI_DS_ACTUAL_TEMPS,   pC->m_iNumCells, pC->m_adActualTemps);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hClimateGroup, CLI_DS_ACTUAL_RAINS,   pC->m_iNumCells, pC->m_adActualRains);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hClimateGroup, CLI_DS_ANN_MEAN_TEMP,  pC->m_iNumCells, pC->m_adAnnualMeanTemp);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hClimateGroup, CLI_DS_ANN_TOT_RAIN,   pC->m_iNumCells, pC->m_adAnnualRainfall);
    }

    if (pC->m_iNumSeasons > 1) {
        if (iResult == 0) {
            iResult = qdf_readArray(m_hClimateGroup, CLI_DS_SEAS_TEMP_DIFF, m_iNumCells*pC->m_iNumSeasons, pC->m_adSeasTempDiff);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hClimateGroup, CLI_DS_SEAS_RAIN_RAT,  m_iNumCells*pC->m_iNumSeasons, pC->m_adSeasRainRatio);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hClimateGroup, CLI_DS_CUR_SEASON,   1, pC->m_adSeasRainRatio);
        }
    }


    if (iResult == 0) {    
        hid_t hDataSet   = H5Dopen(m_hClimateGroup, CLI_DS_CUR_SEASON, H5P_DEFAULT);
        herr_t status = H5Dread(hDataSet, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(pC->m_iCurSeason));
        qdf_closeDataSet(hDataSet);
        iResult = (status >= 0)?0:-1;
    }

    pC->m_bUpdated = true;

    return iResult;
}
