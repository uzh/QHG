#include <stdio.h>

#include <hdf5.h>
#include <map>

#include "QDFUtils.h"
#include "QDFNodes.h"


//-----------------------------------------------------------------------------
// constructor
//
QDFNodes::QDFNodes()
    : m_pNodes(NULL) {
}

//-----------------------------------------------------------------------------
// destructor
//
QDFNodes::~QDFNodes() {
    if (m_pNodes != NULL) {
        delete[] m_pNodes;
    }
}


//-----------------------------------------------------------------------------
// createInstance
//
QDFNodes *QDFNodes::createInstance(const char *pQDFFile) {
    QDFNodes *pQN = new QDFNodes();
    int iResult = pQN->init(pQDFFile);
    if (iResult != 0) {
        delete pQN;
        pQN = NULL;
    }
    return pQN;
}


//-----------------------------------------------------------------------------
// createInstance
//
QDFNodes *QDFNodes::createInstance(hid_t hQDFFile) {
    QDFNodes *pQN = new QDFNodes();
    int iResult = pQN->init(hQDFFile);
    if (iResult != 0) {
        delete pQN;
        pQN = NULL;
    }
    return pQN;
}


//-----------------------------------------------------------------------------
// init
//
int QDFNodes::init(const char *pQDFFile) {
    int iResult = -1;
    
    hid_t hQDFFile = qdf_openFile(pQDFFile);
    if (hQDFFile >= 0) {
        iResult = init(hQDFFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// init
//
int QDFNodes::init(hid_t hQDFFile) {
    int iResult = -1;
    int iNumCells = 0;

    hid_t hGridGroup = qdf_openGroup(hQDFFile, GRIDGROUP_NAME);
    if (hGridGroup >= 0) {
        iResult = qdf_extractAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, &iNumCells);
        if (iResult == 0) {
            m_pNodes = new int[iNumCells];
            
            hid_t hDataSet = H5Dopen(hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
            if (hDataSet >= 0) {
                hid_t hType = H5Tcreate(H5T_COMPOUND, sizeof(int));
                herr_t status = H5Tinsert(hType, GRID_DS_CELL_ID, 0, H5T_NATIVE_INT);
                if (status >= 0) {
                    status = H5Dread(hDataSet, hType, H5S_ALL, H5S_ALL, H5P_DEFAULT, m_pNodes);
                    if (status >= 0) {
                        for (int i = 0; i < iNumCells; i++) {
                            m_mNode2Index[m_pNodes[i]] = i;
                        }
                        iResult = 0;
                    } else {
                        iResult = -1;
                        printf("Couldn't read data from [%s]\n", GRID_DS_CELL_ID);
                    }
                    qdf_closeDataType(hType);
                } else {
                    iResult = -1;
                    printf("Couldn't create datatype\n");
                }
                qdf_closeDataSet(hDataSet);
            } else {
                iResult = -1;
                printf("Couldn't open dataset [%s]\n", CELL_DATASET_NAME);
            }
        } else {
            iResult = -1;
            printf("Couldn't get attribute [%s]\n", GRID_ATTR_NUM_CELLS);
        }
        qdf_closeGroup(hGridGroup);
    } else {
        iResult = -1;
        printf("Couldn't open group [%s]\n", GRIDGROUP_NAME);
    }
    
    return iResult;
}

int QDFNodes::getIndexForNode(int iNode) {
    int iIndex = -1;
    std::map<int, int>::const_iterator it = m_mNode2Index.find(iNode);
    if (it != m_mNode2Index.end()) {
        iIndex = it->second;
    }
    return iIndex;
}
