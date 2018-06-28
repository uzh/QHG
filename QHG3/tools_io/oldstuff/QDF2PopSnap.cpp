#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include "strutils.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDF2PopSnap.h"

#define BUF_SIZE 65536

//----------------------------------------------------------------------------
// constructor
//
QDF2PopSnap::QDF2PopSnap() 
    : m_iNumCells(0),
      m_hSpeciesGroup(H5P_DEFAULT),
      m_hDataSetPop(H5P_DEFAULT),
      m_hDataSpacePop(H5P_DEFAULT), 
      m_hGridGroup(H5P_DEFAULT),
      m_hDataSetGrid(H5P_DEFAULT),
      m_hDataSpaceGrid(H5P_DEFAULT),
      m_pSpecies(NULL) {


};

//----------------------------------------------------------------------------
// destructor
//
QDF2PopSnap::~QDF2PopSnap() {
    qdf_closeDataSpace(m_hDataSpacePop);
    
    qdf_closeDataSet(m_hDataSetPop);

    qdf_closeDataSpace(m_hDataSpaceGrid);
    
    qdf_closeDataSet(m_hDataSetGrid);
    

    qdf_closeGroup(m_hSpeciesGroup);
    qdf_closeGroup(m_hGridGroup);
}

//----------------------------------------------------------------------------
// createInstance
//
QDF2PopSnap *QDF2PopSnap::createInstance(hid_t hFile, int iNumCells, const char *pSpecies, const char *pGridFile) {
    QDF2PopSnap *pQP = new QDF2PopSnap();
    int iResult = pQP->init(hFile, iNumCells, pSpecies, pGridFile);
  
    if (iResult != 0) {
        delete pQP;
        pQP = NULL;
    }
  
    return pQP;
}


