#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDF2VegSnap.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2VegSnap::QDF2VegSnap() 
    : QDF2SnapBase(),
      m_pParam(NULL),
      m_iVeg(-1) {
};

//----------------------------------------------------------------------------
// destructor
//
QDF2VegSnap::~QDF2VegSnap() {
}


//----------------------------------------------------------------------------
// createInstance
//
QDF2VegSnap *QDF2VegSnap::createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    QDF2VegSnap *pQV = new QDF2VegSnap();
    int iResult = pQV->init(hFile, iNumCells, pAttr, pParam);
    if (iResult != 0) {
        delete pQV;
        pQV = NULL;
    }
    return pQV;
}

//----------------------------------------------------------------------------
// init
//
int QDF2VegSnap::init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    int iResult = QDF2SnapBase::init(hFile, iNumCells, VEGGROUP_NAME, pAttr);

    if (iResult == 0) {
        m_pParam = pParam;
    }
    return iResult;
}
//----------------------------------------------------------------------------
// fillSnapData
//
int QDF2VegSnap::fillSnapData(char *pBuffer) {
    int iResult = -1;
    if (strcasecmp(m_pAttr, VEG_DS_MASS) == 0) {
        iResult = 0;
    } else if (strcasecmp(m_pAttr, VEG_DS_NPP) == 0) {
        iResult = 0;
    }
    if (iResult == 0) {
        if (m_pParam != NULL) {
            if (strToNum(m_pParam, &m_iVeg)) {
                iResult = fillSubArrayData(pBuffer);
            } else {
                printf("Veg type not numeric: [%s]\n", m_pParam);
            }
        } else {
            printf("Need parameter (0,1,2) for vegetation type\n");
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// fillSubArrayData
//
int QDF2VegSnap::fillSubArrayData(char *pBuffer) {
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
 

    hsize_t iCount2[2];
    iCount2[0] = 0;  
    iCount2[1] = 1;

    hsize_t iOffset2[2];
    iOffset2[0] = 0;
    iOffset2[1] = m_iVeg;
    hsize_t iStride2[2] = {1, 1};
    hsize_t iBlock2[2]  = {1, 1};
   
    hsize_t dims2[2];
    dims2[0] = dimsm;
    dims2[1] = 3;

    hid_t hMemSpace2 = H5Screate_simple (1, dims2, NULL); 

    char *pCur = pBuffer;

    while ((dims > 0) && (iResult ==0)) {
        // change Mem data space if necessary
        hsize_t count  = ((dims < BUF_SIZE)?dims:BUF_SIZE);
        iCount2[0] = count;  

        if (count != dimsm) {
            qdf_closeDataSpace(hMemSpace); 
            dimsm = count;
            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

            qdf_closeDataSpace(hMemSpace2); 
            dims2[0] = count;
            hMemSpace2 = H5Screate_simple (1, dims2, NULL); 

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
                                                    iOffset2, iStride2, iCount2, iBlock2);
                printf("[fillSubArrayData] selected veg hyperslab: %d\n", status);fflush(stdout);
                if (status >= 0) {
                    status = H5Dread(m_hDataSetValue, H5T_NATIVE_DOUBLE, hMemSpace2,
                                     m_hDataSpaceValue, H5P_DEFAULT, pValueData);
                    printf("[fillSubArrayData] read veg data: %d\n", status);fflush(stdout);

                    if (status >= 0) {
                        // data read ok: put to array
                        for (uint i = 0; i < count; i++) {
                            //                            pCur = putMem(pCur, &(pGridData[i].m_iGlobalID), sizeof(int));
                            pCur = putMem(pCur, &(pGridData[i]), sizeof(int));
                            pCur = putMem(pCur, &(pValueData[i]), sizeof(double));
                            
                        } 
                        dims -= count;
                        offset += count;
                        iOffset2[0] += count;
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
    qdf_closeDataSpace(hMemSpace2); 
    
    return iResult;
}
