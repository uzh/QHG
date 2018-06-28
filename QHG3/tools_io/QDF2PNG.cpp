#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <vector>
#include <string>

#include <hdf5.h>


#include "ParamReader.h"
#include "strutils.h"
#include "Vec3D.h"
#include "QDFUtils.h"
#include "QDFArray.h" 
#include "SCellGrid.h"

#include "LookUp.h"
#include "RainbowLookUp.h"
#include "Rainbow2LookUp.h"
#include "GeoLookUp.h"
#include "TwoToneLookUp.h"
#include "FadeOutLookUp.h"
#include "PNGImage.h"

#include "IcoFace.h"
#include "Surface.h"
#include "SurfaceGrid.h"
#include "TextRenderer.h"
#include "AlphaComposer.h"

#define ARR_CODE_NONE  -1
#define ARR_CODE_LON    1
#define ARR_CODE_LAT    2
#define ARR_CODE_ALT    3
#define ARR_CODE_ICE    4
#define ARR_CODE_WATER  5
#define ARR_CODE_COAST  6
#define ARR_CODE_TEMP   7
#define ARR_CODE_RAIN   8
#define ARR_CODE_NPP_B  9
#define ARR_CODE_NPP   10
#define ARR_CODE_DIST  11
#define ARR_CODE_TIME  12
#define ARR_CODE_POP   20

#define DS_TYPE_POP 14