//----------------------------------------------------------------------------
// init
//
int QDF2PopSnap::init(hid_t hFile, int iNumCells, const char *pSpecies, const char *pGridFile) {
    int iResult = -1;
    
   
 
    m_iNumCells = iNumCells;
    m_pSpecies  = pSpecies;

    bool bPopExists = H5Lexists(hFile, POPGROUP_NAME, H5P_DEFAULT);
    if (bPopExists) {
        
        // get pop related stuff
        printf("opening pop-related stuff\n");
        hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
        if (hPopGroup > 0) {
            printf("OKOK\n");
            bool bSpeciesExists =  H5Lexists(hPopGroup, m_pSpecies, H5P_DEFAULT);
            if (bSpeciesExists) {
                m_hSpeciesGroup = qdf_openGroup(hPopGroup, m_pSpecies);
                if (m_hSpeciesGroup > 0) {
                    
                    // open pop dataset for species
                    m_hDataSetPop = H5Dopen2(m_hSpeciesGroup, AGENT_DATASET_NAME, H5P_DEFAULT);
                    if (m_hDataSetPop > 0) {
                        // open geo dataspace for attr
                        m_hDataSpacePop = H5Dget_space(m_hDataSetPop);
                        if (m_hDataSpacePop > 0) {
                            iResult = 0;
                            printf("pop-related stuff opened OK!\n");
                            
                        } else {
                            printf("Couldn't open pop dataspace for [%s]\n", m_pSpecies);
                        }
                        
                    } else {
                        printf("Couldn't open popdataset for [%s]\n", m_pSpecies);
                    }
                    
                } else {
                    printf("Couldn't open species group [%s]\n", m_pSpecies);
                }
            } else {
                printf("Couldn't find species group [%s]\n", m_pSpecies);
            }
            qdf_closeGroup(hPopGroup);
        } else {
            printf("Couldn't open group [%s]\n", POPGROUP_NAME);
        }
    
        if (iResult == 0) {

            if (pGridFile != NULL) {
                printf("Opening file [%s]\n", pGridFile);
                hid_t hFileGrid = qdf_openFile(pGridFile);
                if (hFileGrid != H5P_DEFAULT) {
                    printf("file [%s] open: %d\n", pGridFile, hFileGrid);
                    m_hGridGroup = qdf_openGroup(hFileGrid, GRIDGROUP_NAME);
                    // qdf_closeFile(hFileGrid);
                } else {
                    printf("Couldn't open file [%s]\n", pGridFile);
                    iResult = -1;
                }

            } else {
                m_hGridGroup = qdf_openGroup(hFile, GRIDGROUP_NAME);
            }
    

            if (m_hGridGroup > 0) {
                printf("opened gridgroup: %d\n", m_hGridGroup);
                // open grid gro
                m_hDataSetGrid = H5Dopen2(m_hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
                if (m_hDataSetGrid > 0) {
                    m_hDataSpaceGrid = H5Dget_space(m_hDataSetGrid);
                    if (m_hDataSpaceGrid > 0) {
                        iResult = 0;
                    } else {
                        printf("Couldn't open grid dataspace for [%s]\n", CELL_DATASET_NAME);
                    }
                } else {
                    printf("Couldn't open grid dataset for [%s]\n", CELL_DATASET_NAME);
                }
            
            } else {
                printf("Couldn't open group [%s]\n", GRIDGROUP_NAME);
            }
        }
    } else {
        printf("Couldn't open population group [%s]\n", POPGROUP_NAME);
    }
  
    return iResult;
}

//----------------------------------------------------------------------------
// createSCellAgentType
//
hid_t QDF2PopSnap::createSCellAgentType() {
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
// fillSimpleSnapData
//
int QDF2PopSnap::fillSnapData(char *pBuffer) {
    int iResult = 0;


    // buffers for data
    int  *pGridData = new int[BUF_SIZE];
    int  *pPopData  = new int[BUF_SIZE];
    memset(pGridData, 170, BUF_SIZE*sizeof(int));
    memset(pPopData,  85, BUF_SIZE*sizeof(int));
    
    std::map<int, int> mCounts;

    hsize_t dimsmPop   = BUF_SIZE;

    // get sizee
    hsize_t dims = 0;
    hsize_t maxdims = 0;
    H5Sget_simple_extent_dims(m_hDataSpacePop, &dims, &maxdims);
    printf("Found num agents: %lld\n", dims);

    hid_t hAgentDataType = H5Tcreate(H5T_COMPOUND, sizeof(int));
    herr_t status = H5Tinsert(hAgentDataType, "CellID", 0, H5T_NATIVE_INT);
    if (status < 0) {
        printf("Couldn't select field [CellID] from dataspace\n");
        iResult = -1;
    }
    hid_t hMemSpacePop = H5Screate_simple (1, &dimsmPop, NULL); 

    hsize_t offsetPop  = 0;

    while ((dims > 0) && (iResult ==0)) {
        // change Mem data space if necessary
        hsize_t countPop  = ((dims < BUF_SIZE)?dims:BUF_SIZE);
        if (countPop != dimsmPop) {
            qdf_closeDataSpace(hMemSpacePop); 
            dimsmPop  = countPop;
            hMemSpacePop = H5Screate_simple (1, &dimsmPop, NULL); 
        }
        // step size when going through data (stride =2: use every second element)
        hsize_t stride = 1;
        hsize_t block  = 1;



        //        printf("pop next bit %lld from %lld\n", countPop, offsetPop);
        status = H5Sselect_hyperslab(m_hDataSpacePop, H5S_SELECT_SET, 
                                     &offsetPop, &stride, &countPop, &block);
        if (status >= 0) {
            status = H5Dread(m_hDataSetPop, hAgentDataType, hMemSpacePop,
                             m_hDataSpacePop, H5P_DEFAULT, pPopData);

            if (status >= 0) {
                for (uint i = 0; i < countPop; i++) {
                    mCounts[pPopData[i]]++;
                }
                dims -= countPop;
                offsetPop += countPop;

            } else {
                printf("reading from p0p hyperslab failed\n");
                iResult = -1;
            }
        } else {
            printf("Hyperslab selection for pop failed\n");
            iResult = -1;
        }
    }
    printf("actually collected %zd counts\n", mCounts.size());
    qdf_closeDataSpace(hMemSpacePop); 

    if (iResult == 0) {
        /*
        std::map<int, int>::const_iterator it0;
        for (it0 = mCounts.begin(); it0 != mCounts.end(); ++it0) {
            printf("%d -> %d\n", it0->first, it0->second);
        }
        */

        dims = m_iNumCells;
        hsize_t offsetGrid  = 0;
        hsize_t dimsmGrid   = BUF_SIZE;
        hid_t hCellDataType = H5Tcreate(H5T_COMPOUND, sizeof(int));
        status = H5Tinsert(hCellDataType, "CellID", 0, H5T_NATIVE_INT);
        if (status < 0) {
            printf("Couldn't select field [CellID] from dataspace\n");
            iResult = -1;
        }

        hid_t hMemSpaceGrid = H5Screate_simple (1, &dimsmGrid, NULL); 
        
        char *pCur = pBuffer;
        
        while ((dims > 0) && (iResult ==0)) {
            // change Mem data space if necessary
            hsize_t countGrid = ((dims < BUF_SIZE)?dims:BUF_SIZE);
            if (countGrid != dimsmGrid) {
                qdf_closeDataSpace(hMemSpaceGrid); 
                dimsmGrid  = countGrid;
                hMemSpaceGrid = H5Screate_simple (1, &dimsmGrid, NULL); 
            }
            //    printf("next bit %lld from %lld\n", countGrid, offsetGrid);
            
            
            // step size when going through data (stride =2: use every second element)
            hsize_t stride = 1;
            hsize_t block  = 1;
            
            
            
            herr_t status = H5Sselect_hyperslab(m_hDataSpaceGrid, H5S_SELECT_SET, 
                                                &offsetGrid, &stride, &countGrid, &block);
            if (status >= 0) {
                status = H5Dread(m_hDataSetGrid, hCellDataType, hMemSpaceGrid,
                                 m_hDataSpaceGrid, H5P_DEFAULT, pGridData);
                
                if (status >= 0) {
                    for (uint i = 0; i < countGrid; i++) {
                        pCur = putMem(pCur, &(pGridData[i]), sizeof(int));
                        std::map<int, int>::const_iterator it = mCounts.find(pGridData[i]);
                        double dC = (it !=mCounts.end())?it->second:0;
                        
                        if (dC != 0) {
                            printf("%d->%f\n", pGridData[i], dC);
                        }
                        
                        pCur = putMem(pCur, &dC, sizeof(double));
                        
                    }
                    dims -= countGrid;
                    offsetGrid += countGrid;
                    
                } else {
                    printf("reading from grid hyperslab failed\n");
                    iResult = -1;
                }
            } else {
                printf("Hyperslab selection for grid failed\n");
                iResult = -1;
            }
        }
        qdf_closeDataSpace(hMemSpacePop); 
    }
    printf("blargblorgblirg\n");
    return iResult;
}
