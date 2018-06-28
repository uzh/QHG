#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include <vector>
#include <string>

#include "QDFUtils.h"

bool s_bShowAttributes=false;

//-----------------------------------------------------------------------------
// attr_info
//  callback for H5Aiterate2
//
herr_t attr_info(hid_t loc_id, const char *name, const H5A_info_t*pInfo, void *opdata) {
    herr_t ret=0;
    hid_t attr, atype, aspace;
    //    int   rank;
    hsize_t sdim[64];
    //    int i;

    int iDum[16];
    double fDum[16];
    char sDum[256];

    attr = H5Aopen_name(loc_id, name);

    aspace = H5Aget_space(attr);
    //    rank = H5Sget_simple_extent_ndims(aspace);
    ret = H5Sget_simple_extent_dims(aspace, sdim, NULL);

    //   printf("\n");
    printf("%s%s:", (char*) opdata, name);

    //   strcpy((char *)opdata, ", ");

    atype  = H5Aget_type(attr);
    hid_t type_class = H5Tget_class (atype); 
    switch (type_class) {
    case H5T_INTEGER :
        H5Aread(attr, atype, iDum);
        for (uint i = 0; i < sdim[0]; i++) {
            printf(" %d", iDum[i]);
        }
        break;
    case H5T_FLOAT :
        H5Aread(attr, atype, fDum);
        for (uint i = 0; i < sdim[0]; i++) {
            printf(" %f", fDum[i]);
        }

        break;
    case H5T_STRING :
        H5Aread(attr, atype, sDum);
        printf(" %s", sDum);
        
        break;
    case H5T_BITFIELD :
        printf("bitfield");
        break;
    case H5T_OPAQUE :
        printf("(opaque)");
        break;
    case H5T_COMPOUND :
        printf("(compound)");
        break;
    case H5T_REFERENCE :
        printf("(reference)");
        break;
    case H5T_ENUM :
        printf("(enum)");
        break;
    case H5T_VLEN :
        printf("vlen");
        break;
    case H5T_ARRAY :
        printf("array");
        break;
    }
    printf("\n");
    /*
    rank = H5Sget_simple_extent_ndims(aspace);
    ret = H5Sget_simple_extent_dims(aspace, sdim, NULL);
    
    if(rank > 0) {
       printf("Rank : %d \n", rank);
       printf("Dimension sizes : ");
       for (i=0; i< rank; i++) printf("%d ", (int)sdim[i]);
       printf("\n");
    }
    */
    qdf_closeDataType(atype);
    qdf_closeDataSpace(aspace);
    qdf_closeAttribute(attr);

    return(ret);
} 
//-----------------------------------------------------------------------------
// group_info
//  callback for H5Giterate2
//
herr_t group_info(hid_t loc_id, const char *name, void *opdata) {
    herr_t ret=0;
    printf("%s%s\n", (char*) opdata, name);
    return ret;
}

//----------------------------------------------------------------------------
// listAttributes
//
int listAttributes(hid_t loc_id, const char *pIndent) {
    int iResult = 0;
    //   printf("  %sA(", pIndent);
    char sIndent[64];
    printf("%s  Attributes\n", pIndent);
    sprintf(sIndent, "    %s", pIndent);

    herr_t status = H5Aiterate2(loc_id, H5_INDEX_NAME, H5_ITER_INC, NULL, attr_info, sIndent);
    
        iResult = (status >= 0)?0:-1;
    return iResult;        
}