typedef struct ds_info{
    std::string sGroup;
    std::string sSubGroup;
    std::string sDataSet;
    int         iDataType;
    ds_info() 
        : sGroup(""), sSubGroup(""),sDataSet(""), iDataType(DS_TYPE_NONE) {};
    ds_info(std::string sGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(""), sDataSet(sDataSet1), iDataType(iDataType1) {};
    ds_info(std::string sGroup1, std::string sSubGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(sSubGroup1), sDataSet(sDataSet1), iDataType(iDataType1) {};
 } ds_info;

typedef  std::pair<std::string, int>        arrind;
typedef  std::map<arrind, ds_info>         array_data;
typedef  std::map<arrind, LookUp *>        lookup_data; 
typedef  std::map<arrind, int>             which_data; 
typedef  std::vector<arrind>               data_order;
typedef  std::vector<std::string>          string_list;


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - export QDF arrays to PNG\n", pApp);
    printf("Usage:\n");
    printf("  %s -g <qdf_geogrid> [-q <qdf_data>[,<qdf_data>]*]  -s <w>x<h> -o <outpng>\n", pApp);
    printf("    -a <arrayspec>,[<arrayspec>]*\n");
    printf("    [-c <operation>] [-r <lon_roll>\n");
    printf("where\n");
    printf("  qdf_geogrid    QDF containing at least grid and geography\n");
    printf("  qdf_data       QDF file containing data to be extracted\n");
    printf("                 which is not contained in <qdf_geogrid>\n");
    printf("  w              width  of output png\n");
    printf("  h              height of output png\n");
    printf("  outpng         pattern for output pngs: the substring '###' will be replaced with the array name\n");
    printf("  arrayspec      array specification. Format:\n");
    printf("                 array_spec ::= <array_name>[@<index>][|<lookup>]\n");
    printf("                 array_name  :   name of array (s. below)\n");
    printf("                 index       :   index of qdf in which to look (0: qdf_geogrid, 1-N: qdf-data in given order)\n");
    printf("                 lookup      :   info for lookup, with format <lookup_name>[:<data>]* (s. below)\n");
    printf("  operation      comopsiting operator (currenly only: 'over')\n");
    printf("  longroll       longitude for smalles x vale (rolls image)\n");
    printf("Arraynames:\n");
    printf("  lon       longitude   (Geography::m_adLongitude)\n");
    printf("  lat       latitude    (Geography::m_adLatitude)\n");
    printf("  alt       altitudes   (Geography::m_adAltitude)\n");
    printf("  ice       ice cover   (Geography::m_abIce)\n");
    printf("  water     water       (Geography::m_adWater)\n");
    printf("  coastal   coastal     (Geography::m_abCoastal)\n");
    printf("  temp      temperature (Climate::m_adAnnualMeanTemp)\n");
    printf("  rain      rainfall    (Climate::m_adAnnualRainfall)\n");
    printf("  npp       total npp   (NPPVegetation::m_adTotalANPP)\n");
    printf("  npp_b     base npp    (NPPVegetation::m_adBaseANPP)\n");
    printf("  dist      travel distance (MoveStats::m_adDist)\n");
    printf("  time      travel time     (MoveStats::m_adTime)\n");
    printf("  pop       population number\n");
    printf("Lookups:\n");
    printf("  rainbow   data: min, max\n");
    printf("  rainbow2  data: min, max\n");
    printf("  geo       data: min, sealevel, max\n");
    printf("  twotone   data: Sepvalue, RGBA1, RGBA2\n");
    printf("  fadeout   data: min,max, RGBAmax\n");

    printf("Postprocessing:\n");
    printf("For superposition use imagemagick:\n");
    printf("  composite -compose Over onklop_ice.png onklop_alt.png destination.png\n");

    printf("Call example\n");
    printf("./QDF2PNG -g zworld_22.0_kya_256.qdf\\\n");
    printf("          -q ooa_pop-Sapiens_ooa__010000.qdf,aternative_ice.qdf \\\n");
    printf("          -s 720x360\\\n");
    printf("          -o onklop_###_024.PNG \\\n");
    printf("          -a alt|geo:-6000:0:6000,ice@2|twotone:0.5:#00000000:#FFFFFFFF,\\\n");
    printf("             pop_Sapiens_ooa@1|fadeout:0:40:#00FF00FF\\\n"); 
    printf("          -r -25\\\n");
    printf("          -c over\n");
}  


//----------------------------------------------------------------------------
// splitQDFs
//
int splitQDFs(char *pQDFData, string_list &vQDFs) {
    int iResult = 0;
    if (pQDFData != NULL) {
        char *pCtx;
        char *p = strtok_r(pQDFData, ",", &pCtx);
        while (p != NULL) {
            vQDFs.push_back(p);
            p = strtok_r(NULL, ",", &pCtx);
        }
        if (vQDFs.size() == 0) {
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// checkArrayName
//
int checkArrayName(arrind &aiNameIndex, array_data &mvArrayData) {
    int iCode = ARR_CODE_NONE;
    const char *pName = aiNameIndex.first.c_str();
    if (strcmp(pName, "lon") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LONGITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LON;
    } else if (strcmp(pName, "lat") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LATITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LAT;
    } else if (strcmp(pName, "alt") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ALTITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_ALT;
    } else if (strcmp(pName, "ice") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ICE_COVER, DS_TYPE_CHAR);
        iCode = ARR_CODE_ICE;
    } else if (strcmp(pName, "water") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_WATER, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_WATER;
    } else if (strcmp(pName, "coastal") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_COASTAL, DS_TYPE_CHAR);
        iCode = ARR_CODE_COAST;
    } else if (strcmp(pName, "temp") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_MEAN_TEMP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TEMP;
    } else if (strcmp(pName, "rain") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_TOT_RAIN, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_RAIN;
    } else if (strcmp(pName, "npp") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (strcmp(pName, "npp_b") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_BASE_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (strcmp(pName, "dist") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(MSTATGROUP_NAME, MSTAT_DS_DIST, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_DIST;
    } else if (strcmp(pName, "time") == 0)  {
        mvArrayData[aiNameIndex] = ds_info(MSTATGROUP_NAME, MSTAT_DS_TIME, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TIME;
    } else if (strncmp(pName, "pop_", 4) == 0)  {
        const char *p1 = pName + 4;
        char sGroup[512];
        sprintf(sGroup, "%s/%s", POPGROUP_NAME, p1);

        mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, p1, AGENT_DATASET_NAME, DS_TYPE_POP);
        //mvArrayData[pName] = ds_info(sGroup, AGENT_DATASET_NAME, DS_TYPE_POP);
        iCode = ARR_CODE_POP;
    } else {
        iCode = ARR_CODE_NONE;
    }
    

    if (iCode != ARR_CODE_NONE) {
        printf("added ARR [%s@%d] -> Group [%s], dataset [%s], type %d\n", 
               aiNameIndex.first.c_str(), 
               aiNameIndex.second, 
               mvArrayData[aiNameIndex].sGroup.c_str(), 
               mvArrayData[aiNameIndex].sDataSet.c_str(), 
               mvArrayData[aiNameIndex].iDataType);
    }
    return iCode;
}


//---------------------------------------------------------------------------
// hex2Val
//
bool hex2Val(const char *pHex, uchar *puVal) {
    bool bOK = false;
    char *pEnd;
    int iVal = strtol(pHex, &pEnd, 16);
    if (*pEnd == '\0') {
        *puVal = (uchar) (iVal & 0xff);
        bOK = true;
    }
    return bOK;
}


//---------------------------------------------------------------------------
// extractColors
//
bool extractColors(const char *pColorDesc, uchar *pR, uchar *pG, uchar *pB, uchar *pA) {
    bool bOK = false;
    char sHex[3];
    sHex[2] = '\0';

    if (strlen(pColorDesc) == 9) {
        if (*pColorDesc == '#') {
            memcpy(sHex, pColorDesc+1, 2);
            if  (hex2Val(sHex, pR)) {
                
                memcpy(sHex, pColorDesc+3, 2);
                if  (hex2Val(sHex, pG)) {
                
                    memcpy(sHex, pColorDesc+5, 2);
                    if  (hex2Val(sHex, pB)) {

                        memcpy(sHex, pColorDesc+7, 2);
                        if  (hex2Val(sHex, pA)) {
                            bOK = true;
                        }
                    }
                }
            }
        }
    }
    
    return bOK;
}


//----------------------------------------------------------------------------
// createLookUp
//
LookUp *createLookUp(char *pLUName, string_list &vParams) {
    LookUp *pLU = NULL;

    double dMin = 0;
    double dMax = 0;
    double dSea = 0;
    uchar uR1  = 0;
    uchar uG1  = 0;
    uchar uB1  = 0;
    uchar uA1  = 0;
    uchar uR2  = 0;
    uchar uG2  = 0;
    uchar uB2  = 0;
    uchar uA2  = 0;
    int iResult = 0;
    int iBadIndex = -1;

    if (strcmp(pLUName, "rainbow") == 0) {
        if (vParams.size() > 1) {
            if (strToNum(vParams[0].c_str(), &dMin)) {
                if (strToNum(vParams[1].c_str(), &dMax)) {
                    pLU = new RainbowLookUp(dMin, dMax);
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iResult = -3;
        }
    } else if (strcmp(pLUName, "rainbow2") == 0) {
        if (vParams.size() > 1) {
            if (strToNum(vParams[0].c_str(), &dMin)) {
                if (strToNum(vParams[1].c_str(), &dMax)) {
                    pLU = new Rainbow2LookUp(dMin, dMax);
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iResult = -3;
        }
    } else if (strcmp(pLUName, "geo") == 0) {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0].c_str(), &dMin)) {
                if (strToNum(vParams[1].c_str(), &dSea)) {
                    if (strToNum(vParams[2].c_str(), &dMax)) {
                        pLU = new GeoLookUp(dMin, dSea, dMax);
                    } else {
                        iBadIndex = 2;
                        iResult = -2;
                    }
                } else {
                    iBadIndex = 2;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iResult = -3;
        }

    } else if (strcmp(pLUName, "twotone") == 0) {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0].c_str(), &dMax)) {
                if (extractColors(vParams[1].c_str(), &uR1, &uG1, &uB1, &uA1)) {
                    if (extractColors(vParams[2].c_str(), &uR2, &uG2, &uB2, &uA2)) {
                        pLU = new TwoToneLookUp(dMax, 
                                                uR1/255.0, uG1/255.0, uB1/255.0, uA1/255.0, 
                                                uR2/255.0, uG2/255.0, uB2/255.0, uA2/255.0);
                    } else {
                        iBadIndex = 2;
                        iResult = -4;
                    }
                } else {
                    iBadIndex = 1;
                    iResult = -4;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iResult = -3;
        }
        
    } else if (strcmp(pLUName, "fadeout") == 0) {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0].c_str(), &dMin)) {
                if (strToNum(vParams[1].c_str(), &dMax)) {
                    if (extractColors(vParams[2].c_str(), &uR1, &uG1, &uB1, &uA1)) {
                        pLU = new FadeOutLookUp(dMin, dMax,
                                                uR1/255.0, uG1/255.0, uB1/255.0, uA1/255.0);
                    } else {
                        iBadIndex = 2;
                        iResult = -4;
                    }
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iResult = -3;
        }

    } else {
        iResult = -1;
    }
    
    switch (iResult) {
    case 0:
        break;
    case -1:
        printf("Unknown lookup name [%s]\n", pLUName);
        break;
    case -2:
        printf("Expected number for parameter of [%s]: [%s]\n", pLUName, vParams[iBadIndex].c_str());
        break;
    case -3:
        printf("Not enough parameters for [%s]\n", pLUName);
        break;
    case -4:
        printf("Not a valid color description ('#RRGGBBAA'): [%s]\n", vParams[iBadIndex].c_str());
        break;
    default:
        printf("Unknown error %d\n", iResult);
    }

    return pLU;

}


//----------------------------------------------------------------------------
// splitArraySpec
//
int splitArraySpec(char *sArrayData, array_data &mvArrayData, lookup_data &mvLookUpData, data_order &vOrder) {
    int iResult = -1;
    char *pCtx;
    char *pName = strtok_r(sArrayData, "|", &pCtx);
    if (pName != NULL) {
        int iIndex = -1;
        char  *pIndex = strchr(pName, '@');
        if (pIndex != NULL) {
            *pIndex = '\0';
            pIndex++;
            if (strToNum(pIndex, &iIndex)) {
                //save it
            } else {
                printf("Bad index value [%s]\n", pIndex);
                iResult = -1;
            }
        }
        arrind aiNameIndex(pName, iIndex);
        // save index

        int iArrCode = checkArrayName(aiNameIndex, mvArrayData);
        if (iArrCode >= 0) {
            char *p2 =  strtok_r(NULL, ":", &pCtx);
            if (p2 != NULL) {
                string_list vParams;
                char *p3 = strtok_r(NULL, ":", &pCtx);
                while (p3 != NULL) {
                    vParams.push_back(p3);
                    p3 = strtok_r(NULL, ":", &pCtx);
                }
                
                LookUp *pLU = createLookUp(p2, vParams);
                mvLookUpData[aiNameIndex] = pLU;
                printf("added LU  [%s@%d] -> [%p]\n", aiNameIndex.first.c_str(), aiNameIndex.second, pLU);

                if (pLU != NULL) {
                    vOrder.push_back(aiNameIndex);
                    iResult = 0;
                } else {
                    iResult =-1;
                }
            } else {
                if (mvArrayData.size() == 0) {
                    printf("No lookupname given\n");
                    iResult = -1;
                }
            }
        } else {
            printf("Unknown arrayname [%s]\n", pName);
            iResult = -1;
        }
    }
    
    return iResult; 
}


//----------------------------------------------------------------------------
// splitArrayColors
//
int splitArrayColors(char *sArrayData,  array_data &mvArrayData, lookup_data &mvLookUpData, data_order &vOrder) {
    int iResult = 0;
    char *pCtx;
    char *pSpec = strtok_r(sArrayData, ",", &pCtx);
    while ((iResult == 0) && (pSpec != NULL)) {
        iResult = splitArraySpec(pSpec, mvArrayData, mvLookUpData, vOrder);
        pSpec = strtok_r(NULL, ",", &pCtx);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// cleanUp
//
void cleanUp(lookup_data &mvLookUpData) {
    lookup_data::const_iterator it;
    for (it = mvLookUpData.begin(); it != mvLookUpData.end(); ++it) {
        delete it->second;
    }
}


//----------------------------------------------------------------------------
// checkGroup
//
bool checkGroup(hid_t hFile, const ds_info &rdInfo) {
    bool bOK = false; 
    hid_t hGroup = qdf_openGroup(hFile, rdInfo.sGroup.c_str(), true);
           
    if (hGroup != H5P_DEFAULT) {
        hid_t hSubGroup = H5P_DEFAULT;
        hid_t hTestGroup = hGroup;
        if (rdInfo.sSubGroup != "") {
            hSubGroup = qdf_openGroup(hGroup, rdInfo.sSubGroup.c_str(), true);
            hTestGroup = hSubGroup;
        } 
        if (hTestGroup != H5P_DEFAULT) {
            if (qdf_exists(hTestGroup, rdInfo.sDataSet.c_str())) {
                bOK = true;
            }
        }
        if (hSubGroup != H5P_DEFAULT) {
            qdf_closeGroup(hSubGroup);
        }
        qdf_closeGroup(hGroup);
    }
    return bOK;
}


//----------------------------------------------------------------------------
// checkConsistent
//
int checkConsistent(string_list &vQDFs, array_data &mvArrayData, which_data &mvWhich) {
    int iResult = 0;

    
    hid_t  hQDFGeoGrid = qdf_openFile(vQDFs[0].c_str());
    if (hQDFGeoGrid != H5P_DEFAULT) {
        hid_t hGrid = qdf_openGroup(hQDFGeoGrid, GRIDGROUP_NAME);
        if (hGrid != H5P_DEFAULT) {
            qdf_closeGroup(hGrid);
        } else {
            printf("No grid group in [%s]\n", vQDFs[0].c_str());
            iResult =-1;
        }
        qdf_closeFile(hQDFGeoGrid);

        array_data::const_iterator it;
        for (it = mvArrayData.begin(); (iResult == 0) && (it != mvArrayData.end()); ++it) {
            bool bSearching = true;
            for (uint i = 0; bSearching && (i < vQDFs.size()); i++) {
                hid_t hQDFData = qdf_openFile(vQDFs[i].c_str());
                if (hQDFData != H5P_DEFAULT) {
                    if (checkGroup(hQDFData, it->second)) {

                        if ((it->first.second < 0) || (it->first.second == (int)i)) {
                    
                            printf("%s: using Group [%s%s%s], Dataset [%s] in [%s]\n", 
                                   it->first.first.c_str(), 
                                   it->second.sGroup.c_str(),  
                                   (it->second.sSubGroup != "")?"/":"",
                                   it->second.sSubGroup.c_str(),  
                                   it->second.sDataSet.c_str(), 
                                   vQDFs[i].c_str());
                            
                            bSearching = false;
                            mvWhich[it->first] = i;
                        } 
                    } 
                    qdf_closeFile(hQDFData);
                } else {
                    printf("Couldn't open [%s]\n", vQDFs[i].c_str());
                    iResult = -1;
                }
            }

            if (bSearching) {
                mvWhich[it->first] = -1;
                iResult = -1;
                printf("Group [%s], Dataset [%s] not found in any of the QDF files\n", 
                       it->second.sGroup.c_str(),  
                       it->second.sDataSet.c_str());
            } else {
                
                iResult = 0;
            }
        }


    } else {
        printf("Couldn't open grid group in [%s]\n", vQDFs[0].c_str());
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractData
//
double *extractData(const char *pQDF, int iNumCells, const ds_info &pGroupDS) {
    int iResult = -1;
    double *pdArr  = NULL;
    char   *pdArr1 = NULL;
 
    // get number of elements
    hid_t hFile = qdf_openFile(pQDF);

    // we know this will work, too
    hid_t hGroup = qdf_openGroup(hFile, pGroupDS.sGroup.c_str());
        
    pdArr = new double[iNumCells];
        
    switch (pGroupDS.iDataType) {
    case DS_TYPE_DOUBLE: 
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet.c_str(), iNumCells, (double *)pdArr);
        break;

    case DS_TYPE_CHAR:
        pdArr1 = new char[iNumCells];
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet.c_str(), iNumCells, (char *)pdArr1);
        if (iResult == 0) {
            for (int i = 0; i < iNumCells; i++) {
                pdArr[i] = pdArr1[i];
            }
        }
        delete[] pdArr1;
        break;

    case DS_TYPE_POP: {
        memset(pdArr, 0, iNumCells*sizeof(double));
        QDFArray *pQA = QDFArray::create(pQDF);
        if (pQA != NULL) {
            iResult = pQA->openArray(POPGROUP_NAME, pGroupDS.sSubGroup.c_str(),  pGroupDS.sDataSet.c_str());
            if (iResult == 0) {
                
                uint iNumAgents = pQA->getSize();
                int *pdArr2 = new int[iNumAgents];
                uint iCount = pQA->getFirstSlab(pdArr2, iNumAgents, "CellID");
                if (iCount != iNumAgents) {
                    printf("Got %d cell IDs instead of %d\n", iCount, iNumAgents);
                    iResult = -1;
                } else {
                    for (uint i = 0; i < iNumAgents; ++i) {
                        pdArr[pdArr2[i]]++;
                    }
                }
                delete[] pdArr2;
            }
            delete pQA;
        }
    }
        break;

    default:
        printf("Can't process data of type %d\n", pGroupDS.iDataType);
    }

    qdf_closeGroup(hGroup);
        
    if (iResult == 0) {
        int iMin = -1;
        int iMax = -1;
        double dMin = 1e100;
        double dMax = -1e100;
        for (int i = 0; i < iNumCells; i++) {
            if (pdArr[i] < dMin) {
                dMin = pdArr[i];
                iMin = i;
            }
            if (pdArr[i] > dMax) {
                dMax = pdArr[i];
                iMax = i;
            }
        }
        printf("min: %f @ %d; max %f @ %d\n", dMin, iMin, dMax, iMax);
    }
    if (iResult != 0) {
        delete[] pdArr;
        pdArr = NULL;
    }

    qdf_closeFile(hFile);
    return pdArr;
}


//----------------------------------------------------------------------------
// createDataMatrix
//
double **createDataMatrix(SurfaceGrid *pSG, double *pData, int iW, int iH, double dLonStart) {
    double **ppdData = NULL;
    
    Surface *pSurf = pSG->getSurface();
    SCellGrid *pCG = pSG->getCellGrid();

    // allocate
    ppdData = new double*[iH];
    for (int i = 0; i < iH; i++) {
        ppdData[i] = new double[iW];
    }

    double dDeltaLon = 360.0/(iW+1);
    double dDeltaLat = 180.0/(iH+1);
        
    double dCurLat = 90-dDeltaLat;
    for (int i = 0; i < iH; i++) {
        double dCurLon = dDeltaLon+dLonStart;
        for (int j = 0; j < iW; j++) {
            IcoFace *pF = dynamic_cast<IcoFace*>(pSurf->findFace(dCurLon*M_PI/180, dCurLat*M_PI/180));
           
            idtype ids[3];
            int    idx[3];
            double val[3];
            for (uint k = 0; k < 3; k++) {
                ids[k] = pF->getVertexID(k);
                idx[k] = pCG->m_mIDIndexes[ids[k]];
                val[k] = pData[idx[k]];
            }
            
            double d0;
            double d1;

            double dX1 = cos(dCurLon*M_PI/180)*cos(dCurLat*M_PI/180);    
            double dY1 = sin(dCurLon*M_PI/180)*cos(dCurLat*M_PI/180);    
            double dZ1 = sin(dCurLat*M_PI/180);    
            Vec3D P(dX1, dY1, dZ1);
            

            pF->calcBary(&P, &d0, &d1);
            double d2 = 1 - d0 -d1;

            ppdData[i][j] = d0*val[0] + d1*val[1] + d2*val[2];

            dCurLon += dDeltaLon;
        }
        dCurLat -= dDeltaLat;
    }
    return ppdData;
}


//----------------------------------------------------------------------------
// deleteMatrix
//
template<typename T>
void deleteMatrix(T **pData, int iW, int iH) {
    if (pData != NULL) {
        for (int i = 0; i < iH; i++) {
            delete[] pData[i];
        }
        delete[] pData;
    }
}


//----------------------------------------------------------------------------
// replacePat
//
std::string replacePat(const char *pPat, const char *pReplace) {
    std::string sPat = pPat;
    std::size_t iPos = sPat.find("###");
    if (iPos != std::string::npos) {
        sPat.replace(iPos, 3, pReplace);
    }
    return sPat;
}


//----------------------------------------------------------------------------
// writePNGFile
//
int writePNGFile(uchar **ppuData, int iW, int iH, const char *pOut) {
    int iResult = -1;
    
    PNGImage *pPI = new PNGImage(iW, iH);
    if (pPI != NULL) {
        printf("Writing data to [%s]\n", pOut);
        bool bOK = pPI->createPNGFromData(ppuData, pOut);
        if (bOK) {
            iResult = 0;
        } else {
            iResult = -1;
        }

        delete pPI;
    }

    return iResult;
}



//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char *sQDFGeoGrid  = NULL;
    char *sQDFData     = NULL;
    char *sSize        = NULL;
    char *sOutPat      = NULL;
    char *sArrayData   = NULL;
    char *sCompOp      = NULL;
    double dLonRoll    = -180.0;
    int  iTime         = -1;             

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(8,  
                               "-g:S!",   &sQDFGeoGrid,
                               "-q:S",    &sQDFData,
                               "-s:S!",   &sSize,
                               "-o:S!",   &sOutPat,
                               "-a:S!",   &sArrayData,
                               "-c:S",    &sCompOp,
                               "-r:d",    &dLonRoll,
                               "-t:i",    &iTime);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            int iW = -1;
            int iH = -1;
            
            printf("dLonRoll: %f\n", dLonRoll);
            printf("iTime:    %d\n", iTime);
            bOK = splitSizeString(sSize, &iW, &iH);
            if (bOK) {
                string_list vQDFs;
                vQDFs.push_back(sQDFGeoGrid);
                iResult = splitQDFs(sQDFData, vQDFs);
                if (iResult == 0) {
                    printf("QDF files:\n");
                    for (uint i = 0; i < vQDFs.size(); ++i) {
                        printf("%2u: %s\n", i, vQDFs[i].c_str());
                    }
                    array_data  mvArrayData;
                    lookup_data mvLookUpData;
                    data_order  vOrder;
                    iResult = splitArrayColors(sArrayData, mvArrayData, mvLookUpData, vOrder);
                    if (iResult == 0) {
                        which_data mvWhich;
                        iResult = checkConsistent(vQDFs, mvArrayData, mvWhich);
                        if (iResult == 0) {
                            int iNumCells = 0;
                            // we know this will work
                            hid_t hFile = qdf_openFile(sQDFGeoGrid);
                            hid_t hGrid = qdf_openGroup(hFile, GRIDGROUP_NAME);
                            iResult = qdf_extractAttribute(hGrid, GRID_ATTR_NUM_CELLS, 1, &iNumCells);
                            qdf_closeGroup(hGrid);
                            qdf_closeFile(hFile);

                            SurfaceGrid *pSG = SurfaceGrid::createInstance(sQDFGeoGrid);

                            if (pSG != NULL) {

                                AlphaComposer *pAC = AlphaComposer::createInstance(iW, iH);
                                if (pAC != NULL) {


                                    for (uint i = 0; i < vOrder.size(); i++) {
                                        // extract data
                                        arrind aiNameIndex   = vOrder[i];
                                        const char *pName = aiNameIndex.first.c_str();
                                        const ds_info dsCur  =  mvArrayData[aiNameIndex];
                                        double *pData = NULL;
                                        if ((mvWhich[aiNameIndex] >= 0) && (mvWhich[aiNameIndex] < (int) vQDFs.size()))  {
                                            pData = extractData(vQDFs[mvWhich[aiNameIndex]].c_str(), iNumCells, dsCur);
                                        } else {
                                            printf("What? whichvalue %d  -- this should not happen!\n", mvWhich[aiNameIndex]);
                                            iResult = -1;
                                        }
                            

                                        if (pData != NULL) {
                                        
                                            double **pdDataMatrix = createDataMatrix(pSG, pData, iW, iH, dLonRoll);

                                            pAC->addMatrix(pdDataMatrix, mvLookUpData[aiNameIndex]);
                                            if (sCompOp == NULL) {
                                            
                                                // no compositing: create png data and write to file
                                                char sFullName[512];
                                                sprintf(sFullName, "%s_%d", pName, mvWhich[aiNameIndex]);
                                                std::string sOutName = replacePat(sOutPat, sFullName);
                                            
                                                iResult = writePNGFile(pAC->getData(), iW, iH, sOutName.c_str());
                                            
                                                if (iResult == 0) {
                                                    printf("success for [%s]\n", pName);
                                                } else {
                                                    printf("Image creation failed\n");
                                                }
                                                pAC->clear();
                                            }

                                            deleteMatrix(pdDataMatrix, iW, iH);
                                            delete[] pData;
                                        }
                                    } // end loop
                                
                                    if (sCompOp != NULL) {
                                        // add a time layer to the image data
                                        uchar **ppTimeData = NULL;
                                        if (iTime >= 0) {
                                            TextRenderer *pTR = TextRenderer::createInstance(iW, iH);
                                            if (pTR != NULL) {
                                                pTR->setFontSize(24);
                                                pTR->setColor(1, 1, 1, 1);
                                                char sText[512];
                                                sprintf(sText, "%03dky", iTime);
                                                pTR->addText(sText, 5, iH-5);
                                            
                                                ppTimeData = pTR->createData();

                                                pAC->addPNGData(ppTimeData);
                                            
                                                //pTR->writeToPNG("glomp.png");
                                                pTR->deleteArray(ppTimeData);

                                                delete pTR;
                                            } else {
                                                printf("Couldn't create TextRenderer\n");
                                                iResult = -1;
                                            }
                                        }
                                        std::string sOutName = replacePat(sOutPat, "merge");
                                
                                        // save the composite
                                        iResult = writePNGFile(pAC->getData(), iW, iH, sOutName.c_str());
                                
                                        if (iResult == 0) {
                                            printf("success for merge\n");
                                        } else {
                                            printf("Image creation failed\n");
                                        }
                                    }
                                    delete pAC;
                                } else {
                                    printf("Couldn't create AlphaComposer\n");
                                    iResult = -1;
                                }

                                if (iResult == 0) {
                                    printf("+++ success +++\n");
                                } else {
                                    printf("--- failure ---\n");
                                }

                                delete pSG;
                            } else {
                                printf("Couldn't create SurfaceGrid\n");
                            }
                        } else {
                            // error already printed in checkConsistent
                        }
                    } else {
                        // error already printed in splitArrayColors
                    }
             
                    cleanUp(mvLookUpData);
                } else {
                    printf("Couldn't split qdf data\n");
                    iResult = -1;
                }

            } else {
                printf("Couldn't split size string\n");
                iResult = -1;
            }
        } else {
            printf("ParamReader result: %d\n", iResult);
            printf("%s: %s %s\n", pPR->getErrorMessage(iResult).c_str(),  pPR->getBadArg().c_str(), pPR->getBadVal().c_str());
            usage(apArgV[0]);
        }
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
