#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDF2GeoSnap.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2GeoSnap::QDF2GeoSnap() 
    : QDF2SnapBase(),
      m_pParam(NULL) {
};

//----------------------------------------------------------------------------
// destructor
//
QDF2GeoSnap::~QDF2GeoSnap() {
}


//----------------------------------------------------------------------------
// createInstance
//
QDF2GeoSnap *QDF2GeoSnap::createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    QDF2GeoSnap *pQG = new QDF2GeoSnap();
    int iResult = pQG->init(hFile, iNumCells, pAttr, pParam);
    if (iResult != 0) {
        delete pQG;
        pQG = NULL;
    }
    return pQG;
}

//----------------------------------------------------------------------------
// init
//
int QDF2GeoSnap::init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam) {
    int iResult = QDF2SnapBase::init(hFile, iNumCells, GEOGROUP_NAME, pAttr);

    if (iResult == 0) {
        m_pParam = pParam;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// fillSnapData
//
int QDF2GeoSnap::fillSnapData(char *pBuffer) {
    int iResult = -1;
    if (strcasecmp(m_pAttr, GEO_DS_ICE_COVER) == 0) {
        iResult = fillSimpleSnapDataInt(pBuffer);
    } else if (strcasecmp(m_pAttr, GEO_DS_DISTANCES) == 0) {
        if (m_pParam != NULL) {
            if (strcasecmp(m_pParam, "avg") == 0) {
                iResult = fillAvgDistData(pBuffer);
            }
        } else {
            printf("Need parameter (0,1,2,3,4,5, or avg) for \"Distances\"\n");
        }
    } else {
        iResult = fillSimpleSnapDataDouble(pBuffer);
    }
    return iResult;
}




//----------------------------------------------------------------------------
// fillAvgDistData
//
int QDF2GeoSnap::fillAvgDistData(char *pBuffer) {
    int iResult = 0;

   // buffers for data
    SCell  *pGridData = new SCell[BUF_SIZE];
    double *pGeoData  = new double[6*BUF_SIZE];
    memset(pGridData, 170, BUF_SIZE*sizeof(SCell));
    memset(pGeoData,  85, 6*BUF_SIZE*sizeof(double));


    hsize_t dims =  m_iNumCells;
    hsize_t offsetGrid = 0;
    hsize_t offsetGeo = 0;

    hsize_t dimsmGrid = BUF_SIZE;
    hsize_t dimsmGeo = 6*BUF_SIZE;

    hid_t hMemSpaceGeo  = H5Screate_simple (1, &dimsmGeo, NULL); 
    
    hid_t hCellDataType = createSCellAgentType();
    
    hid_t hMemSpaceGrid = H5Screate_simple (1, &dimsmGrid, NULL); 


    char *pCur = pBuffer;

    while ((dims > 0) && (iResult ==0)) {
        // change Mem data space if necessary
        hsize_t countGrid  = ((dims < BUF_SIZE)?dims:BUF_SIZE);
        hsize_t countGeo = 6*countGrid;
        if (countGrid != dimsmGrid) {
            qdf_closeDataSpace(hMemSpaceGrid); 
            dimsmGrid = countGrid;
            hMemSpaceGrid = H5Screate_simple (1, &dimsmGrid, NULL); 
            qdf_closeDataSpace(hMemSpaceGeo); 
            dimsmGeo = countGeo;
            hMemSpaceGeo = H5Screate_simple (1, &dimsmGeo, NULL); 
        }
        printf("next bit %lld from %lld\n", countGrid, offsetGrid);

            
        // step size when going through data (stride =2: use every second element)
        hsize_t stride = 1;
        hsize_t block  = 1;
                    
        herr_t status = H5Sselect_hyperslab(m_hDataSpaceGrid, H5S_SELECT_SET, 
                                            &offsetGrid, &stride, &countGrid, &block);
        if (status >= 0) {
            status = H5Dread(m_hDataSetGrid, hCellDataType, hMemSpaceGrid,
                             m_hDataSpaceGrid, H5P_DEFAULT, pGridData);
            if (status >= 0) {
                printf("Grid read ok\n");
                printf("Now set hyperslab at %lld for %lld elemets\n", offsetGeo, countGeo);
                herr_t status = H5Sselect_hyperslab(m_hDataSpaceValue, H5S_SELECT_SET, 
                                                    &offsetGeo, &stride, &countGeo, &block);
                if (status >= 0) {
                    status = H5Dread(m_hDataSetValue, H5T_NATIVE_DOUBLE, hMemSpaceGeo,
                                     m_hDataSpaceValue, H5P_DEFAULT, pGeoData);

                    if (status >= 0) {
                        // data read ok: put to array
                        for (uint i = 0; i < countGrid; i++) {
                            pCur = putMem(pCur, &(pGridData[i].m_iGlobalID), sizeof(int));
                            double d = 0;
                            for (int j = 1; j < pGridData[i].m_iNumNeighbors; j++) {
                                d += pGeoData[6*i+j];
                            }
                            d /= pGridData[i].m_iNumNeighbors; 
                            pCur = putMem(pCur, &d, sizeof(double));
                            
                        } 
                        dims -= countGrid;
                        offsetGrid += countGrid;
                        offsetGeo  += countGeo;
                    } else {
                        printf("reading from geo hyperslab failed\n");
                        iResult = -1;
                    }
                } else {
                    printf("Hyperslab selection for geo failed\n");
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
    qdf_closeDataSpace(hMemSpaceGrid); 
    qdf_closeDataSpace(hMemSpaceGeo); 
    

    return iResult;
}
