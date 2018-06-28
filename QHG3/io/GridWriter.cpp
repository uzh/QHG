#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "SCell.h"
#include "SCellGrid.h"
#include "QDFUtils.h"
#include "GridWriter.h"

//----------------------------------------------------------------------------
// constructor
//
GridWriter::GridWriter(SCellGrid *pCG, stringmap *psm) 
    : m_pCG(pCG),
      m_psm((psm != NULL)?psm:&(pCG->m_smSurfaceData)) {
    
       
    m_hCellDataType = createCellDataType();
}

//----------------------------------------------------------------------------
// writeToQDF
//
int GridWriter::writeToQDF(const char *pFileName, float fTime, bool bNew) {
    int iResult = -1;
    hid_t hFile;
    if (bNew) {
        hFile = qdf_createFile(pFileName, fTime);
    } else {
        hFile = qdf_opencreateFile(pFileName, fTime);
    }
    if (hFile > 0) {
        iResult = write(hFile);
        qdf_closeFile(hFile);  
    }
    return iResult;
}

//----------------------------------------------------------------------------
// write
//
int GridWriter::write(hid_t hFile) {
    herr_t status = H5P_DEFAULT;

    hid_t hGridGroup = qdf_opencreateGroup(hFile, GRIDGROUP_NAME);
    if (hGridGroup > 0) {
        writeGridAttributes(hGridGroup);
        
        // Create the data space for the dataset.
        hsize_t dims = m_pCG->m_iNumCells; 
        hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
        if (hDataSpace > 0) {
            
            // Create the dataset
            hid_t hDataSet = H5Dcreate2(hGridGroup, CELL_DATASET_NAME, m_hCellDataType, hDataSpace, 
                                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            
            if (hDataSet > 0) {
                writeCellData(hDataSpace, hDataSet, m_hCellDataType); 
                qdf_closeDataSet(hDataSet);                
            }
            qdf_closeDataSpace(hDataSpace);
        } else {
            // couldn't open dataset
        }
        qdf_closeGroup(hGridGroup);
    } else {
        // couldn't open group
    }
    int iResult =  (status >= 0)?0:-1;
    return iResult;
}


//-----------------------------------------------------------------------------
// createCellDataType
//
hid_t GridWriter::createCellDataType() {
    hid_t hCellDataType = H5Tcreate (H5T_COMPOUND, sizeof(SCell));
    H5Tinsert(hCellDataType, GRID_DS_CELL_ID,    HOFFSET(SCell, m_iGlobalID),      H5T_NATIVE_INT);
    H5Tinsert(hCellDataType, GRID_DS_NUM_NEIGH,  HOFFSET(SCell, m_iNumNeighbors),  H5T_NATIVE_UCHAR);
    hsize_t dims = MAX_NEIGH;
    hid_t hAttrArr = H5Tarray_create2(H5T_NATIVE_INT, 1, &dims);
    H5Tinsert(hCellDataType, GRID_DS_NEIGHBORS,  HOFFSET(SCell, m_aNeighbors), hAttrArr);


    return hCellDataType;
}

//-----------------------------------------------------------------------------
// writeGridData
//
int GridWriter::writeGridAttributes(hid_t hGridGroup) {

    
    int iResult = qdf_insertAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, &m_pCG->m_iNumCells);
    
    stringmap::iterator it;
    for (it = m_psm->begin(); (iResult == 0) && (it != m_psm->end()); ++it) {
        iResult = qdf_insertSAttribute(hGridGroup, it->first.c_str(), it->second.c_str());
    }
    /*
    if (iResult == 0) {
        iResult = qdf_insertSAttribute(hGridGroup, GRID_ATTR_TYPE, SCellGrid::getGridSType(m_pCG->m_iType));
    }
    if (iResult == 0) {
        int aiFormat[2];
        if (m_pCG->m_iType == GRID_TYPE_ICO) {
            aiFormat[0] = 0;
            aiFormat[1] = 0;
        } else {
            aiFormat[0] = m_pCG->m_iW;
            aiFormat[1] = m_pCG->m_iH;
        }
        iResult = qdf_insertAttribute(hGridGroup, GRID_ATTR_FORMAT, 2, aiFormat);
    }
    if (iResult == 0) {
        char sParams[64];
        if (m_pCG->m_iType == GRID_TYPE_ICO) {
            strcpy(sParams, "");
        } else {
            sprintf(sParams, "%s", m_pCG->m_bPeriodic?"yes":"no");
        }
        iResult = qdf_insertSAttribute(hGridGroup, GRID_ATTR_PERIODIC, sParams);
    }

    */
    return iResult;
}

//-----------------------------------------------------------------------------
// writeCellData
//
int GridWriter::writeCellData(hid_t hDataSpace, hid_t hDataSet, hid_t hCellType) {
    hsize_t dimsm = m_pCG->m_iNumCells;
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
    
    herr_t status = H5Dwrite(hDataSet, hCellType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pCG->m_aCells);
    
    return (status >= 0)?0:-1;

}

