#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "PolyLine.h"
#include "QDFUtils.h"
//#include "UnitConverter.h"

#define ROOT_ATTR_VALUE  "a QHG data file"


//----------------------------------------------------------------------------
// close
//
void qdf_closeFile(hid_t hFile) {
    if (hFile > 0) {
        H5Fclose(hFile);
    }
}



//----------------------------------------------------------------------------
// create
//
hid_t qdf_createFile(const char *pFileName, float fTime) {
    int iResult = -1;
    hid_t hFile = H5P_DEFAULT;

    H5E_auto2_t  hOldFunc;
    void *old_client_data;
    hid_t hErrorStack = H5Eget_current_stack();

    // remember previous settings
    H5Eget_auto(hErrorStack, &hOldFunc, &old_client_data);

    // Turn off error handling */
    H5Eset_auto(hErrorStack, NULL, NULL);



    hFile = H5Fcreate(pFileName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (hFile > 0) {
        //@@        printf("file opened\n");
        // prepare datatypace for attribute
        hid_t hRoot=qdf_opencreateGroup(hFile, "/", false);
    
        if (hRoot > 0) {
            //@@            printf("root ok\n");
            iResult = qdf_insertSAttribute(hRoot, ROOT_ATTR_NAME, ROOT_ATTR_VALUE);
            if (iResult == 0) {
                iResult = qdf_insertSAttribute(hRoot, ROOT_TIME_NAME, fTime);
            }
            if (iResult == 0) {
                qdf_closeGroup(hRoot);
            } else {
                qdf_closeFile(hFile);
                hFile = H5P_DEFAULT;
                printf("pflarz\n");
            } 
        }
    } else {
        printf("Couldn't create file [%s]\n", pFileName);
    }
    // Restore previous error handler
    H5Eset_auto(hErrorStack, hOldFunc, old_client_data);
  
    return hFile;
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
        if (hFile > 0) {
            if (true || H5Aexists(hFile, ROOT_ATTR_NAME)) {
                char sValue[128];
                iResult = qdf_extractSAttribute(hFile, ROOT_ATTR_NAME, 128, sValue);
                if (iResult != 0) {
                    qdf_closeFile(hFile);
                    hFile = H5P_DEFAULT;
                    printf("qdf_openFile: [%s] couldn't extract attr [%s]\n", pFileName, ROOT_ATTR_NAME);
                }
            } else {
                printf("qdf_openFile: [%s] does not have attr [%s]\n", pFileName, ROOT_ATTR_NAME);
            }
        } else { 
            printf("qdf_openFile: couldn't open [%s] in RDWR mode\n", pFileName);
        }
    } else {
        printf("qdf_openFile: [%s] doesn't exist\n", pFileName);
    }
    // Restore previous error handler
    H5Eset_auto(hErrorStack, hOldFunc, old_client_data);

    return hFile;
}

//----------------------------------------------------------------------------
// opencreate
//
hid_t qdf_opencreateFile(const char *pFileName, float fTime) {
    hid_t hFile = qdf_openFile(pFileName);
    if (hFile == H5P_DEFAULT) {
        hFile = qdf_createFile(pFileName, fTime);
    }
    return hFile;
}


