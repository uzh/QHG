#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "MoveStats.h"
#include "QDFUtils.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "MoveStatGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
MoveStatGroupReader::MoveStatGroupReader() {
}


//----------------------------------------------------------------------------
// createMoveStatGroupReader
//
MoveStatGroupReader *MoveStatGroupReader::createMoveStatGroupReader(const char *pFileName) {
    MoveStatGroupReader *pGR = new MoveStatGroupReader();
    int iResult = pGR->init(pFileName, MSTATGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createMoveStatGroupReader
//
MoveStatGroupReader *MoveStatGroupReader::createMoveStatGroupReader(hid_t hFile) {
    MoveStatGroupReader *pGR = new MoveStatGroupReader();
    int iResult = pGR->init(hFile, MSTATGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int MoveStatGroupReader::tryReadAttributes(MoveStatAttributes *pAttributes) {
    int iResult = GroupReader<MoveStats, MoveStatAttributes>::tryReadAttributes(pAttributes);

 
    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int MoveStatGroupReader::readArray(MoveStats *pMG, const char *pArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pMG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells does not correspond:\n");
            printf("  MoveStatGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pMG->m_iNumCells);
        }
    }



    if (iResult == 0) {
        if (strcmp(pArrayName, MSTAT_DS_HOPS) == 0) {
            iResult = qdf_readArray(m_hGroup, MSTAT_DS_HOPS,   pMG->m_iNumCells, pMG->m_aiHops);
        } else if (strcmp(pArrayName, MSTAT_DS_DIST) == 0) {
            iResult = qdf_readArray(m_hGroup, MSTAT_DS_DIST,   pMG->m_iNumCells, pMG->m_adDist);
        } else if (strcmp(pArrayName, MSTAT_DS_TIME) == 0) {
            iResult = qdf_readArray(m_hGroup, MSTAT_DS_TIME,  pMG->m_iNumCells, pMG->m_adTime);
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
int MoveStatGroupReader::readData(MoveStats *pMG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pMG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            printf("Number of cells or max neighbors do not correspond:\n");
            printf("  MoveStatGroupReader::m_iNumCells: %d; Climate::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pMG->m_iNumCells);
        }
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, MSTAT_DS_HOPS,   m_iNumCells, pMG->m_aiHops);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, MSTAT_DS_DIST,   m_iNumCells, pMG->m_adDist);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, MSTAT_DS_TIME,   m_iNumCells, pMG->m_adTime);
    }


    return iResult;


}
