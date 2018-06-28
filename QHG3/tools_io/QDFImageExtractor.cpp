#include "string.h"

#include "QDFImageExtractor.h"

#include "strutils.h"
#include "Vec3D.h"
#include "QDFUtils.h"
#include "QDFArray.h" 
#include "SCellGrid.h"

#include "LookUp.h"
#include "RainbowLookUp.h"
#include "Rainbow2LookUp.h"
#include "GeoLookUp.h"
#include "Geo2LookUp.h"
#include "TwoToneLookUp.h"
#include "FadeOutLookUp.h"
#include "PNGImage.h"

#include "IcoFace.h"
#include "Surface.h"
#include "SurfaceGrid.h"
#include "TextRenderer.h"
#include "AlphaComposer.h"

//----------------------------------------------------------------------------
// createInstance
//
QDFImageExtractor *QDFImageExtractor::createInstance(SurfaceGrid *pSG, 
                                                     char        *sQDFGrid, 
                                                     string_list &vQDFs, 
                                                     char        *sArrayData, 
                                                     img_prop    &ip, 
                                                     bool         bVerbose) {
    QDFImageExtractor *pQIE = new QDFImageExtractor();
    int iResult = pQIE->init(pSG, sQDFGrid, vQDFs, sArrayData, ip, bVerbose);
    if (iResult != 0) {
        delete pQIE;
        pQIE = NULL;
    }
    return pQIE;
}


//----------------------------------------------------------------------------
// constructor
//
QDFImageExtractor::QDFImageExtractor() 
    : m_iW(0),
      m_iH(0),
      m_dLonRoll(0),
      m_iNumLayers(0),
      m_iNumCells(0),
      m_pAC(NULL),
      m_bVerbose(false) {


}

//----------------------------------------------------------------------------
// destructor
//
QDFImageExtractor::~QDFImageExtractor() {
    
    if (m_pAC != NULL) {
        delete m_pAC;
    }

    cleanUpLookUps();
}

    
//----------------------------------------------------------------------------
// init
//
int QDFImageExtractor::init(SurfaceGrid *pSG, 
                            char        *sQDFGrid, 
                            string_list &vQDFs, 
                            char        *sArrayData, 
                            img_prop    &ip,
                            bool         bVerbose) {
    int iResult = 0;

    m_pSG   = pSG;
    m_vQDFs = vQDFs;
    m_iNumLayers = m_vQDFs.size();

    m_iW = ip.iW;
    m_iH = ip.iH;
    m_dLonRoll = ip.dLonRoll;
    
    m_bVerbose = bVerbose;
    iResult = splitArrayColors(sArrayData);
    if (iResult == 0) {
        iResult = checkConsistent();
        if (iResult == 0) {

            // we know this will work
            hid_t hFile = qdf_openFile(sQDFGrid);
            hid_t hGrid = qdf_openGroup(hFile, GRIDGROUP_NAME);
            iResult = qdf_extractAttribute(hGrid, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells);
            qdf_closeGroup(hGrid);
            qdf_closeFile(hFile);
            
            if (iResult == 0) {
                m_pAC = AlphaComposer::createInstance(m_iW, m_iH);
                if (m_pAC != NULL) {
                } else {
                    printf("Couldn't create AlphaComposer\n");
                    iResult = -1;
                }
            } else {
                printf("couldn't extrac attribute [%s] from [%s]\n", GRID_ATTR_NUM_CELLS, sQDFGrid);
            }
        } else {
            printf("checkConsistent failed\n");
            // error already printed in checkConsistent
        }
    } else {
        printf("splitArrayColors failed\n");
        // error already printed in splitArrayColors
    }

    return iResult;
}



