#include <hdf5.h>
#include <string.h>

#include "SCell.h"
#include "QDFVisitUtils.h"
#include "QDFReaderUtils.h"



//-----------------------------------------------------------------------------
// createCellDataType
//
hid_t QDFReaderUtils::createCellDataType() {
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
int QDFReaderUtils::readCellData(hid_t hGridGroup, int nCells, SCell* aCells) {
    hid_t hCellType  = createCellDataType();
    hid_t hDataSet   = H5Dopen(hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    hsize_t dimsm = nCells;
    hid_t hMemSpace = H5Screate_simple(1, &dimsm, NULL);

    herr_t status = H5Dread(hDataSet, hCellType, hMemSpace, hDataSpace, H5P_DEFAULT, aCells);

    return (status >= 0)?0:-1;
}


