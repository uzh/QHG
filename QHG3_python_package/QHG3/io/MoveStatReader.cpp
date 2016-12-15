#include <stdio.h>
#include <hdf5.h>

#include "MoveStats.h"
#include "QDFUtils.h"
#include "MoveStatReader.h"

//----------------------------------------------------------------------------
// constructor
//
MoveStatReader::MoveStatReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hMoveStatGroup(H5P_DEFAULT) {
}

//----------------------------------------------------------------------------
// destructor
//
MoveStatReader::~MoveStatReader() {
}

//----------------------------------------------------------------------------
// createMoveStatReader
//
MoveStatReader *MoveStatReader::createMoveStatReader(const char *pFileName) {
    MoveStatReader *pMR = new MoveStatReader();
    int iResult = pMR->init(pFileName);
    if (iResult != 0) {
        delete pMR;
        pMR = NULL;
    }
    return pMR;
}

//----------------------------------------------------------------------------
// createMoveStatReader
//
MoveStatReader *MoveStatReader::createMoveStatReader(hid_t hFile) {
    MoveStatReader *pMR = new MoveStatReader();
    int iResult = pMR->init(hFile);
    if (iResult != 0) {
        delete pMR;
        pMR = NULL;
    }
    return pMR;
}

//----------------------------------------------------------------------------
// init
//
int MoveStatReader::init(const char *pFileName) {
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
int MoveStatReader::init(hid_t hFile) {
    int iResult = -1;
    m_hMoveStatGroup = qdf_openGroup(hFile, MSTATGROUP_NAME);
    if (m_hMoveStatGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}

//----------------------------------------------------------------------------
// readAttributes
//
int MoveStatReader::readAttributes(uint *piNumCells) {
    int iResult = qdf_extractAttribute(m_hMoveStatGroup, MSTAT_ATTR_NUM_CELLS, 1, piNumCells); 
    m_iNumCells = *piNumCells;

    return iResult;
}

//----------------------------------------------------------------------------
// readData
//
int MoveStatReader::readData(MoveStats *pMS) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if (m_iNumCells == pMS->m_iNumCells) {
        iResult = 0;
    } else {
        iResult = -1;
        printf("Number of cells do not correspond:\n");
        printf("  MoveStatReader::m_iNumCells: %d; MoveStats::m_iNumCells: %d\n", m_iNumCells,  pMS->m_iNumCells);
    }

    if (iResult == 0) {
        iResult = qdf_readArray(m_hMoveStatGroup, MSTAT_DS_HOPS,   m_iNumCells, pMS->m_aiHops);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hMoveStatGroup, MSTAT_DS_DIST,   m_iNumCells, pMS->m_adDist);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hMoveStatGroup, MSTAT_DS_TIME,   m_iNumCells, pMS->m_adTime);
    }


    return iResult;
}



