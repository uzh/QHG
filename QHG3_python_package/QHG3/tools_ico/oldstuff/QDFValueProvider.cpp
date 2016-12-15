#include <stdio.h>
#include <string.h>

#include <map>
#include <string>
#include <algorithm>

#include "strutils.h"
#include "QDFUtils.h"
#include "QDFNodes.h"
#include "QDFArraySniffer.h"
#include "QDFValueProvider.h"


//-----------------------------------------------------------------------------
//  isNULL
//   predicate function  for use in prune
//
bool isNULL (const std::pair<std::string, qdfdata *> &e) {
  return (e.second == NULL);
}


//-----------------------------------------------------------------------------
//  extractArray
//
template<typename T>
qdfdata *QDFValueProvider::extractArray(hid_t hGroup, const char *pDataSetName, int iIndex, T t) {
    int iResult = -1;
    qdfdata *pqdfdata = NULL;
    double *pdData = NULL;

    T *ptData = new T[m_iNumCells];

    // read ptData from QDF 
    if (iIndex < 0) {
        iResult = qdf_readArray(hGroup, pDataSetName, m_iNumCells, ptData);
    } else {
        printf("Slabbing for index %d of [%s]\n", iIndex, pDataSetName);
        // use slab to get correct location
        hsize_t dims = m_iNumCells;
        hid_t hMemSpace  = H5Screate_simple (1, &dims, NULL); 
        hid_t hDataSet   = H5Dopen2(hGroup, pDataSetName, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        hsize_t iCount[2];
        iCount[0] = m_iNumCells;  
        iCount[1] = 1;

        hsize_t iOffset[2];
        iOffset[0] = 0;
        iOffset[1] = iIndex;
        hsize_t iStride[2] = {1, 1};
        hsize_t iBlock[2]  = {1, 1};

        herr_t status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     iOffset, iStride, iCount, iBlock);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, ptData);
        iResult = (status >= 0)?0:-1;
    }
    // if read ok
    if (iResult == 0) {
        pqdfdata = new qdfdata;
        double dMin = dPosInf;
        double dMax = dNegInf;
        pdData = new double[m_iNumCells];
    //   loop through ptData and fill pdData
        for (int i = 0; i < m_iNumCells; i++) {
            pdData[i] = (double)ptData[i];
            if (pdData[i] < dMin) {
                dMin = pdData[i];
            }
            if (pdData[i] > dMax) {
                dMax = pdData[i];
            }
        }
        pqdfdata->pdData = pdData;
        pqdfdata->dMinVal = dMin;
        pqdfdata->dMaxVal = dMax;
    } 

    delete[] ptData;
    //    return pdData;
    return pqdfdata;
}


//-----------------------------------------------------------------------------
//  constructor
//
QDFValueProvider::QDFValueProvider() 
    : m_hCurFile(H5P_DEFAULT),
      m_iNumCells(0),
      m_pQN(NULL),
      m_sCurDataSet("") {
}


//-----------------------------------------------------------------------------
//  destructor
//
QDFValueProvider::~QDFValueProvider() {
    std::map<std::string, qdfdata *>::iterator it;
    for (it = m_mDataSets.begin(); it != m_mDataSets.end(); ++it) {
        if (it->second != NULL) {
            if (it->second->pdData != NULL) {
                delete[] it->second->pdData;
            }
            delete it->second;
        }
    }

    if  (m_hCurFile != H5P_DEFAULT) {
        qdf_closeFile(m_hCurFile);
    }

    if (m_pQN != NULL) {
        delete m_pQN;
    }
}



//-----------------------------------------------------------------------------
//  prune
//    erase all elements whose array is NULL
//
void QDFValueProvider::prune() {    
    std::map<std::string, qdfdata *>::iterator it = m_mDataSets.begin();
    while ((it = std::find_if(it, m_mDataSets.end(), isNULL)) != m_mDataSets.end()) {
        m_mDataSets.erase(it++);
    }
}


