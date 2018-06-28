#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDF2SnapBase.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2SnapBase::QDF2SnapBase() 
    : m_iNumCells(0),
      m_hGridGroup(H5P_DEFAULT),
      m_hDataSetGrid(H5P_DEFAULT),
      m_hDataSpaceGrid(H5P_DEFAULT),
      m_hValueGroup(H5P_DEFAULT),
      m_hDataSetValue(H5P_DEFAULT),
      m_hDataSpaceValue(H5P_DEFAULT),
      m_pAttr(NULL) {
};

//----------------------------------------------------------------------------
// destructor
//
QDF2SnapBase::~QDF2SnapBase() {
    qdf_closeDataSpace(m_hDataSpaceGrid);
    qdf_closeDataSet(m_hDataSetGrid);

    qdf_closeGroup(m_hGridGroup);

    qdf_closeDataSpace(m_hDataSpaceValue);
    qdf_closeDataSet(m_hDataSetValue);

    qdf_closeGroup(m_hValueGroup);

}

//----------------------------------------------------------------------------
// init
//
int QDF2SnapBase::init(hid_t hFile, int iNumCells, const char *pGroupName, const char *pAttr) {
    int iResult = -1;

    m_pAttr = pAttr;

    m_iNumCells = iNumCells;
    m_hGridGroup = qdf_openGroup(hFile, GRIDGROUP_NAME);
    printf("[init] init gridgroup -> %d\n", m_hGridGroup);
    if (m_hGridGroup > 0) {
        m_hDataSetGrid = H5Dopen2(m_hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
        printf("[init] init griddataset -> %d\n", m_hDataSetGrid);
        if (m_hDataSetGrid > 0) {
            m_hDataSpaceGrid = H5Dget_space(m_hDataSetGrid);
            printf("[init] init griddataset -> %d\n", m_hDataSpaceGrid);
            if (m_hDataSpaceGrid > 0) {
                printf("Contents of group [%s]\n", pGroupName);
                m_hValueGroup = qdf_openGroup(hFile, pGroupName);
                printf("[init] init valuegroup -> %d (%d)\n", m_hValueGroup,H5P_DEFAULT);
                if (m_hValueGroup > 0) {
                    // open geo dataset fro attr
                    m_hDataSetValue = H5Dopen2(m_hValueGroup, pAttr, H5P_DEFAULT);
                    printf("[init] init valuedataset -> %d\n", m_hDataSetValue);
                    if (m_hDataSetValue > 0) {
                        // open geo dataspace for attr
                        m_hDataSpaceValue = H5Dget_space(m_hDataSetValue);
                        printf("[init] init valuedataspace -> %d\n", m_hDataSpaceValue);
                        if (m_hDataSpaceValue > 0) {
                            // open grid gro
                            iResult = 0;
                        } else {
                            printf("Couldn't open value dataspace for [%s]\n", pAttr);
                        }
                    } else {
                        printf("Couldn't open value dataset for [%s]\n", pAttr);
                    }
                    
                } else {
                    printf("Couldn't open group [%s]\n", pGroupName);
                }
            } else {
                printf("Couldn't open data space for grid\n");
            }
        } else {
            printf("Couldn't open data set for grid  [%s]\n", CELL_DATASET_NAME);
        }
    } else {
        printf("Couldn't open group [%s]\n", GRIDGROUP_NAME);
    }
    return iResult;

}

//----------------------------------------------------------------------------
// createSCellAgentType
//
hid_t QDF2SnapBase::createSCellAgentType() {
    //agent type for SCell
    hid_t hCellDataType = H5Tcreate (H5T_COMPOUND, sizeof(SCell));
    H5Tinsert(hCellDataType, "CellID",             HOFFSET(SCell, m_iGlobalID),      H5T_NATIVE_INT);
    H5Tinsert(hCellDataType, "NumNeighbors",       HOFFSET(SCell, m_iNumNeighbors),  H5T_NATIVE_UCHAR);
    hsize_t dims1 = MAX_NEIGH;
    hid_t hAttrArr = H5Tarray_create2(H5T_NATIVE_INT, 1, &dims1);
    H5Tinsert(hCellDataType, "Neighbors",          HOFFSET(SCell, m_aNeighbors), hAttrArr);

    return hCellDataType;
}

//----------------------------------------------------------------------------
// fillSimpleSnapDataDouble
//
int QDF2SnapBase::fillSimpleSnapDataDouble(char *pBuffer) {
    int iResult = 0;

    // buffers for data
    int    *pGridData  = new int[BUF_SIZE];
    double *pValueData = new double[BUF_SIZE];
    memset(pGridData, 170, BUF_SIZE*sizeof(int));
    memset(pValueData,  85, BUF_SIZE*sizeof(double));

    
    hsize_t dims =  m_iNumCells;

    dims = m_iNumCells;
    hsize_t offset  = 0;
    hsize_t dimsm   = BUF_SIZE;
    hid_t hCellDataType = H5Tcreate(H5T_COMPOUND, sizeof(int));
    herr_t status = H5Tinsert(hCellDataType, GRID_DS_CELL_ID, 0, H5T_NATIVE_INT);
    if (status < 0) {
        printf("Couldn't select field [%s] from dataspace\n", GRID_DS_CELL_ID);
        iResult = -1;
    }

    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
        
    char *pCur = pBuffer;

    while ((dims > 0) && (iResult ==0)) {
        // change Mem data space if necessary
        hsize_t count  = ((dims < BUF_SIZE)?dims:BUF_SIZE);
        if (count != dimsm) {
            qdf_closeDataSpace(hMemSpace); 
            dimsm = count;
            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
        }
        printf("next bit %lld from %lld\n", count, offset);

            
        // step size when going through data (stride =2: use every second element)
        hsize_t stride = 1;
        hsize_t block  = 1;
                    
        status = H5Sselect_hyperslab(m_hDataSpaceGrid, H5S_SELECT_SET, 
                                                &offset, &stride, &count, &block);
        if (status >= 0) {
            status = H5Dread(m_hDataSetGrid, hCellDataType, hMemSpace,
                             m_hDataSpaceGrid, H5P_DEFAULT, pGridData);
            if (status >= 0) {
            
                herr_t status = H5Sselect_hyperslab(m_hDataSpaceValue, H5S_SELECT_SET, 
                                                    &offset, &stride, &count, &block);
                if (status >= 0) {
                    status = H5Dread(m_hDataSetValue, H5T_NATIVE_DOUBLE, hMemSpace,
                                     m_hDataSpaceValue, H5P_DEFAULT, pValueData);

                    if (status >= 0) {
                        // data read ok: put to array
                        for (uint i = 0; i < count; i++) {
                            //                            pCur = putMem(pCur, &(pGridData[i].m_iGlobalID), sizeof(int));
                            pCur = putMem(pCur, &(pGridData[i]), sizeof(int));
                            pCur = putMem(pCur, &(pValueData[i]), sizeof(double));
                            
                        } 
                        dims -= count;
                        offset += count;
                    } else {
                        printf("reading from value hyperslab failed\n");
                        iResult = -1;
                    }
                } else {
                    printf("Hyperslab selection for value failed\n");
                    iResult = -1;
                }
            } else {
                printf("reading from grid hyperslab failed\n");
                iResult = -1;
            }
        } else {
            printf("Hyperslab selection for grid failed\n");
            iResult = -1;
        }
    }
    qdf_closeDataSpace(hMemSpace); 
    
    return iResult;
}

//----------------------------------------------------------------------------
// fillSimpleSnapDataInt
//
int QDF2SnapBase::fillSimpleSnapDataInt(char *pBuffer) {
    int iResult = 0;

    // buffers for data
    int    *pGridData  = new int[BUF_SIZE];
    char   *pValueData = new char[BUF_SIZE];
    memset(pGridData, 170, BUF_SIZE*sizeof(int));
    memset(pValueData,  85, BUF_SIZE*sizeof(char));

  
    hsize_t dims =  m_iNumCells;
    hsize_t offset = 0;

    hsize_t dimsm = BUF_SIZE;

    hid_t hCellDataType = H5Tcreate(H5T_COMPOUND, sizeof(int));
    herr_t status = H5Tinsert(hCellDataType, GRID_DS_CELL_ID, 0, H5T_NATIVE_INT);
    if (status < 0) {
        printf("Couldn't select field [%s] from dataspace\n", GRID_DS_CELL_ID);
        iResult = -1;
    }
    
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    char *pCur = pBuffer;

    while ((dims > 0) && (iResult ==0)) {
        // change Mem data space if necessary
        hsize_t count  = ((dims < BUF_SIZE)?dims:BUF_SIZE);
        if (count != dimsm) {
            qdf_closeDataSpace(hMemSpace); 
            dimsm = count;
            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
        }
        printf("next bit %lld from %lld\n", count, offset);

            
        // step size when going through data (stride =2: use every second element)
        hsize_t stride = 1;
        hsize_t block  = 1;
                    
        status = H5Sselect_hyperslab(m_hDataSpaceGrid, H5S_SELECT_SET, 
                                                &offset, &stride, &count, &block);
        if (status >= 0) {
            status = H5Dread(m_hDataSetGrid, hCellDataType, hMemSpace,
                             m_hDataSpaceGrid, H5P_DEFAULT, pGridData);
            if (status >= 0) {
                
                herr_t status = H5Sselect_hyperslab(m_hDataSpaceValue, H5S_SELECT_SET, 
                                                    &offset, &stride, &count, &block);
                if (status >= 0) {
                    status = H5Dread(m_hDataSetValue, H5T_NATIVE_CHAR, hMemSpace,
                                     m_hDataSpaceValue, H5P_DEFAULT, pValueData);

                    if (status >= 0) {
                        // data read ok: put to array
                        for (uint i = 0; i < count; i++) {
                            pCur = putMem(pCur, &(pGridData[i]), sizeof(int));
                            double d = pValueData[i];
                            pCur = putMem(pCur, &d, sizeof(double));
                            
                        } 
                        dims -= count;
                        offset += count;
                    } else {
                        printf("reading from value hyperslab failed\n");
                        iResult = -1;
                    }
                } else {
                    printf("Hyperslab selection for value failed\n");
                    iResult = -1;
                }
            } else {
                printf("reading from grid hyperslab failed\n");
                iResult = -1;
            }
        } else {
            printf("Hyperslab selection for grid failed\n");
            iResult = -1;
        }
    }
    qdf_closeDataSpace(hMemSpace); 
    
    return iResult;
}
