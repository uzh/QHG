#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#include "QDFUtils.h"
#include "QDFArray.h"


//----------------------------------------------------------------------------
// create
//
QDFArray *QDFArray::create(const char *pQDFFile) {
    QDFArray *pQDFA = new QDFArray();
    int iResult = pQDFA->init(pQDFFile);
    if (iResult != 0) {
        delete pQDFA;
        pQDFA = NULL;
    }
    return pQDFA;
}


//----------------------------------------------------------------------------
// constructor
//
QDFArray::QDFArray() 
    : m_hFile(H5P_DEFAULT),
      m_hRoot(H5P_DEFAULT),
      m_hDataSet(H5P_DEFAULT),
      m_hDataSpace(H5P_DEFAULT),
      m_hMemSpace(H5P_DEFAULT),
      m_hDataType(H5P_DEFAULT),
      m_iArraySize(0),
      m_offset(0),
      m_count(0),
      m_stride(0),
      m_block(0),
      m_memsize(0),
      m_iBufSize(0),
      m_bDeleteDataType(true) {

    m_vhGroups.clear();
}


//----------------------------------------------------------------------------
// destructor
//
QDFArray::~QDFArray() {

    closeArray();
    if (m_hRoot > 0) {
         qdf_closeGroup(m_hRoot);
    }
    if (m_hFile > 0) {
         qdf_closeFile(m_hFile);
    }

}


//----------------------------------------------------------------------------
// closeArray
//
void QDFArray::closeArray() {
    if (m_bDeleteDataType && (m_hDataType != H5P_DEFAULT)) {
        qdf_closeDataType(m_hDataType);
        m_hDataType = H5P_DEFAULT;
    }
    if (m_hMemSpace != H5P_DEFAULT) {
        qdf_closeDataSpace(m_hMemSpace);
        m_hMemSpace = H5P_DEFAULT;
    }
    if (m_hDataSpace != H5P_DEFAULT) {
        qdf_closeDataSpace(m_hDataSpace);
        m_hDataSpace = H5P_DEFAULT;
    }
    if (m_hDataSet != H5P_DEFAULT) {
        qdf_closeDataSet(m_hDataSet);
        m_hDataSet = H5P_DEFAULT;
    }
    for (uint i = 0; i < m_vhGroups.size(); i++) {
        if (m_vhGroups[i] != H5P_DEFAULT) {
            qdf_closeGroup(m_vhGroups[i]);
        }
    }
    m_vhGroups.clear();
 
}


