#include <stdio.h>
#include <string.h>

#include <map>

#include <hdf5.h>
#include "QDFUtils.h"
#include "QDFArraySniffer.h"

static bool s_bVerbose = false;

//----------------------------------------------------------------------------
// constructor
//
QDFArraySniffer::QDFArraySniffer(hid_t hFile, int iNumCells) 
    : m_hFile(hFile),
      m_iNumCells(iNumCells) {

}


//----------------------------------------------------------------------------
// scan
//
int QDFArraySniffer::scan() {
    int iResult = -1;
    if (m_hFile > 0) {
        hid_t hRoot = qdf_openGroup(m_hFile, "/", false);
        if (hRoot > 0) {
            // check if root group has QHG attribute named 
            htri_t hExists = H5Aexists(hRoot, ROOT_ATTR_NAME);
            if (hExists > 0) {
                iResult = 0;
                iResult = scanGroup(hRoot, GRIDGROUP_NAME);
                if (iResult == 0) {
                    iResult = scanGroup(hRoot, GEOGROUP_NAME);
                }
                if (iResult == 0) {
                    iResult = scanGroup(hRoot, CLIGROUP_NAME);
                }
                if (iResult == 0) {
                    iResult = scanGroup(hRoot, VEGGROUP_NAME);
                }
                if (iResult == 0) {
                    iResult = scanPops(hRoot);
                }
                if (iResult == 0) {
                    iResult = scanGroup(hRoot, MSTATGROUP_NAME);
                }
            } else {
                printf("File is not a qdf file\n");
            }
            qdf_closeGroup(hRoot);
        } else {
            printf("Couldn't open root group in file\n");
        }
    } else {
        printf("QDF file not open\n");
    }

    
    return iResult;  
}