//----------------------------------------------------------------------------
// listDataSets
//
int listDataSets(hid_t loc_id, const char *pName, const char *pIndent) {
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


//-----------------------------------------------------------------------------
// attr_info
//  callback for H5Aiterate2
//
herr_t addData_info(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {

    H5O_info_t      infobuf;
    std::vector<std::string> *pv = (std::vector<std::string>* )(opdata);
    herr_t status = H5Oget_info_by_name (loc_id, name, &infobuf, H5P_DEFAULT);
    if (infobuf.type == H5O_TYPE_DATASET) {
        if (strcmp(name, AGENT_DATASET_NAME) != 0) {
            pv->push_back(name);
        }
    }
    return (status >= 0)?0:-1;

}

//----------------------------------------------------------------------------
// popInfo
//
herr_t popInfo(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status=-1;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        bool bShowAttr = false;
        char *p = (char *) opdata;
        if (*p =='*') {
            bShowAttr = true;
            p++;
        }
        hid_t hSpecies = qdf_openGroup(loc_id, name); 
        
        hid_t hDataSet = H5Dopen(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);
        hsize_t dims[3];
        hsize_t maxdims = 3;
        int iNumDims = H5Sget_simple_extent_dims(hDataSpace, dims, &maxdims);
        int iNum = 1;
        for (int i = 0; i < iNumDims; i++) {
            iNum *= dims[i];
        }

        int iS=-1;
        /*int iResult =*/ qdf_extractAttribute(hSpecies, SPOP_ATTR_SPECIES_ID, 1, &iS);
        /*
        hid_t attr = H5Aopen_by_name(loc_id, name, "SpeciesID", H5P_DEFAULT, H5P_DEFAULT);
        hid_t atype  = H5Aget_type(attr);
        hsize_t size = H5Tget_size (atype);
        char *pType = new char[size+1];
        H5Aread(attr, atype, pType);
        */

        int iC = -1;
        qdf_extractAttribute(hSpecies, SPOP_ATTR_CLASS_ID, 1, &iC);
        char sClass[64];
        qdf_extractSAttribute(hSpecies, SPOP_ATTR_CLASS_NAME, 64, sClass);
        printf("%s+species [%s(%d)][class:%s(%d)]:%d agents\n", p, name, iS, sClass, iC, iNum);
        //printf("%s  ", (char *)opdata);
        if (bShowAttr) {
            listAttributes(hSpecies, p);
        }

        // list additional data sets if present
        std::vector<std::string> vAdditional;

        status = H5Literate(hSpecies, H5_INDEX_NAME, H5_ITER_INC, NULL, addData_info, &vAdditional);
        if (vAdditional.size() > 0) {
            printf("%s  +additional Datasets\n", p);
            for (uint i = 0; i < vAdditional.size(); ++i) {
                printf("%s    %s\n", p, vAdditional[i].c_str());
            }
        }
        //  delete[] pType;
        qdf_closeDataSet(hDataSet);
        qdf_closeGroup(hSpecies);
    }
    return (status >= 0)?0:1;
}

int showGroup(hid_t hLoc, const char *pName, bool bShowAttributes) {
    int iResult = 0;
    if (H5Lexists(hLoc, pName, H5P_DEFAULT)) {
        hid_t hGroup = qdf_openGroup(hLoc, pName);
        if (hGroup > 0) {
            printf("  +group [%s]\n", pName);
            if (bShowAttributes) {
                iResult = listAttributes(hGroup, "   ");
            }
            iResult = listDataSets(hLoc, pName, "   ");
        }
    }
    return iResult;
}

int showPopGroups(hid_t hLoc, const char *pName, bool bShowAttributes) {
    int iResult = 0;
    if (H5Lexists(hLoc, pName, H5P_DEFAULT)) {
        hid_t hPopGroup = qdf_openGroup(hLoc, pName);
         if (hPopGroup > 0) {
             printf("  +group [%s]\n", pName);
             char s[64];
             
             sprintf(s, "%s     ", bShowAttributes?"*":"");
             H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfo, s);
         }
    }
    return iResult;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char *pFile;
    bool bShowAttributes = false;
    if (iArgC > 1) {
        int iFile = 1;
        if (iArgC > 2) {
            if (strcmp(apArgV[1], "-a") == 0) {
                iFile = 2;
                bShowAttributes = true;
            } else {
                
            }
        }
        pFile = apArgV[iFile];
    
        //open file
        hid_t hFile = qdf_openFile(pFile);
        if (hFile > 0) {
            // open root group
            hid_t hRoot = qdf_openGroup(hFile, "/", false);
            if (hRoot > 0) {
                // check if root group has QHG attribute named 
                htri_t hExists = H5Aexists(hRoot, ROOT_ATTR_NAME);
                if (hExists > 0) {
                    printf("file [%s]\n", pFile);
                    if (bShowAttributes) {
                        listAttributes(hRoot, "  ");
                    }
                 
                    iResult = showGroup(hRoot, GRIDGROUP_NAME, bShowAttributes);
                    if (iResult == 0) {
                        iResult = showGroup(hRoot, GEOGROUP_NAME, bShowAttributes);
                    }
                    if (iResult == 0) {
                        iResult = showGroup(hRoot, CLIGROUP_NAME, bShowAttributes);
                    }
                    if (iResult == 0) {
                        iResult = showGroup(hRoot, VEGGROUP_NAME, bShowAttributes);
                    }
                    if (iResult == 0) {
                        iResult = showPopGroups(hRoot, POPGROUP_NAME, bShowAttributes);
                    }
                    if (iResult == 0) {
                        iResult = showGroup(hRoot, MSTATGROUP_NAME, bShowAttributes);
                    }
                } else {
                    printf("File [%s] is not a qdf file\n", pFile);
                }
                qdf_closeGroup(hRoot);
            } else {
                printf("Couldn't open root group in file [%s]\n", pFile);
            }
        } else {
            printf("File [%s] is not a hdf file\n", pFile);
        }
    } else {
        printf("usage: %s [-a] <hfdfile>\n", apArgV[0]);
    }
    return iResult;
}
