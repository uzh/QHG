#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Climate.h"
#include "QDFUtils.h"
#include "ClimateWriter.h"

//----------------------------------------------------------------------------
// constructor
//
ClimateWriter::ClimateWriter(Climate *pC) 
    : m_pC(pC) {

    //    m_hCellDataType = createCellDataType();
}

//----------------------------------------------------------------------------
// writeToHDF
//
int ClimateWriter::writeToQDF(const char *pFileName, float fTime) {
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
int ClimateWriter::write(hid_t hFile) {
    int iResult = -1;
    printf("write has climate: %p\n", m_pC);

    if (m_pC != NULL) {
        hid_t hClimateGroup = qdf_opencreateGroup(hFile, CLIGROUP_NAME);
        if (hClimateGroup > 0) {
            printf("writing Climate Attrs #%d\n", m_pC->m_iNumCells);
            writeClimateAttributes(hClimateGroup);
            
            printf("writing double ActualTemp[%d]\n", m_pC->m_iNumCells);
            iResult = qdf_writeArray(hClimateGroup, CLI_DS_ACTUAL_TEMPS,   m_pC->m_iNumCells, m_pC->m_adActualTemps);
            printf("writing double ActualRain[%d]\n", m_pC->m_iNumCells);
            iResult = qdf_writeArray(hClimateGroup, CLI_DS_ACTUAL_RAINS,   m_pC->m_iNumCells, m_pC->m_adActualRains);
            printf("writing double MeanTemp[%d]\n", m_pC->m_iNumCells);
            iResult = qdf_writeArray(hClimateGroup, CLI_DS_ANN_MEAN_TEMP,  m_pC->m_iNumCells, m_pC->m_adAnnualMeanTemp);
            printf("writing double TotRain[%d]\n", m_pC->m_iNumCells);
            iResult = qdf_writeArray(hClimateGroup, CLI_DS_ANN_TOT_RAIN,   m_pC->m_iNumCells, m_pC->m_adAnnualRainfall);
            
            if (m_pC->m_iNumSeasons > 0) {
                printf("writing double SeasTempDiff[%d]\n", m_pC->m_iNumCells*m_pC->m_iNumSeasons);
                iResult = qdf_writeArray(hClimateGroup, CLI_DS_SEAS_TEMP_DIFF, m_pC->m_iNumCells*m_pC->m_iNumSeasons, m_pC->m_adSeasTempDiff);
                printf("writing double SeasRainRat[%d]\n", m_pC->m_iNumCells*m_pC->m_iNumSeasons);
                iResult = qdf_writeArray(hClimateGroup, CLI_DS_SEAS_RAIN_RAT,  m_pC->m_iNumCells*m_pC->m_iNumSeasons, m_pC->m_adSeasRainRatio);
            }
            printf("writing char CurSeason[1]\n");
            iResult = qdf_writeArray(hClimateGroup, CLI_DS_CUR_SEASON,     1,                                   (char*) &m_pC->m_iCurSeason);
            printf("finished writing CLimate[1]\n");
            
            qdf_closeGroup(hClimateGroup);
         
        } else {
            // couldn't open group
        }
    } else {
//        iResult = 0;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// writeClimateData
//
int ClimateWriter::writeClimateAttributes(hid_t hClimateGroup) {
    int iResult = qdf_insertAttribute(hClimateGroup, CLI_ATTR_NUM_CELLS,  1, &m_pC->m_iNumCells);
    
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hClimateGroup, CLI_ATTR_NUM_SEASONS, 1, (char *)&m_pC->m_iNumSeasons);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hClimateGroup, CLI_ATTR_DYNAMIC, 1, (char *)&m_pC->m_bDynamic);
    }
    return iResult;
}