//-----------------------------------------------------------------------------
//  createValueProvider
//
ValueProvider *QDFValueProvider::createValueProvider(const char *pQDFFile) {
    ValueProvider *pVP = new QDFValueProvider();
    int iResult = pVP->init(pQDFFile);
    if (iResult != 0) {
        delete pVP;
        pVP = NULL;
    }
    return pVP;
}

//-----------------------------------------------------------------------------
//  init
//
int QDFValueProvider::init(const char *pQDFFile) {
    int iResult = 0;

    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile != H5P_DEFAULT) {
        // hFile will be assigned to m_hCurFile
        m_pQN = QDFNodes::createInstance(hFile);
        if (m_pQN != NULL) {
            if (m_iNumCells == 0) {
                m_iNumCells = m_pQN->getNumCells();
            }
            // now check if you can extract some stuff from here
            iResult = addFile(hFile);
            m_iDataFileType = DATA_FILE_TYPE_LIST;

        } else {
            iResult = -1;
            printf("Couldn't create QDFNodes from file [%s]\n", pQDFFile);
        }
    } else {
        iResult = -1;
        printf("Couldn't open QDF file [%s]\n", pQDFFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  getValue
//    assumption:
//       data for current dataset is loaded
//
double QDFValueProvider::getValue(gridtype lID) {
    double dVal = dNaN;
    
    int iIndex = m_pQN->getIndexForNode(lID);
    if (iIndex >= 0) {
        dVal = m_mDataSets[m_sCurDataSet]->pdData[iIndex];
    }

    return dVal;
}



//-----------------------------------------------------------------------------
//  addFile
//
int QDFValueProvider::addFile(const char *pQDFFile) {
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile != H5P_DEFAULT) {
        iResult = addFile(hFile);
    } else {
        iResult = -1;
        printf("Couldn't open QDF data file [%s]\n", pQDFFile);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  addFile
//
int QDFValueProvider::addFile(hid_t hFile) {
    int iResult = -1;
    if  (m_hCurFile != H5P_DEFAULT) {
        qdf_closeFile(m_hCurFile);
    }
    m_hCurFile = hFile;

    QDFArraySniffer *pQAS = new QDFArraySniffer(m_hCurFile, m_iNumCells);
    if (pQAS != NULL) {
        iResult = pQAS->scan();
        if (iResult == 0) {
            // remove all empty arrays from list before adding new items
            prune();

            // now add new stuff ("names"); data NULL
            std::vector<std::string> &vNewItems = pQAS->getItems();
            for (uint i = 0; i < vNewItems.size(); i++) {
                m_mDataSets[vNewItems[i]] = NULL;
            }
        } else {
            iResult = -1;
            printf("Couldn't scan QDF data file for arrays\n");
        }
        delete pQAS;
    } else {
        iResult = -1;
        printf("Couldn't create array sniffer for file\n");
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  extractPopArray
//
qdfdata *QDFValueProvider::extractCellDataArray(hid_t hGroup, const char *pDataSetName) {
    qdfdata *pqdfData = NULL;
    hid_t hDataSet =  H5Dopen(hGroup, CELL_DATASET_NAME, H5P_DEFAULT);
    if (hDataSet >= 0) {
        int *pCellIDs = new int[m_iNumCells];
        hid_t tcell = H5Tcreate(H5T_COMPOUND, sizeof(int));
        herr_t status = H5Tinsert(tcell, GRID_DS_CELL_ID, 0, H5T_NATIVE_INT);
        if (status >= 0) {
            status = H5Dread(hDataSet, tcell, H5S_ALL, H5S_ALL, H5P_DEFAULT, pCellIDs);

            if (status >= 0) {
                pqdfData = new qdfdata;
                pqdfData->pdData = new double[m_iNumCells];
                pqdfData->dMinVal = dPosInf;
                pqdfData->dMaxVal = dNegInf;
                for (int i = 0; i < m_iNumCells; i++) {
                    double dVal = pCellIDs[i];
                    pqdfData->pdData[pCellIDs[i]] = dVal;
                    if (dVal < pqdfData->dMinVal) {
                        pqdfData->dMinVal = dVal;
                    }
                    if (dVal > pqdfData->dMaxVal) {
                        pqdfData->dMaxVal = dVal;
                    }
                }
            } else {
                printf("Couldn't read data\n");
            }
            qdf_closeDataType(tcell);
        } else {
            printf("Couldn't create datatype\n");
        }
        delete[] pCellIDs;
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Couldn't open dataset [%s]\n", CELL_DATASET_NAME);
    }

    return pqdfData;
}

//-----------------------------------------------------------------------------
//  extractPopArray
//
qdfdata *QDFValueProvider::extractPopArray(hid_t hGroup, const char *pPopName) {
    qdfdata *pqdfData = NULL;
    hid_t hPop = qdf_openGroup(hGroup, pPopName);
    if (hPop >= 0) {
        hid_t hDataSet =  H5Dopen(hPop, AGENT_DATASET_NAME, H5P_DEFAULT);
        if (hDataSet >= 0) {
            hid_t hDataSpace = H5Dget_space(hDataSet);

            hsize_t dims[3];
            hsize_t maxdims = 3;
            int iNumDims = H5Sget_simple_extent_dims(hDataSpace, dims, &maxdims);
            if (iNumDims == 1) {
                // allocate array of ints
                int *pCellIDs = new int[dims[0]];
                // initialize to zero
                // load only CellID of op data

                hid_t tcell = H5Tcreate(H5T_COMPOUND, sizeof(int));
                herr_t status = H5Tinsert(tcell, GRID_DS_CELL_ID, 0, H5T_NATIVE_INT);
                if (status >= 0) {
                    status = H5Dread(hDataSet, tcell, H5S_ALL, H5S_ALL, H5P_DEFAULT, pCellIDs);
                    if (status >= 0) {
                        pqdfData = new qdfdata;
                        pqdfData->pdData = new double[m_iNumCells];
                        memset(pqdfData->pdData, 0, m_iNumCells*sizeof(double));
                        for (uint i = 0; i < dims[0]; i++) {
                            if ((pCellIDs[i] < 0) ||(pCellIDs[i] >= m_iNumCells)) {
                                printf("bad index at %d: %d\n", i, pCellIDs[i]);
                            }
                            pqdfData->pdData[pCellIDs[i]]++;
                        }
                        pqdfData->dMinVal = dPosInf;
                        pqdfData->dMaxVal = dNegInf;
                        //   loop through ptData and set minmax
                        for (int i = 0; i < m_iNumCells; i++) {
                            double dVal = pqdfData->pdData[i];
                            if (dVal < pqdfData->dMinVal) {
                                pqdfData->dMinVal = dVal;
                            }
                            if (dVal > pqdfData->dMaxVal) {
                                pqdfData->dMaxVal = dVal;
                            }
                        }

                    } else {
                        printf("Couldn't read data\n");
                    }
                    qdf_closeDataType(tcell);
                } else {
                    printf("Couldn't create datatype\n");
                }
                delete[] pCellIDs;
            } else {
                printf("Data set has bad number of dimensions [%d]\n", iNumDims);
            }
            qdf_closeDataSpace(hDataSpace);
            qdf_closeDataSet(hDataSet);
        } else {
            printf("Couldn't open dataset [%s]\n", AGENT_DATASET_NAME);
        }
        qdf_closeGroup(hPop);
    } else {
        printf("Couldn't open population subgroup [%s]\n", pPopName);
    }
    return pqdfData;
}

//-----------------------------------------------------------------------------
//  extractStandardArray
//
qdfdata *QDFValueProvider::extractStandardArray(hid_t hGroup, const char *pDataSet, int iIndex) {
    qdfdata *pqdfData = NULL;
    hid_t hDataSet = H5Dopen(hGroup, pDataSet, H5P_DEFAULT);
    if (hDataSet >= 0) {
        int iType = qdf_getDataType(hDataSet);
        switch (iType) {
        case DS_TYPE_CHAR:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (char)0);
            break;
        case DS_TYPE_SHORT:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (short)0);
            break;
        case DS_TYPE_INT:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (int)0);
            break;
        case DS_TYPE_LONG:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (long)0);
            break;
        case DS_TYPE_LLONG:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (llong)0);
            break;
        case DS_TYPE_UCHAR:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (uchar)0);
            break;
        case DS_TYPE_USHORT:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (ushort)0);
            break;
        case DS_TYPE_UINT:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (uint)0);
            break;
        case DS_TYPE_ULONG:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (ulong)0);
            break;
        case DS_TYPE_ULLONG:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (ullong)0);
            break;
        case DS_TYPE_FLOAT:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (float)0.0);
            break;
        case DS_TYPE_DOUBLE:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (double)0.0);
            break;
        case DS_TYPE_LDOUBLE:
            pqdfData = extractArray(hGroup, pDataSet, iIndex, (ldouble) 0.0);
            break;
        default:
            pqdfData = NULL;
        }
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Couldn't open dataset [%s]\n", pDataSet);
    }
    return pqdfData;
}