//----------------------------------------------------------------------------
// scanGroup
//
int QDFArraySniffer::scanGroup(hid_t hLoc, const char *pName) {
    int iResult = -1;
    if (H5Lexists(hLoc, pName, H5P_DEFAULT)) {
        hid_t hGroup = qdf_openGroup(hLoc, pName);
        if (hGroup > 0) {
            //            printf("  +group [%s]\n", pName);
            int iNC = -1;
            iResult = qdf_extractAttribute(hGroup, "NumCells", 1, &iNC); 
            if (iResult == 0) {
                if (m_iNumCells <= 0) {
                    m_iNumCells = iNC;
                    //                    printf("Got numcell for [%s]: %d\n", pName, m_iNumCells);
                } else {
                    if (m_iNumCells != iNC) {
                        iResult = -1;
                        printf("Mismatch number of cells for [%s]: %d != %d\n", pName, iNC, m_iNumCells);
                    } else {
                        //                        printf("Numcells for [%s] is ok\n", pName);
                    }
                }
            // look for num cells
                if (iResult == 0) {
                    if (s_bVerbose) printf("Collecting datasets for [%s]\n", pName);
                    iResult = collectDataSets(hLoc, pName);
                }
            }
        }
    } else {
        iResult = 0;
        if (s_bVerbose) printf("Didn't find group [%s]\n", pName);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// getTypeName
// 
void getTypeName(hid_t hDataSet, char *sType, int iLen) {
    *sType = '\0';
    int iType = qdf_getDataType(hDataSet);
    switch (iType) {
    case DS_TYPE_CHAR:
        strncpy(sType, "CHAR", iLen);
        break;
    case DS_TYPE_SHORT:
        strncpy(sType, "SHORT", iLen);
        break;
    case DS_TYPE_INT:
        strncpy(sType, "INT", iLen);
        break;
    case DS_TYPE_LONG:
        strncpy(sType, "LONG", iLen);
        break;
    case DS_TYPE_LLONG:
        strncpy(sType, "LLONG", iLen);
        break;
    case DS_TYPE_UCHAR:
        strncpy(sType, "UCHAR", iLen);
        break;
    case DS_TYPE_USHORT:
        strncpy(sType, "USHORT", iLen);
        break;
    case DS_TYPE_UINT:
        strncpy(sType, "UINT", iLen);
        break;
    case DS_TYPE_ULONG:
        strncpy(sType, "ULONG", iLen);
        break;
    case DS_TYPE_ULLONG:
        strncpy(sType, "ULLONG", iLen);
        break;
    case DS_TYPE_FLOAT:
        strncpy(sType, "FLOAT", iLen);
        break;
    case DS_TYPE_DOUBLE:
        strncpy(sType, "DOUBLE", iLen);
        break;
    case DS_TYPE_LDOUBLE:
        strncpy(sType, "LDOUBLE", iLen);
        break;
    default:
        strncpy(sType, "???", iLen);
    }
}

//-----------------------------------------------------------------------------
// ds_info
//  callback for H5Giterate2
//
herr_t ds_info(hid_t loc_id, const char *name, void *opdata) {
    herr_t status=0;
    int iResult = 0;

    H5G_stat_t st;

    status =  H5Gget_objinfo(loc_id, name, false,  &st);
    if (status >= 0) {
        if (st.type == H5G_DATASET) {
            // now find the dimensions
            hid_t hDataSet = H5Dopen2(loc_id, name, H5P_DEFAULT);
            hid_t hDataSpace = H5Dget_space(hDataSet);
            

            char sType[64];
            
            *sType = '\0';
            if (s_bVerbose) {
                getTypeName(hDataSet, sType, 64);
            }

            int iType = qdf_getDataType(hDataSet);
            switch (iType) {
            case DS_TYPE_CHAR:
            case DS_TYPE_SHORT:
            case DS_TYPE_INT:
            case DS_TYPE_LONG:
            case DS_TYPE_LLONG:
            case DS_TYPE_UCHAR:
            case DS_TYPE_USHORT:
            case DS_TYPE_UINT:
            case DS_TYPE_ULONG:
            case DS_TYPE_ULLONG:
            case DS_TYPE_FLOAT:
            case DS_TYPE_DOUBLE:
            case DS_TYPE_LDOUBLE:
                iResult = 0;
                break;
            default:
                if (strcmp(name, CELL_DATASET_NAME) == 0) {
                    iResult = 0;
                } else {
                    strcpy(sType, "???");
                    iResult = -1;
                }
            }

            hsize_t dims[2];
            int iNDims = H5Sget_simple_extent_dims(hDataSpace, dims, NULL);            
            if (s_bVerbose) printf(" [%s] is a %u-dim dataset of type %s\n", name, iNDims, sType);

            if ((iResult == 0) && ((iNDims == 1) || (iNDims == 2))) {
                if (s_bVerbose) printf(" Result and ndims (%u) ok\n", iNDims);
                std::map<std::string, uint> *pTemp = (std::map<std::string, uint>*)(opdata);
                if (iNDims == 1) {
                    (*pTemp)[name] = dims[0];
                } else {
                    if (s_bVerbose) printf("Handling 2-dim set: %llu arrays\n", dims[1]);
                    for (uint i = 0; i < dims[1]; i++) {
                        char *pNameExt = new char[strlen(name)+6];
                        sprintf(pNameExt, "%s:%02d", name, i);
                        (*pTemp)[pNameExt] = dims[0];
                    }
                }
            } else {
                if (s_bVerbose) printf(" Result (%d) or ndims (%u) bad\n", iResult, iNDims);
                
                // bad number of dimstoo many dims
            }
            
            qdf_closeDataSpace(hDataSpace);
            qdf_closeDataSet(hDataSet);
            
        } else {
            //            printf("[%s] is something else\n", name);
        }
    }
    return status;
}

//----------------------------------------------------------------------------
// collectDataSets
//
int QDFArraySniffer::collectDataSets(hid_t loc_id, const char *pName) {
    int iResult = 0;
    int idx=0;
    std::string slash("/");
    std::map<std::string, uint> mTemp;
    herr_t status = H5Giterate(loc_id, pName, &idx, ds_info, &mTemp);
    std::map<std::string, uint>::const_iterator it;
    for (it = mTemp.begin(); it != mTemp.end(); ++it) {
        if (it->second == (uint) m_iNumCells) {
            std::string sName(pName);

            m_vItems.push_back(sName+"/"+it->first);
        }
    }
    iResult = (status >= 0)?0:-1;
    return iResult;        
}

//----------------------------------------------------------------------------
// popInfo
//
herr_t popInfo(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status=-1;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        if (s_bVerbose) printf("Have group [%s]\n", name);

        hid_t hSpecies = qdf_openGroup(loc_id, name); 
        int iNC;
        int iResult = qdf_extractAttribute(hSpecies, "NumCells", 1, &iNC);
        if (iResult == 0) {
            hid_t hDataSet = H5Dopen(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
            hid_t hDataSpace = H5Dget_space(hDataSet);
            hsize_t dims[3];
            hsize_t maxdims = 3;
            int iNumDims = H5Sget_simple_extent_dims(hDataSpace, dims, &maxdims);
            uint iNum = 1;
           if (s_bVerbose) printf("Found %u array dims:", iNum);
            for (int i = 0; i < iNumDims; i++) {
                if (s_bVerbose) printf(" %llu", dims[i]);
                iNum *= dims[i];
            }
            printf("\n");
            std::map<std::string, uint> *pTemp = (std::map<std::string, uint>*)(opdata);
            (*pTemp)[name] = iNC;

            qdf_closeDataSet(hDataSet);
        } else {
            if (s_bVerbose) printf("  ->no 'NumCellls'\n");

        }
        qdf_closeGroup(hSpecies);
    }
    return (status >= 0)?0:1;
}

//----------------------------------------------------------------------------
// scanPops
//
int QDFArraySniffer::scanPops(hid_t hLoc) {
    int iResult = 0;
    hid_t hPopGroup = qdf_openGroup(hLoc, POPGROUP_NAME);
    std::string sName(POPGROUP_NAME);

    if (hPopGroup > 0) {
        // loop through species
        //  and collect numbers/cell
        std::map<std::string, uint> mTemp;
        if (s_bVerbose) printf("Found Population group\n");
        H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfo, &mTemp);
        std::map<std::string, uint>::const_iterator it;
        for (it = mTemp.begin(); it != mTemp.end(); ++it) {
            if (it->second == (uint) m_iNumCells) {
                
                m_vItems.push_back(sName+"/"+it->first);
            }
        }

        qdf_closeGroup(hPopGroup);
    } else {
        printf("Group [%s] not found\n", POPGROUP_NAME);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// scanPops
//
int QDFArraySniffer::scanPopFile(hid_t hFile) {
    int iResult = -1;
    
    if (hFile > 0) {
        hid_t hRoot = qdf_openGroup(hFile, "/", false);
        if (hRoot > 0) {
            iResult = scanPops(hRoot);
            qdf_closeGroup(hRoot);
        } else {
            printf("No root found\n");
        }
       
    } else {
        printf("Couldn't qdf file\n");
    }

    return iResult;
}