//----------------------------------------------------------------------------
// qdf_createGroup
//
hid_t qdf_createGroup(hid_t hFile, const char *pGroupName) {
    hid_t hGroup = H5Gcreate(hFile, pGroupName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    return hGroup;
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
// qdf_opencreateGroup
//
 hid_t qdf_opencreateGroup(hid_t hFile, const char *pGroupName, bool bForceCheck) {
     hid_t hGroup = qdf_openGroup(hFile, pGroupName, bForceCheck);
     if (hGroup <= 0) {
        // does not exist: create
        hGroup = qdf_createGroup(hFile, pGroupName);
        if (hGroup > 0) {
            // success
            //@@            printf("group [%s] didn't exist: created\n", pGroupName);
        }
    }
    return hGroup;
}

//----------------------------------------------------------------------------
// qdf_closeGroup
//
void  qdf_closeGroup(hid_t hGroup) {
    if (hGroup > 0) {
        H5Gclose(hGroup);
    }
}

//----------------------------------------------------------------------------
// qdf_openDataSet
//
hid_t qdf_openDataSet(hid_t hGroup, const char *pDataSet, bool bForceCheck) {
    hid_t hDataSet = H5P_DEFAULT;
    if ((!bForceCheck) || H5Lexists(hGroup, pDataSet, H5P_DEFAULT)) {
        hDataSet = H5Dopen(hGroup, pDataSet, H5P_DEFAULT);
    }
    return hDataSet;
}

//----------------------------------------------------------------------------
// qdf_closeDataSet
//
void  qdf_closeDataSet(hid_t hDataSet) {
    if (hDataSet > 0) {
        H5Dclose(hDataSet);
    }
}

//----------------------------------------------------------------------------
// qdf_closeDataSpace
//
void  qdf_closeDataSpace(hid_t hDataSpace) {
    if (hDataSpace > 0) {
        H5Sclose(hDataSpace);
    }
}

//----------------------------------------------------------------------------
// qdf_closeDataType
//
void  qdf_closeDataType(hid_t hDataType) {
    if (hDataType > 0) {
        H5Tclose(hDataType);
    }
}

//----------------------------------------------------------------------------
// qdf_closeAttribute
//
void  qdf_closeAttribute(hid_t hAttribute) {
    if (hAttribute > 0) {
        H5Aclose(hAttribute);
    }
}



//----------------------------------------------------------------------------
// insertSAttribute
//    strings
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const char  *pValue) {
    int iResult = -1;
    hid_t hAttrMemType = H5Tcopy (H5T_C_S1);
    herr_t status = H5Tset_size (hAttrMemType, strlen(pValue)+1);
    hsize_t dims = 1;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 hAttrMemType, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);
 
    status = H5Awrite (hAttribute, hAttrMemType, pValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("Couldn't create root attribute\n");
    } 
    qdf_closeDataType(hAttrMemType);
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   char
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const char   cValue) {
    char sBuf[32];
    sprintf(sBuf, "%c", cValue);
    return qdf_insertSAttribute(hLoc, pName, sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   short
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const short  sValue)  {
    char sBuf[32];
    sprintf(sBuf, "%hd", sValue);
    return qdf_insertSAttribute(hLoc, pName, (const char *)sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   int
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const int    iValue)  {
    char sBuf[32];
    sprintf(sBuf, "%d", iValue);
    return qdf_insertSAttribute(hLoc, pName, sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   long
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const long   lValue)  {
    char sBuf[32];
    sprintf(sBuf, "%ld", lValue);
    return qdf_insertSAttribute(hLoc, pName, sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//  float
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const float  fValue)  {
    char sBuf[32];
    sprintf(sBuf, "%f", fValue);
    return qdf_insertSAttribute(hLoc, pName, sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   double
//
int qdf_insertSAttribute(hid_t hLoc, const char *pName, const double dValue)  {
    char sBuf[32];
    sprintf(sBuf, "%f", dValue);
    return qdf_insertSAttribute(hLoc, pName, sBuf);
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    char
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, char *cValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_CHAR, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_CHAR, cValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write char attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    ints
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, uint *sValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_INT32, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_INT32, sValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write int attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    ints
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, int *iValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_INT, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_INT, iValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write int attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    longs
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, long *lValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_LONG, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_LONG, lValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write long attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    floats
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, float *fValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_FLOAT, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_FLOAT, fValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write float attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}

//----------------------------------------------------------------------------
// qdf_insertAttribute
//    doubles
//
int qdf_insertAttribute(hid_t hLoc, const char *pName, int iNum, double *dValue) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, pName, 
                                 H5T_NATIVE_DOUBLE, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, H5T_NATIVE_DOUBLE, dValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        printf("write double attribute err\n");
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
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
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_CHAR, cValue);
        
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
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);
        
        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
            
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_INT, iValue);
            
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
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_LONG, lValue);
        
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
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_FLOAT, fValue);
        
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
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, H5T_NATIVE_DOUBLE, dValue);
        
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
// qdf_writeArray
//   char
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, char *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_CHAR, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   uchar
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, uchar *pData) {
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
// qdf_writeArray
//   uchar
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, uchar *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_UCHAR, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   short
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, short *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_writeArray
//   short
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, short *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_SHORT, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
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
// qdf_writeArray
//   ushort
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, ushort *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_USHORT, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
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
// qdf_writeArray
//   int
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, int *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_INT, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   uint
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, uint *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   uint
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, uint *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_UINT, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   long
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, long *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   long
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, long *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_LONG, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   ulong
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, ulong *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   ulong
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, ulong *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_ULONG, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   llong
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, llong *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   llong
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, llong *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_LLONG, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArray
//   ullong
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, ullong *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   ullong
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, ullong *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_ULLONG, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
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

        /*
        // unit conversion
        char sUnits[64];
        qdf_extractSAttribute(hDataSet, DS_ATTR_UNITS, 64, sUnits);
        size_t iTypeSize = H5Tget_size( H5Dget_type(hDataSet) );
        hssize_t nItems = H5Dget_storage_size(hDataSet)/iTypeSize; 
        UnitConverter::convert(sUnits, nItems, pData);
        */
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_writeArray
//   float
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, float *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_FLOAT, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
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

        /*
        // unit conversion
        char sUnits[64];
        qdf_extractSAttribute(hDataSet, DS_ATTR_UNITS, 64, sUnits);
        size_t iTypeSize = H5Tget_size( H5Dget_type(hDataSet) );
        hssize_t nItems = H5Dget_storage_size(hDataSet)/iTypeSize; 
        UnitConverter::convert(sUnits, nItems, pData);
        */
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   double
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, double *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_DOUBLE, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   ldouble
//
int qdf_readArray(hid_t hGroup, const char *pName, int iNum, ldouble *pData) {
    herr_t status = H5P_DEFAULT;
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hid_t hDataSet   = H5Dopen(hGroup, pName, H5P_DEFAULT);
        status = H5Dread(hDataSet, H5T_NATIVE_LDOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

        /*
        // unit conversion
        char sUnits[64];
        qdf_extractSAttribute(hDataSet, DS_ATTR_UNITS, 64, sUnits);
        size_t iTypeSize = H5Tget_size( H5Dget_type(hDataSet) );
        hssize_t nItems = H5Dget_storage_size(hDataSet)/iTypeSize; 
        UnitConverter::convert(sUnits, nItems, pData);
        */
        qdf_closeDataSet(hDataSet);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   ldouble
//
int qdf_writeArray(hid_t hGroup, const char *pName, int iNum, ldouble *pData) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName, H5T_NATIVE_LDOUBLE, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_LDOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   double**
//
int qdf_readArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {
    herr_t status = H5P_DEFAULT;
    
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
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

        /*
        // unit conversion
        char sUnits[64];
        qdf_extractSAttribute(hDataSet, DS_ATTR_UNITS, 64, sUnits);
        size_t iTypeSize = H5Tget_size( H5Dget_type(hDataSet) );
        hssize_t nItems = H5Dget_storage_size(hDataSet)/iTypeSize; 
        */
        for(int i = 0; i < iNumArr; i++) {
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         iOffset, iStride, iCount, iBlock);
            status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                             hDataSpace, H5P_DEFAULT, pData[i]);

            //        UnitConverter::convert(sUnits, nItems, pData[i]);

            iOffset[1]++;
        }


        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSpace(hMemSpace);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_writeArray
//   double**
//
int qdf_writeArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {
    herr_t status   = H5P_DEFAULT;

    hsize_t iCount[2];
    iCount[0] = iSize;
    iCount[1] = 1;
  
    hsize_t iOffset[2];
    iOffset[0] = 0;
    iOffset[1] = 0;

    hsize_t iStride[2] = {1, 1};
    hsize_t iBlock[2]  = {1, 1};

    hsize_t dims[2];
    dims[0] = iSize;
    dims[1] = iNumArr;

    hid_t hDataSpace = H5Screate_simple(2, dims, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName,  H5T_NATIVE_DOUBLE, hDataSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t hMemSpace  = H5Screate_simple (1, iCount, NULL); 

    for(int i = 0; i < iNumArr; i++) {
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     iOffset, iStride, iCount, iBlock);
        status = H5Dwrite(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData[i]);


        iOffset[1]++;
    }
    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSpace(hMemSpace);
    return (status >= 0)?0:-1;
}

/*
//----------------------------------------------------------------------------
// qdf_readArray
//   double**
//
int qdf_readArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {
    herr_t status = H5P_DEFAULT;
    
    hsize_t iOffset = 0;
    hsize_t iCount  = iSize;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
   
    hid_t hMemSpace  = H5Screate_simple (1, &iCount, NULL); 
    hid_t hDataSet   = H5Dopen2(hGroup, pName, H5P_DEFAULT);
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
// qdf_writeArray
//   double**
//
int qdf_writeArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData) {

    herr_t status   = H5P_DEFAULT;
    hsize_t iOffset = 0;
    hsize_t iCount  = iSize;  
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
    hsize_t iFullSize   = iNumArr*iSize;

    hid_t hDataSpace = H5Screate_simple(1, &iFullSize, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, pName,  H5T_NATIVE_DOUBLE, hDataSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t hMemSpace  = H5Screate_simple (1, &iCount, NULL); 

    for(int i = 0; i < iNumArr; i++) {
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dwrite(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData[i]);


        iOffset += iSize;
    }
    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSpace(hMemSpace);
    return (status >= 0)?0:-1;
}
*/


//-----------------------------------------------------------------------------
// group_info
//  callback for H5Giterate2
//
static herr_t group_info(hid_t loc_id, const char *name, void *opdata) {
    herr_t ret=0;
    printf("%s%s\n", (char*) opdata, name);
    return ret;
}

//----------------------------------------------------------------------------
// listDataSets
//
int qdf_listGroupContents(hid_t loc_id, const char *pName, const char *pIndent) {
    int iResult = 0;
    //   printf("  %sA(", pIndent);
    char sIndent[64];
    printf("%s  DataSets\n", pIndent);
    sprintf(sIndent, "    %s", pIndent);
    int idx=0;
    herr_t status = H5Giterate(loc_id, pName, &idx, group_info, sIndent);
    
    iResult = (status >= 0)?0:-1;
    return iResult;        
}

//----------------------------------------------------------------------------
// qdf_getDataType
//
int qdf_getDataType(hid_t hDataSet) {
    int iType = DS_TYPE_NONE;
    
    hid_t hType = H5Dget_type(hDataSet);
    if (H5Tequal(hType, H5T_NATIVE_CHAR) > 0)  {
        iType = DS_TYPE_CHAR;
    } else if (H5Tequal(hType, H5T_NATIVE_SHORT) > 0)  {
        iType = DS_TYPE_SHORT;
    } else if (H5Tequal(hType, H5T_NATIVE_INT) > 0)  {
        iType = DS_TYPE_INT;
    } else if (H5Tequal(hType, H5T_NATIVE_LONG) > 0)  {
        iType = DS_TYPE_LONG;
    } else if (H5Tequal(hType, H5T_NATIVE_LLONG) > 0)  {
        iType = DS_TYPE_LLONG;
    } else if (H5Tequal(hType, H5T_NATIVE_UCHAR) > 0)  {
        iType = DS_TYPE_UCHAR;
    } else if (H5Tequal(hType, H5T_NATIVE_USHORT) > 0)  {
        iType = DS_TYPE_USHORT;
    } else if (H5Tequal(hType, H5T_NATIVE_UINT) > 0)  {
        iType = DS_TYPE_UINT;
    } else if (H5Tequal(hType, H5T_NATIVE_ULONG) > 0)  {
        iType = DS_TYPE_ULONG;
    } else if (H5Tequal(hType, H5T_NATIVE_ULLONG) > 0)  {
        iType = DS_TYPE_ULLONG;
    } else if (H5Tequal(hType, H5T_NATIVE_FLOAT) > 0)  {
        iType = DS_TYPE_FLOAT;
    } else if (H5Tequal(hType, H5T_NATIVE_DOUBLE) > 0)  {
        iType = DS_TYPE_DOUBLE;
    } else if (H5Tequal(hType, H5T_NATIVE_LDOUBLE) > 0)  {
        iType = DS_TYPE_LDOUBLE;
    } else {
        iType = DS_TYPE_NONE;
    }
    H5Tclose(hType);
    return iType;
}




//----------------------------------------------------------------------------
// popInfo
//  callback function for iteration in getFirstPopulation()
//
herr_t popInfoUtil(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        strcpy((char *)opdata, name);
        status = 1;
    }
    return status;
}

//----------------------------------------------------------------------------
// qdf_getFirstPopulation
//  create and return name for first population found in QDF file
//  the returned string must be delete[]d by the caller
//
char *qdf_getFirstPopulation(const char *pPopQDF) {
    char *pPop = NULL;

    hid_t hFile = qdf_openFile(pPopQDF);
    if (hFile > 0) {
        if (H5Lexists(hFile, POPGROUP_NAME, H5P_DEFAULT)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup > 0) {
                char s[64];
                *s = '\0';
                H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfoUtil, s);
                
                if (*s != '\0') {
                    pPop = new char[strlen(s)+1];
                    strcpy(pPop, s);
                } else {
                    printf("No Population subgroup in file [%s]\n", pPopQDF);
                }
                qdf_closeGroup(hPopGroup);
            }
        } else {
            printf("No Population Group in file [%s]\n", pPopQDF);
        }
        qdf_closeFile(hFile);
    } else {
        printf("Not a QDF file [%s]\n", pPopQDF);
    }
    return pPop;
}


