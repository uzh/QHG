#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Climate.h"
#include "QDFUtils.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "ClimateGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
ClimateGroupReader::ClimateGroupReader() {
}


//----------------------------------------------------------------------------
// createClimateGroupReader
//
ClimateGroupReader *ClimateGroupReader::createClimateGroupReader(const char *pFileName) {
    ClimateGroupReader *pGR = new ClimateGroupReader();
    int iResult = pGR->init(pFileName, CLIGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createClimateGroupReader
//
ClimateGroupReader *ClimateGroupReader::createClimateGroupReader(hid_t hFile) {
    ClimateGroupReader *pGR = new ClimateGroupReader();
    int iResult = pGR->init(hFile, CLIGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int ClimateGroupReader::tryReadAttributes(ClimateAttributes *pAttributes) {
    int iResult = GroupReader<Climate, ClimateAttributes>::tryReadAttributes(pAttributes);


    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, CLI_ATTR_NUM_SEASONS, 1,  &(pAttributes->m_iNumSeasons)); 
    }
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, CLI_ATTR_DYNAMIC, 1, (char *)(&pAttributes->m_bDynamic)); 
    }


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int ClimateGroupReader::readArray(Climate *pCG, const char *pArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pCG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells does not correspond:\n");
            printf("  ClimateGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pCG->m_iNumCells);
        }
    }
    
    if (iResult == 0) {
        if (strcmp(pArrayName, CLI_DS_ACTUAL_TEMPS) == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_TEMPS,   pCG->m_iNumCells, pCG->m_adActualTemps);
        } else if (strcmp(pArrayName, CLI_DS_ACTUAL_RAINS) == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_RAINS,   pCG->m_iNumCells, pCG->m_adActualRains);
        } else if (strcmp(pArrayName, CLI_DS_ANN_MEAN_TEMP) == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_MEAN_TEMP,  pCG->m_iNumCells, pCG->m_adAnnualMeanTemp);
        } else if (strcmp(pArrayName, CLI_DS_ANN_TOT_RAIN) == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_TOT_RAIN,   pCG->m_iNumCells, pCG->m_adAnnualRainfall);
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
int ClimateGroupReader::readData(Climate *pCG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pCG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells or max neighbors do not correspond:\n");
            printf("  ClimateGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pCG->m_iNumCells);
        }
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_TEMPS,   pCG->m_iNumCells, pCG->m_adActualTemps);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ACTUAL_RAINS,   pCG->m_iNumCells, pCG->m_adActualRains);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_MEAN_TEMP,  pCG->m_iNumCells, pCG->m_adAnnualMeanTemp);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, CLI_DS_ANN_TOT_RAIN,   pCG->m_iNumCells, pCG->m_adAnnualRainfall);
    }

    if (pCG->m_iNumSeasons > 1) {
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_SEAS_TEMP_DIFF, m_iNumCells*pCG->m_iNumSeasons, pCG->m_adSeasTempDiff);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_SEAS_RAIN_RAT,  m_iNumCells*pCG->m_iNumSeasons, pCG->m_adSeasRainRatio);
        }
        if (iResult == 0) {
            iResult = qdf_readArray(m_hGroup, CLI_DS_CUR_SEASON,   1, &(pCG->m_iCurSeason));
        }
 
    }

    pCG->m_bUpdated = true;
    pCG->notifyObservers(EVT_CLIMATE_CHANGE, NULL);
    return iResult;


}
