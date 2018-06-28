#include <stdio.h>
#include <string.h>
#include <hdf5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "QDFVisitUtils.h"

#define ROOT_ATTR_VALUE  "a QHG data file"


//----------------------------------------------------------------------------
// close
//
void qdf_closeFile(hid_t hFile) {
    H5Fclose(hFile);
}



//----------------------------------------------------------------------------
// open
//
hid_t qdf_openFile(const char *pFileName) {
    int iResult = -1;

    H5E_auto2_t  hOldFunc;
    void *old_client_data;
    hid_t hErrorStack = H5E_DEFAULT;

    // remember previous settings
    H5Eget_auto(hErrorStack, &hOldFunc, &old_client_data);

    // Turn off error handling */
    H5Eset_auto(hErrorStack, NULL, NULL);


    hid_t hFile = H5P_DEFAULT;
    struct stat buf;
    iResult = stat(pFileName, &buf);
    if (iResult == 0) {
        hFile = H5Fopen(pFileName, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (true || H5Aexists(hFile, ROOT_ATTR_NAME)) {
            char sValue[128];
            iResult = qdf_extractSAttribute(hFile, ROOT_ATTR_NAME, 128, sValue);
            if (iResult != 0) {
                qdf_closeFile(hFile);
                hFile = H5P_DEFAULT;
            }
        }
    }
    // Restore previous error handler
    H5Eset_auto(hErrorStack, hOldFunc, old_client_data);

    return hFile;
}


//----------------------------------------------------------------------------
// qdf_openGroup
//
hid_t qdf_openGroup(hid_t hFile, const char *pGroupName, bool bForceCheck) {
    hid_t hGroup = H5P_DEFAULT;
    if ((!bForceCheck) || H5Lexists(hFile, pGroupName, H5P_DEFAULT)) {
        hGroup = H5Gopen(hFile, pGroupName, H5P_DEFAULT);
    }
    return hGroup;
}


//----------------------------------------------------------------------------
// qdf_closeGroup
//
void  qdf_closeGroup(hid_t hGroup) {
    if (hGroup != H5P_DEFAULT) {
        H5Gclose(hGroup);
    }
}

//----------------------------------------------------------------------------
// qdf_closeDataSet
//
void  qdf_closeDataSet(hid_t hDataSet) {
    if (hDataSet != H5P_DEFAULT) {
        H5Dclose(hDataSet);
    }
}

//----------------------------------------------------------------------------
// qdf_closeDataSpace
//
void  qdf_closeDataSpace(hid_t hDataSpace) {
    if (hDataSpace != H5P_DEFAULT) {
        H5Sclose(hDataSpace);
    }
}

//----------------------------------------------------------------------------
// qdf_closeDataType
//
void  qdf_closeDataType(hid_t hDataType) {
    if (hDataType != H5P_DEFAULT) {
        H5Tclose(hDataType);
    }
}

//----------------------------------------------------------------------------
// qdf_closeAttribute
//
void  qdf_closeAttribute(hid_t hAttribute) {
    if (hAttribute != H5P_DEFAULT) {
        H5Aclose(hAttribute);
    }
}



//----------------------------------------------------------------------------
// qdf_extractAttribute
//    char
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, char *cValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if ((rank == 1) && (dims[0] == iNum)) {
        
            status = H5Aread(hAttribute, H5T_NATIVE_CHAR, cValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read char attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// qdf_extractAttribute
//    int
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, int *iValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);
        
        if ((rank == 1) && (dims[0] == iNum)) {
            
            status = H5Aread(hAttribute, H5T_NATIVE_INT, iValue);
            
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read int attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractAttribute
//    uint
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, uint *iValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);
        
        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
            
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_INT32, iValue);
            
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read int attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractAttribute
//    long
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, long *lValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if ((rank == 1) && (dims[0] == iNum)) {
        
            status = H5Aread(hAttribute, H5T_NATIVE_LONG, lValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read long attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractAttribute
//    float
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, float *fValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if ((rank == 1) && (dims[0] == iNum)) {
        
            status = H5Aread(hAttribute, H5T_NATIVE_FLOAT, fValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read float attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_extractAttribute
//    double
//
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, double *dValue) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, pName);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if ((rank == 1) && (dims[0] == iNum)) {
        
            status = H5Aread(hAttribute, H5T_NATIVE_DOUBLE, dValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                printf("read double attribute err\n");
            } 
        } else {
            printf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractSAttribute
//    string
//
int qdf_extractSAttribute(hid_t hLoc, const char *pName, uint iNum, char *sValue) {
    int iResult = -1;

        
    if (H5Aexists(hLoc, pName)) {
        hid_t hAttribute = H5Aopen_by_name(hLoc, ".", pName, H5P_DEFAULT, H5P_DEFAULT);
        hid_t atype  = H5Aget_type(hAttribute);
        hsize_t size = H5Tget_size (atype);
        char *pString = new char[size+1];

        herr_t status = H5Aread(hAttribute, atype, pString);
        if (status >= 0) {
            strncpy(sValue, pString, iNum);
            sValue[iNum-1] = '\0';
            iResult = 0;
        }
        qdf_closeAttribute(hAttribute);
        delete[] pString;
    } else {
        printf("Attribute [%s] does not exist\n", pName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   double
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, double *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   float
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, float *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   int
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, int *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   ushort
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, ushort *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   char
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, char *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   uchar
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, unsigned char *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   double**
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {
    herr_t status = H5P_DEFAULT;
    
    hsize_t iOffset = 0;
    hsize_t iCount  = iSize;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
    hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
    hid_t hDataSet = H5Dopen2(hGroup, pName, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);
    for(int i = 0; i < iNumArr; i++) {
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData[i]);


        iOffset += iSize;
    }


    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSpace(hMemSpace);
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   double**
//
int qdf_readArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {
    herr_t status = H5P_DEFAULT;
    

    hsize_t iCount[2];
    iCount[0] = iSize;  
    iCount[1] = 1;

    hsize_t iOffset[2];
    iOffset[0] = 0;
    iOffset[1] = 0;
    hsize_t iStride[2] = {1, 1};
    hsize_t iBlock[2]  = {1, 1};
   
    hsize_t dims = iSize;

    hid_t hMemSpace  = H5Screate_simple (1, &dims, NULL); 
    hid_t hDataSet   = H5Dopen2(hGroup, pName, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);
    
    for(int i = 0; i < iNumArr; i++) {
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     iOffset, iStride, iCount, iBlock);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData[i]);

        iOffset[1]++;
    }


    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSpace(hMemSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArraySlabT
//   double
//   
int qdf_readArraySlabT(hid_t hGroup, const char *pName, int iCount, int iOffset, int iStride, int iBlock, double *pData) {
    herr_t status = H5P_DEFAULT;
    
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {

        hsize_t iSCount = iCount;  
        hsize_t iSOffset = iBlock * iOffset;
        hsize_t iSStride = iBlock * iStride;
        hsize_t iSBlock  = iBlock;

        hsize_t iMOffset = 0;
        hsize_t iMCount  = iBlock*iCount;
        
        hid_t hMemSpace  = H5Screate_simple (1, &iMCount, NULL); 
        status = H5Sselect_hyperslab(hMemSpace, H5S_SELECT_SET, 
                                     &iMOffset, NULL, &iMCount, NULL);

        hid_t hDataSet   = H5Dopen2(hGroup, pName, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iSOffset, &iSStride, &iSCount, &iSBlock);

        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData);
     
        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSpace(hMemSpace);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