//----------------------------------------------------------------------------
// checkArrayName
//
int QDFImageExtractor::checkArrayName(arrind &aiNameIndex) {
    int iCode = ARR_CODE_NONE;
    const char *pName = aiNameIndex.first.c_str();
    if (strcmp(pName, "lon") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LONGITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LON;
    } else if (strcmp(pName, "lat") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LATITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LAT;
    } else if (strcmp(pName, "alt") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ALTITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_ALT;
    } else if (strcmp(pName, "ice") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ICE_COVER, DS_TYPE_CHAR);
        iCode = ARR_CODE_ICE;
    } else if (strcmp(pName, "water") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_WATER, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_WATER;
    } else if (strcmp(pName, "coastal") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_COASTAL, DS_TYPE_CHAR);
        iCode = ARR_CODE_COAST;
    } else if (strcmp(pName, "temp") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_MEAN_TEMP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TEMP;
    } else if (strcmp(pName, "rain") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_TOT_RAIN, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_RAIN;
    } else if (strcmp(pName, "npp") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (strcmp(pName, "npp_b") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_BASE_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (strcmp(pName, "dist") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(MSTATGROUP_NAME, MSTAT_DS_DIST, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_DIST;
    } else if (strcmp(pName, "time") == 0)  {
        m_mvArrayData[aiNameIndex] = ds_info(MSTATGROUP_NAME, MSTAT_DS_TIME, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TIME;
    } else if (strncmp(pName, "pop_", 4) == 0)  {
        const char *p1 = pName + 4;
        char sGroup[512];
        sprintf(sGroup, "%s/%s", POPGROUP_NAME, p1);

        m_mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, p1, AGENT_DATASET_NAME, DS_TYPE_POP);
        //mvArrayData[pName] = ds_info(sGroup, AGENT_DATASET_NAME, DS_TYPE_POP);
        iCode = ARR_CODE_POP;
    } else {
        iCode = ARR_CODE_NONE;
    }
    

    if (iCode != ARR_CODE_NONE) {
        if (m_bVerbose) {
            printf("added ARR [%s@%d] -> Group [%s], dataset [%s], type %d\n", 
                   aiNameIndex.first.c_str(), 
                   aiNameIndex.second, 
                   m_mvArrayData[aiNameIndex].sGroup.c_str(), 
                   m_mvArrayData[aiNameIndex].sDataSet.c_str(), 
                   m_mvArrayData[aiNameIndex].iDataType);
        }
    }
    return iCode;
}


//----------------------------------------------------------------------------
// splitArraySpec
//
int QDFImageExtractor::splitArraySpec(char *sArraySpec) {
    int iResult = -1;
    char *pCtx;
    char *pName = strtok_r(sArraySpec, "|", &pCtx);
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

        int iArrCode = checkArrayName(aiNameIndex);
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
                m_mvLookUpData[aiNameIndex] = pLU;
                if (m_bVerbose) {
                    printf("added LU  [%s@%d] -> [%p]\n", aiNameIndex.first.c_str(), aiNameIndex.second, pLU);
                }

                if (pLU != NULL) {
                    m_vOrder.push_back(aiNameIndex);
                    iResult = 0;
                } else {
                    iResult =-1;
                }
            } else {
                if (m_mvArrayData.size() == 0) {
                    printf("No lookupname given\n");
                    iResult = -1;
                }
            }
        } else {
            printf("Unknown arrayname [%s]\n", pName);
            iResult = -1;
        }
    } else {
        printf("Got empty name\n");
        iResult = -1;
    }
    
    return iResult; 
}


//----------------------------------------------------------------------------
// splitArrayColors
//
int QDFImageExtractor::splitArrayColors(const char *sArrayData) {
    int iResult = 0;
    char *pCtx;
    char *pCopy = new char[strlen(sArrayData)+1];
    strcpy(pCopy, sArrayData);

    char *pSpec = strtok_r(pCopy, ",", &pCtx);
    while ((iResult == 0) && (pSpec != NULL)) {
        iResult = splitArraySpec(pSpec);
        pSpec = strtok_r(NULL, ",", &pCtx);
    }
    delete pCopy;
    return iResult;
}