//-----------------------------------------------------------------------------
//  createValueProvider
//
int QDFValueProvider::setMode(const char *pMode) {
    int iResult = -1;
    printf("Setting mode [%s]\n", pMode);
    std::map<std::string, qdfdata *>::iterator it = m_mDataSets.find(pMode);
    if (it != m_mDataSets.end()) {
        if (it->second == NULL) {
            // load the data:
            char sGroup[1024];
            strcpy(sGroup, pMode);
            
            char *pDataSet = strchr(sGroup, '/');
            if (pDataSet != NULL) {
                *pDataSet = '\0';
                pDataSet++;
                int iIndex = -1;
                char *pNumber = strchr(pDataSet, ':');
                if (pNumber != NULL) {
                    *pNumber = '\0';
                    pNumber++;
                    if (!strToNum(pNumber, &iIndex)) {
                        iIndex = -1;
                    }
                }


                // open the group (identified by pMode)
                hid_t hGroup = qdf_openGroup(m_hCurFile, sGroup);
                if (hGroup >= 0) {
                    qdfdata *pqdfData = NULL;
                    if (strcmp(sGroup, POPGROUP_NAME)== 0) {
                        pqdfData = extractPopArray(hGroup, pDataSet);
                    } else if (strcmp(sGroup, GRIDGROUP_NAME)== 0) {
                        pqdfData = extractCellDataArray(hGroup, pDataSet);
                    } else {
                        pqdfData = extractStandardArray(hGroup, pDataSet, iIndex);
                    }

                    if (pqdfData != NULL) {
                        pqdfData->sFile = "?";
                        printf("Setting [%s] to 0x%p\n", pMode, pqdfData->pdData);fflush(stdout);
                        m_mDataSets[pMode]= pqdfData;
                        iResult = 0;
                    } else {
                        printf("Couldn't create array for dataset [%s]\n", pDataSet);
                    }
                    qdf_closeGroup(hGroup);
                } else {
                    printf("Didn't find group [%s] in file\n", sGroup);
                    iResult = -1;
                }
            
            } else {
                printf("path to dataset invalid [%s]\n", pMode);
                iResult = -1;
            }

        } else {
            // already has data
            iResult = 0;
        }
    } else {
        // "new" path
        iResult = -1;
    }
    if (iResult == 0) {
        m_sCurDataSet = pMode;
        m_dMinData = m_mDataSets[pMode]->dMinVal;
        m_dMaxData = m_mDataSets[pMode]->dMaxVal;
    }
    return iResult;
}


void QDFValueProvider::show() {
    printf("---------------------------\n");
    printf("QDFValuProvider %p\n", this);
    printf("NumCells: %d\n", m_iNumCells);
    printf("Items:\n");
    std::map<std::string, qdfdata *>::iterator it;
    for (it = m_mDataSets.begin(); it != m_mDataSets.end(); ++it) {
        printf("  %s -> %p\n", it->first.c_str(), (it->second==NULL)?NULL:it->second->pdData);
    }
    printf("CurDataSet: [%s]\n", m_sCurDataSet.c_str());
    printf("---------------------------\n");

}
