#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "SCellGrid.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "GridReader.h"

//----------------------------------------------------------------------------
// constructor
//
GridReader::GridReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hGridGroup(H5P_DEFAULT) {
}

//----------------------------------------------------------------------------
// destructor
//
GridReader::~GridReader() {
}

//----------------------------------------------------------------------------
// createGridReader
//
GridReader *GridReader::createGridReader(const char *pFileName) {
    GridReader *pGR = new GridReader();
    int iResult = pGR->init(pFileName);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createGridReader
//
GridReader *GridReader::createGridReader(hid_t hFile) {
    GridReader *pGR = new GridReader();
    int iResult = pGR->init(hFile);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// init
//
int GridReader::init(const char *pFileName) {
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
int GridReader::init(hid_t hFile) {
    int iResult = -1;
    m_hGridGroup = qdf_openGroup(hFile, GRIDGROUP_NAME);
    if (m_hGridGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}


//-----------------------------------------------------------------------------
// attr_info
//  callback for H5Aiterate2
//
herr_t attr_info(hid_t loc_id, const char *name, const H5A_info_t*pInfo, void *opdata) {
    stringmap *psm = (stringmap *)(opdata);

    hid_t attr = H5Aopen_name(loc_id, name);
    hid_t atype = H5Aget_type(attr);
    hid_t type_class = H5Tget_class(H5Aget_type(attr)); 
    if (type_class == H5T_STRING) {
        char sDum[256];
        H5Aread(attr, atype, sDum);
        (*psm)[name] = sDum;
    }
    return 0;
}


//----------------------------------------------------------------------------
// readAttributes
//
int GridReader::readAttributes(int *piNumCells, stringmap &smData) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGridGroup, GRID_ATTR_NUM_CELLS, 1, piNumCells); 
    }                

    if (iResult == 0) {
        herr_t status = H5Aiterate2(m_hGridGroup, H5_INDEX_NAME, H5_ITER_INC, NULL, attr_info, &smData);
        iResult = (status == 0)?0:-1;
    }
    /*

    if (iResult == 0) {
        char sType[64];
        iResult = qdf_extractSAttribute(m_hGridGroup, GRID_ATTR_TYPE, 64, sType); 
        if (iResult == 0) {
            *piGridType = SCellGrid::getGridType(sType);
            if (*piGridType == GRID_TYPE_NONE) {
                printf("Bad Grid Type [%s]\n", sType);
                iResult =-1;
            }
        }
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGridGroup, GRID_ATTR_FORMAT, 2, piFormat); 
    }                
    if (iResult == 0) {
        *pbPeriodic = false;
        char sParam[64];
        iResult = qdf_extractSAttribute(m_hGridGroup, GRID_ATTR_PERIODIC, 16, sParam); 
        if (iResult == 0) {
            if (strcasecmp(sParam, "yes") == 0) {
                *pbPeriodic = true;
            }
        }
    }            
    */    
    return iResult;
}



//-----------------------------------------------------------------------------
// createCellDataType
//
hid_t GridReader::createCellDataType() {
    hid_t hCellDataType = H5Tcreate (H5T_COMPOUND, sizeof(SCell));
    H5Tinsert(hCellDataType, GRID_DS_CELL_ID,    HOFFSET(SCell, m_iGlobalID),      H5T_NATIVE_INT);
    H5Tinsert(hCellDataType, GRID_DS_NUM_NEIGH,  HOFFSET(SCell, m_iNumNeighbors),  H5T_NATIVE_UCHAR);
    hsize_t dims = MAX_NEIGH;
    hid_t hAttrArr = H5Tarray_create2(H5T_NATIVE_INT, 1, &dims);
    H5Tinsert(hCellDataType, GRID_DS_NEIGHBORS,  HOFFSET(SCell, m_aNeighbors), hAttrArr);


    return hCellDataType;
}


//-----------------------------------------------------------------------------
// readCellData
//
int GridReader::readCellData(SCellGrid *pCG) {
    hid_t hCellType = createCellDataType();
    hid_t hDataSet   = H5Dopen(m_hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    hsize_t dimsm = pCG->m_iNumCells;
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
    
    herr_t status = H5Dread(hDataSet, hCellType, hMemSpace, hDataSpace, H5P_DEFAULT, pCG->m_aCells);
    
    for (uint i = 0; i < pCG->m_iNumCells; i++) {
        pCG->m_mIDIndexes[pCG->m_aCells[i].m_iGlobalID]=i;
    }
    return (status >= 0)?0:-1;

}