//----------------------------------------------------------------------------
// checkConsistent
//
int QDFImageExtractor::checkConsistent() {
    int iResult = 0;

    
    hid_t  hQDFGeoGrid = qdf_openFile(m_vQDFs[0].c_str());
    if (hQDFGeoGrid != H5P_DEFAULT) {
        hid_t hGrid = qdf_openGroup(hQDFGeoGrid, GRIDGROUP_NAME);
        if (hGrid != H5P_DEFAULT) {
            qdf_closeGroup(hGrid);
        } else {
            printf("No grid group in [%s]\n", m_vQDFs[0].c_str());
            iResult =-1;
        }
        qdf_closeFile(hQDFGeoGrid);

        array_data::const_iterator it;
        for (it = m_mvArrayData.begin(); (iResult == 0) && (it != m_mvArrayData.end()); ++it) {
            bool bSearching = true;
            for (uint i = 0; bSearching && (i < m_vQDFs.size()); i++) {
                hid_t hQDFData = qdf_openFile(m_vQDFs[i].c_str());
                if (hQDFData != H5P_DEFAULT) {
                    if (checkGroup(hQDFData, it->second)) {

                        if ((it->first.second < 0) || (it->first.second == (int)i)) {
                            if (m_bVerbose) {
                                printf("%s: using Group [%s%s%s], Dataset [%s] in [%s]\n", 
                                       it->first.first.c_str(), 
                                       it->second.sGroup.c_str(),  
                                       (it->second.sSubGroup != "")?"/":"",
                                       it->second.sSubGroup.c_str(),  
                                       it->second.sDataSet.c_str(), 
                                       m_vQDFs[i].c_str());
                            }
                            bSearching = false;
                            m_mvWhich[it->first] = i;
                        } 
                    } 
                    qdf_closeFile(hQDFData);
                } else {
                    printf("Couldn't open [%s]\n", m_vQDFs[i].c_str());
                    iResult = -1;
                }
            }

            if (bSearching) {
                m_mvWhich[it->first] = -1;
                iResult = -1;
                printf("Group [%s], Dataset [%s] not found in any of the QDF files\n", 
                       it->second.sGroup.c_str(),  
                       it->second.sDataSet.c_str());
            } else {
                
                iResult = 0;
            }
        }


    } else {
        printf("Couldn't open grid group in [%s]\n", m_vQDFs[0].c_str());
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// checkGroup
//
bool QDFImageExtractor::checkGroup(hid_t hFile, const ds_info &rdInfo) {
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
// extractData
//
double *QDFImageExtractor::extractData(const char *pQDF, const ds_info &pGroupDS) {
    int iResult = -1;
    double *pdArr  = NULL;
    char   *pdArr1 = NULL;
 
    // get number of elements
    hid_t hFile = qdf_openFile(pQDF);

    // we know this will work, too
    hid_t hGroup = qdf_openGroup(hFile, pGroupDS.sGroup.c_str());
        
    pdArr = new double[m_iNumCells];
        
    switch (pGroupDS.iDataType) {
    case DS_TYPE_DOUBLE: 
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet.c_str(), m_iNumCells, (double *)pdArr);
        break;

    case DS_TYPE_CHAR:
        pdArr1 = new char[m_iNumCells];
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet.c_str(), m_iNumCells, (char *)pdArr1);
        if (iResult == 0) {
            for (int i = 0; i < m_iNumCells; i++) {
                pdArr[i] = pdArr1[i];
            }
        }
        delete[] pdArr1;
        break;

    case DS_TYPE_POP: {
        memset(pdArr, 0, m_iNumCells*sizeof(double));
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
        for (int i = 0; i < m_iNumCells; i++) {
            if (pdArr[i] < dMin) {
                dMin = pdArr[i];
                iMin = i;
            }
            if (pdArr[i] > dMax) {
                dMax = pdArr[i];
                iMax = i;
            }
        }
        if (m_bVerbose) {
            printf("%s: min: %f @ %d; max %f @ %d\n", pGroupDS.sDataSet.c_str(), dMin, iMin, dMax, iMax);
        }
    }
    if (iResult != 0) {
        delete[] pdArr;
        pdArr = NULL;
    }

    qdf_closeFile(hFile);
    return pdArr;
}


//----------------------------------------------------------------------------
// createLookUp
//
LookUp *QDFImageExtractor::createLookUp(char *pLUName, string_list &vParams) {
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

    } else if (strcmp(pLUName, "geo2") == 0) {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0].c_str(), &dMin)) {
                if (strToNum(vParams[1].c_str(), &dSea)) {
                    if (strToNum(vParams[2].c_str(), &dMax)) {
                        pLU = new Geo2LookUp(dMin, dSea, dMax);
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
// createLookUp
//
int QDFImageExtractor::addTimeLayer(int iTime) {
    int iResult = 0;

    uchar **ppTimeData = NULL;
    if (iTime >= 0) {
        TextRenderer *pTR = TextRenderer::createInstance(m_iW, m_iH);
        if (pTR != NULL) {
            pTR->setFontSize(24);
            pTR->setColor(1, 1, 1, 1);
            char sText[512];
            sprintf(sText, "%05.1fky", iTime/1000.0);
            pTR->addText(sText, 5, m_iH-5);
            
            ppTimeData = pTR->createData();

            m_pAC->addPNGData(ppTimeData);
            
            //pTR->writeToPNG("glomp.png");
            pTR->deleteArray(ppTimeData);
            
            delete pTR;
        } else {
            printf("Couldn't create TextRenderer\n");
            iResult = -1;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// extractAll
//
int QDFImageExtractor::extractAll(const char *sOutPat, const char *sCompOp, int iTime) {
    int iResult = -1;
    
    LoopLayers(sOutPat, sCompOp);
    if (sCompOp != NULL) {
        addTimeLayer(iTime);
        
        // save the composite
        iResult = writePNGFile(sOutPat, "merge");
                                    
        if (iResult == 0) {
            if (m_bVerbose) {
                printf("success for merge\n");
            }
        } else {
            printf("Image creation failed\n");
        }
    }
    return iResult;                                
}


//----------------------------------------------------------------------------
// LoopLayers
//
int QDFImageExtractor::LoopLayers(const char *sOutPat, const char *sCompOp) {
    int iResult = 0;

    for (uint i = 0; (iResult == 0) && (i < m_vOrder.size()); i++) {
        // extract data
        arrind aiNameIndex   = m_vOrder[i];
        const char *pName    = aiNameIndex.first.c_str();
        const ds_info dsCur  = m_mvArrayData[aiNameIndex];
        double *pData = NULL;
        if ((m_mvWhich[aiNameIndex] >= 0) && (m_mvWhich[aiNameIndex] < m_iNumLayers))  {
            pData = extractData(m_vQDFs[m_mvWhich[aiNameIndex]].c_str(), dsCur);
        } else {
            printf("What? whichvalue %d  -- this should not happen!\n", m_mvWhich[aiNameIndex]);
            iResult = -1;
        }


        if (pData != NULL) {
                        
            double **pdDataMatrix = createDataMatrix(pData);
            
            m_pAC->addMatrix(pdDataMatrix, m_mvLookUpData[aiNameIndex]);
            if (sCompOp == NULL) {
                            
                // no compositing: create png data and write to file
                char sFullName[512];
                sprintf(sFullName, "%s_%d", pName, m_mvWhich[aiNameIndex]);
                
                iResult = writePNGFile(sOutPat, sFullName);
                            
                if (iResult == 0) {
                    if (m_bVerbose) {
                        printf("success for [%s]\n", pName);
                    }
                } else {
                    printf("Image creation failed\n");
                }
                m_pAC->clear();
            }
                        
            deleteMatrix(pdDataMatrix);
            delete[] pData;
        } else {
            // unknown data type or so
            iResult = -1;
        }
    } // end loop



    return iResult;
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
bool QDFImageExtractor::extractColors(const char *pColorDesc, uchar *pR, uchar *pG, uchar *pB, uchar *pA) {
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
// writePNGFile
//
int QDFImageExtractor::writePNGFile(const char *pPat, const char *pReplace) {
    int iResult = -1;
    
    std::string sPat = pPat;
    std::size_t iPos = sPat.find("###");
    if (iPos != std::string::npos) {
        sPat.replace(iPos, 3, pReplace);
    }

    PNGImage *pPI = new PNGImage(m_iW, m_iH);
    if (pPI != NULL) {
        if (m_bVerbose) {
            printf("Writing data to [%s]\n", sPat.c_str());
        }
        bool bOK = pPI->createPNGFromData(m_pAC->getData(), sPat.c_str());
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
// createDataMatrix
//
double **QDFImageExtractor::createDataMatrix(double *pData) {
    double **ppdData = NULL;
    
    Surface *pSurf = m_pSG->getSurface();
    SCellGrid *pCG = m_pSG->getCellGrid();

    // allocate
    ppdData = new double*[m_iH];
    for (int i = 0; i < m_iH; i++) {
        ppdData[i] = new double[m_iW];
    }

    double dDeltaLon = 360.0/(m_iW+1);
    double dDeltaLat = 180.0/(m_iH+1);
        
    double dCurLat = 90-dDeltaLat;
    for (int i = 0; i < m_iH; i++) {
        double dCurLon = dDeltaLon+m_dLonRoll;
        for (int j = 0; j < m_iW; j++) {
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
// cleanUpLookUps
//
void QDFImageExtractor::cleanUpLookUps() {
    lookup_data::const_iterator it;
    for (it = m_mvLookUpData.begin(); it != m_mvLookUpData.end(); ++it) {
        delete it->second;
    }
}

//----------------------------------------------------------------------------
// deleteMatrix
//
void QDFImageExtractor::deleteMatrix(double **pData) {
    if (pData != NULL) {
        for (int i = 0; i < m_iH; i++) {
            delete[] pData[i];
        }
        delete[] pData;
    }
}