//----------------------------------------------------------------------------
// init
//
int QDFArray::init(const char *pQDFFile) {
    int iResult = -1;
    
    m_hFile = qdf_openFile(pQDFFile);
    if (m_hFile > 0) {
        iResult = 0;
        
        //        hid_t hRoot=qdf_openGroup(m_hFile, "/", false);
        char sValue[128];
        iResult = qdf_extractSAttribute(m_hFile, ROOT_TIME_NAME, 128, sValue);
        if (iResult == 0) {
            char *pEnd;
            m_fTimeStep = strtof(sValue, &pEnd);
            if (*pEnd == '\0') {
                iResult = 0;
            } else{
                printf("Time step is not a float [%s]\n", sValue);
            }
        } else {
            printf("No time step found\n");
        }
    } else {
        printf("QDFArray couldn't open [%s]\n", pQDFFile);
        m_hFile = H5P_DEFAULT;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in group
//
int QDFArray::openArray(const char *pGroup, const char *pDataset) {
    int iResult = 0;
    std::vector<const char *> vGroups;
    vGroups.push_back(pGroup);
    iResult = openArray(vGroups, pDataset);
    return iResult;   
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in subgroup group2 of group1
//
int QDFArray::openArray(const char *pGroup1, const char *pGroup2, const char *pDataset) {
    int iResult = 0;
    std::vector<const char *> vGroups;
    vGroups.push_back(pGroup1);
    vGroups.push_back(pGroup2);
    iResult = openArray(vGroups, pDataset);
    return iResult;   
}


//----------------------------------------------------------------------------
// openArray
//  open dataset given in path ("group1/.../groupN/dataset")
//
int QDFArray::openArray(const char *pPathToDataset) {
    int iResult = -1;

    if (strlen(pPathToDataset) > 0) {

        char *pPath = new char[strlen(pPathToDataset)+1];
        strcpy(pPath, pPathToDataset);

        // extract the elements of the path
        std::vector<const char *> vGroups;
        char *pEnd;
        char *p = strtok_r(pPath, "/", &pEnd);
        while (p != NULL) {
            vGroups.push_back(p);
            p = strtok_r(NULL, "/", &pEnd);
            
        }
        const char *pDataSet = vGroups.back();
        vGroups.pop_back();
        iResult = openArray(vGroups, pDataSet);
        delete[] pPath;
    } else {
        printf("Empty path provided\n");
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in "vGroups[0]/.../vGroups[N]"
//
int QDFArray::openArray(std::vector<const char *> &vGroups, const char *pDataSet) {
    int iResult = 0;

    // open the groups (last element is the dataset)
    hid_t hPrev = m_hFile;
    for (uint i = 0; (iResult == 0) && (i < vGroups.size()); i++) {
        //@@        printf("opening group [%s]...\n", vGroups[i]);
        hid_t hGroup = qdf_openGroup(hPrev, vGroups[i]);
        if (hGroup > 0) {
            m_vhGroups.push_back(hGroup);
            hPrev = hGroup;
            //@@printf("OK\n");fflush(stdout);
        } else {
            iResult = -1;
            printf("failed\n");fflush(stdout);
        }
    }
    
    // open the dataset
    if (iResult == 0) {
        //@@        printf("Opening data set [%s]\n", pDataSet);
        m_hDataSet = qdf_openDataSet(m_vhGroups.back(), pDataSet);
        if (m_hDataSet > 0) {
            m_hDataSpace = H5Dget_space(m_hDataSet);
            if (m_hDataSpace >= 0) {
                hsize_t sdim[3];
                int iDims = H5Sget_simple_extent_dims(m_hDataSpace, sdim, NULL);
                if (iDims == 1) {
                    m_iArraySize = (uint)sdim[0];
                    iResult = 0;
                    
                } else {
                    printf("Not a one-dimensinal data set [%s]\n", pDataSet);
                    iResult = -1;
                }
            } else {
                printf("Couldn't get sata space for data set [%s]\n", pDataSet);
                iResult = -1;
            }
        } else {
            printf("Couldn't open dataset [%s] in group [%s]\n", vGroups.back(), pDataSet);
            iResult = -1;
        }
    }
    
    if (iResult != 0) {
        closeArray();
    }

    return iResult;
}


//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(int *pBuffer, int iSize, const char *pFieldName) {
    int iResult = -1;

    iResult = getFirstSlab(pBuffer, iSize, H5T_NATIVE_INT, pFieldName);
    
    return iResult;
}


//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(double *pBuffer, int iSize, const char *pFieldName) {
    int iResult = -1;
  
    iResult = getFirstSlab(pBuffer, iSize, H5T_NATIVE_DOUBLE, pFieldName);

    return iResult;
}

//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(long *pBuffer, int iSize, const char *pFieldName) {
    int iResult = -1;
  
    iResult = getFirstSlab(pBuffer, iSize, H5T_NATIVE_LONG, pFieldName);

    return iResult;
}


//----------------------------------------------------------------------------
// getFirstSlab
//
template<typename T>
int QDFArray::getFirstSlab(T *pBuffer, int iSize, hid_t hBaseType, const char *pFieldName) {
    int iResult = -1;
    
    m_memsize = iSize;
    if (m_iArraySize < m_memsize) {
        m_memsize = m_iArraySize;
    }
    m_hMemSpace = H5Screate_simple (1, &m_memsize, NULL); 
                    
    //@@    printf("Dataspace num elements: %lld\n",  H5Sget_simple_extent_npoints(m_hDataSpace));
    m_offset = 0;
    m_count  = m_memsize;  
                    
                    // step size when going through data (stride =2: use every second element)
    m_stride = 1;
    m_block  = 1;
    
    iResult = setDataType(pFieldName, hBaseType, sizeof(T));
    if (iResult == 0) {
        iResult = readSlab(pBuffer);
    } 
    
    return iResult;
}


//----------------------------------------------------------------------------
// getNextSlab
//
template<typename T>
int QDFArray::getNextSlab(T *pBuffer, int iSize) {
    int iResult = 0;
    
    hsize_t remaining = m_iArraySize - m_offset;
    if (remaining < m_memsize) {
        qdf_closeDataSpace(m_hMemSpace);
        m_memsize = remaining;
        m_hMemSpace = H5Screate_simple (1, &m_memsize, NULL); 
    }

    m_count = m_memsize;

    if (iResult == 0) {
        iResult = readSlab(pBuffer);
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// readSlab
//
template<typename T>
int QDFArray::readSlab(T *pBuffer) {
    int iResult = 0;

    herr_t status = H5Sselect_hyperslab(m_hDataSpace, H5S_SELECT_SET, 
                                        &m_offset, &m_stride, &m_count, &m_block);
        
    status = H5Dread(m_hDataSet, m_hDataType, m_hMemSpace,
                     m_hDataSpace, H5P_DEFAULT, pBuffer);
    if (status >= 0) {
        m_offset += m_count;
        iResult = (int)m_count;
    } else {
        printf("Error during read\n");
        iResult = -1;
    }
     
    return iResult;
}


//----------------------------------------------------------------------------
// setDataType
//
int QDFArray::setDataType(const char *pFieldName, hid_t hBaseType, int iSize) {
    int iResult = -1;

    herr_t status = H5P_DEFAULT;
    if (pFieldName != NULL) {
        hid_t hType = H5Dget_type(m_hDataSet); 
        int iR = H5Tget_member_index(hType, pFieldName);
        if (iR >=  0) {
            m_hDataType = H5Tcreate(H5T_COMPOUND, iSize);
            status = H5Tinsert(m_hDataType, pFieldName, 0, hBaseType);
            if (status >= 0) {
                iResult = 0;
            }
            m_bDeleteDataType = true;
        } else {
            printf("The field [%s]  does not exist in this Dataset\n", pFieldName);
        }
    } else {
        m_hDataType = hBaseType;
        m_bDeleteDataType = false;
        iResult = 0;
    }
    return iResult;
}