//----------------------------------------------------------------------------
// qdf_checkForPop
//  create and return name of population if it appears in QDF file
//  the returned string must be delete[]d by the caller
//
char *qdf_checkForPop(const char *pPopQDF, const char *pSpeciesName) {
    char *pPop = NULL;

    hid_t hFile = qdf_openFile(pPopQDF);
    if (hFile > 0) {
        if (H5Lexists(hFile, POPGROUP_NAME, H5P_DEFAULT)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup > 0) {
                if (H5Lexists(hPopGroup, pSpeciesName, H5P_DEFAULT)) {
                    pPop = new char[strlen(pSpeciesName)+1];
                    strcpy(pPop, pSpeciesName);
                }
                qdf_closeGroup(hPopGroup);
            }
            qdf_closeFile(hFile);
        } else {
            printf("No Population Group in file [%s]\n", pPopQDF);
        }
    } else {
        printf("Not a QDF file [%s]\n", pPopQDF);
    }
    return pPop;
}


//----------------------------------------------------------------------------
// qdf_checkForGeo
//  check if geo and grid sections are present
//
int qdf_checkForGeo(const char *pQDF) {
    int iResult = -1;

    hid_t hFile = qdf_openFile(pQDF);
    if (hFile > 0) {
        if (H5Lexists(hFile, GRIDGROUP_NAME, H5P_DEFAULT)) {
            if (H5Lexists(hFile, GEOGROUP_NAME, H5P_DEFAULT)) {
                iResult = 0;
            }
        }
        qdf_closeFile(hFile);
    } else {
        printf("Not a QDF file [%s]\n", pQDF);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// qdf_createPolyLine
//
PolyLine *qdf_createPolyLine(hid_t hSpeciesGroup, const char *pPLParName) {
    printf("qdf_createPoly will work on %s\n", pPLParName);
    PolyLine *pPL = NULL;

    hid_t hAttr = H5Aopen(hSpeciesGroup, pPLParName, H5P_DEFAULT);

    hid_t hSpace = H5Aget_space(hAttr);
    hsize_t dims = 3;
    hvl_t       aPLData[3];
    int ndims = H5Sget_simple_extent_dims(hSpace, &dims, NULL);
    if (ndims == 1) {
        hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);

        herr_t status = H5Aread(hAttr, hMemType, aPLData);
            
        if (status >= 0) {
            if (aPLData[0].len > 0) {
                pPL = new PolyLine((uint)aPLData[0].len-1);
                memcpy(pPL->m_afX, aPLData[0].p, (pPL->m_iNumSegments+1)*sizeof(double));
                memcpy(pPL->m_afV, aPLData[1].p, (pPL->m_iNumSegments+1)*sizeof(double));
                memcpy(pPL->m_afA, aPLData[2].p, pPL->m_iNumSegments*sizeof(double));
            } else {
                pPL = new PolyLine((uint)0);
            }
        } else {
            printf("Couldn't read %s attrtibute\n", pPLParName);
            
        }
        status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
       
        qdf_closeAttribute(hAttr);
        qdf_closeDataType(hMemType);
        qdf_closeDataSpace(hSpace);
    } else {
        printf("Bad number of dimensions: %d\n", ndims);
    }

    return pPL;

}

//-----------------------------------------------------------------------------
// qdf_writePolyLine
//
int qdf_writePolyLine(hid_t hSpeciesGroup, PolyLine *pPL, const char *pPLParName) {
    int iResult = 0;
    
    hvl_t       aPLData[3];
    if ((pPL != NULL) && (pPL->m_iNumSegments > 0)) {
        int iNumPoints = pPL->m_iNumSegments+1;
        aPLData[0].len = iNumPoints;
        aPLData[0].p   = (void *) malloc(iNumPoints*sizeof(double));
        memcpy(aPLData[0].p, pPL->m_afX, iNumPoints*sizeof(double));
        aPLData[1].len = iNumPoints;
        aPLData[1].p   = (void *) malloc(iNumPoints*sizeof(double));
        memcpy(aPLData[1].p, pPL->m_afV, iNumPoints*sizeof(double));
        aPLData[2].len = pPL->m_iNumSegments;
        aPLData[2].p   = (void *) malloc(pPL->m_iNumSegments*sizeof(double));
        memcpy(aPLData[2].p, pPL->m_afA, pPL->m_iNumSegments*sizeof(double));
    } else {
        aPLData[0].len = 0;
        aPLData[0].p   = NULL;
        aPLData[1].len = 0;
        aPLData[1].p   = NULL;
        aPLData[2].len = 0;
        aPLData[2].p   = NULL;
    }

    hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);
    hsize_t dims = 3;
    hid_t hSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttr = H5Acreate (hSpeciesGroup, pPLParName, hMemType, hSpace, H5P_DEFAULT,
                             H5P_DEFAULT);
    herr_t status = H5Awrite (hAttr, hMemType, aPLData);


    status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
    qdf_closeAttribute (hAttr);
    iResult = (status >= 0)?0:-1;

    return iResult;
}
