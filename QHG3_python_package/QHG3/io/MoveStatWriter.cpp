#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "MoveStats.h"
#include "QDFUtils.h"
#include "MoveStatWriter.h"

//----------------------------------------------------------------------------
// constructor
//
MoveStatWriter::MoveStatWriter(MoveStats *pMS) 
    : m_pMS(pMS) {

    //    m_hCellDataType = createCellDataType();
}

//----------------------------------------------------------------------------
// writeToHDF
//
int MoveStatWriter::writeToQDF(const char *pFileName, float fTime) {
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
int MoveStatWriter::write(hid_t hFile) {
    int iResult = -1;

    if (m_pMS != NULL) {
        iResult = 0;
        hid_t hMStatGroup = qdf_opencreateGroup(hFile, MSTATGROUP_NAME);
        if (hMStatGroup > 0) {
            writeMStatAttributes(hMStatGroup);
            
            if (iResult == 0) {
                //                printf("int Hops[%d]\n", m_pMS->m_iNumCells);
                iResult = qdf_writeArray(hMStatGroup, MSTAT_DS_HOPS, m_pMS->m_iNumCells, m_pMS->m_aiHops);
            }
            
            if (iResult == 0) {
                //                printf("int Dist[%d]\n", m_pMS->m_iNumCells);
                iResult = qdf_writeArray(hMStatGroup, MSTAT_DS_DIST, m_pMS->m_iNumCells, m_pMS->m_adDist);
            }
            
            if (iResult == 0) {
                //                printf("int Time[%d]\n", m_pMS->m_iNumCells);
                iResult = qdf_writeArray(hMStatGroup, MSTAT_DS_TIME, m_pMS->m_iNumCells, m_pMS->m_adTime);
            }
            
            
            qdf_closeGroup(hMStatGroup);

        } else {
            iResult = -1;
            // couldn't open group
        }
    }
    return iResult;
}
//----------------------------------------------------------------------------
// writeGeoData
//
int MoveStatWriter::writeMStatAttributes(hid_t hMStatGroup) {
    int iResult = qdf_insertAttribute(hMStatGroup, MSTAT_ATTR_NUM_CELLS,  1, &m_pMS->m_iNumCells);
    
    return iResult;
}
