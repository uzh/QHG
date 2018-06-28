#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "GeoInfo.h"
#include "geomutils.h"
#include "strutils.h"
#include "SCell.h"
#include "Geography.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "VertexLinkage.h"
#include "EQsahedron.h"
#include "SCellGrid.h"
#include "ValReader.h"
#include "MessLogger.h"
#include "GridFactory.h"
#include "SeasonProvider.h"
#include "QMapUtils.h"

#define KEY_DDIR      0
#define KEY_GRID      1
#define KEY_ALT       2
#define KEY_ICE       3
#define KEY_RAIN      4
#define KEY_TEMP      5
#define KEY_DENS      6
#define KEY_ANPP      7

//-----------------------------------------------------------------------------
// constructor
//
GridFactory::GridFactory(const char *sDefFile) 
    : m_iNumCells(0),
      m_pCG(NULL),
      m_pGeo(NULL),
      m_pClimate(NULL),
      m_pVeg(NULL),
      m_pLine(sDefFile) {
    
    m_pLR = LineReader_std::createInstance(sDefFile, "rt");
    if (m_pLR != NULL) {
        m_pLine = NULL;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
GridFactory::GridFactory() 
    : m_iNumCells(0),
      m_pCG(NULL),
      m_pGeo(NULL),
      m_pClimate(NULL),
      m_pVeg(NULL),
      m_pLR(NULL) {
    

}

//-----------------------------------------------------------------------------
// destructor
//
GridFactory::~GridFactory() {

    // the GridFactory creates the cell grid, geo and climate
    // but does not delete them - they are used elsewhere
    if (m_pLR != NULL) {
        delete m_pLR;
    }
}

//-----------------------------------------------------------------------------
// createEmptyQDF
//
int GridFactory::createEmptyQDF(const char*pIGNFile) {
    int iResult = 0;
  
    printf("Creating IGN from [%s]...\n", pIGNFile);
    IcoGridNodes *pIGN = new IcoGridNodes();
    iResult = pIGN->read(pIGNFile);

    if (iResult == 0) {
        m_iNumCells = (uint)pIGN->m_mNodes.size();
    } else {
        iResult = -1;
        printf("The file [%s] is not an IGN file\n", pIGNFile); 
    }

    if (iResult == 0) {
        printf("Creating CellGrid...\n");
        m_pCG = new SCellGrid(0, m_iNumCells, pIGN->getData());
        iResult = createCells(pIGN);
    }
    if (iResult == 0) {
        printf("Initializing Geography...\n");
        geonumber dRadius = 6371.3;
        int iMaxNeigh     = 6;
        m_pGeo = new Geography(m_iNumCells, iMaxNeigh, dRadius);  // create geography
        iResult = initializeGeography(pIGN);
    }
    if (iResult == 0) {
        int iNumSeasons = 0;
        m_pClimate = new Climate(m_iNumCells, iNumSeasons, m_pGeo);
        int iNumVegSpecies = 0;
        m_pVeg = new Vegetation(m_iNumCells, iNumVegSpecies, m_pGeo, m_pClimate);
    }
    if (iResult == 0) {
        m_pCG->setGeography(m_pGeo);
        m_pCG->setClimate(m_pClimate);
        m_pCG->setVegetation(m_pVeg);
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// setDataDirs
//
int GridFactory::setDataDirs(const char *pDataDirs) {
    char sDataDirs[MAX_PATH];
    strcpy(sDataDirs, pDataDirs);
    char *p = strtok(sDataDirs, ",;");
    while (p != NULL) {
        std::string s = p;
        if (p[strlen(p)-1] != '/') {
            s += '/';
        }
        m_vDataDirs.push_back(s);
        printf("added data dir [%s]\n", s.c_str());
        p = strtok(NULL, ",;");
    }
    return 0;
}
//----------------------------------------------------------------------------
// exists
//  if pFile exists in current directory, 
//    true is returned and pExists is set to pFile
//  otherwise, if data dirs are given, and pFile exists in a data dir,
//    true is returned and pExists is set to DataDirX/pFile, 
//    where DataDirX is the first directory in the list containg 
//    a file named pFile 
//  otherwise,
//    false is returned  
//
bool GridFactory::exists(const char *pFile, char *pExists) {
    struct stat statbuf;
    
    int iResult = stat(pFile, &statbuf);
    //    printf("Errore1: [%s]\n", strerror(iResult));
    if (iResult != 0)  {
        if ((pExists != NULL) && !m_vDataDirs.empty()) {
            char sTest[MAX_PATH];
            *pExists = '\0';
            for (uint i = 0; (*pExists ==  '\0') && (i < m_vDataDirs.size()); i++) {
                sprintf(sTest, "%s%s", m_vDataDirs[i].c_str(), pFile);
                iResult = stat(sTest, &statbuf);
                if (iResult == 0) {
                    strcpy(pExists, sTest);
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
        } 
    } else {
        strcpy(pExists, pFile);
    }
    if (iResult == 0) {
        printf("[GridFactory::exists] [%s] -> [%s]\n", pFile, pExists);
    } else {
        *pExists = '\0';
        printf("[GridFactory::exists] [%s] not found\n", pFile);
    }
    
    return (0 == iResult);
}


//-----------------------------------------------------------------------------
// collectLines
//
int GridFactory::collectLines(std::string *pLineList) {
    int iResult = -1;
    if (m_pLR != NULL) {
        while (!m_pLR->isEoF()) {
            char *pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {
                if (strstr(pLine, "DATA_DIR") == pLine) {
                    pLineList[KEY_DDIR] = pLine;
                    iResult = 0;
                } else if (strstr(pLine, "GRID_TYPE") == pLine) {
                    pLineList[KEY_GRID] = pLine;
                    iResult = 0;
                } else if ((strstr(pLine, "ALT") == pLine) ||
                           (strstr(pLine, "ALTITUDE") == pLine)) {
                    pLineList[KEY_ALT] = pLine;
                } else if (strstr(pLine, "ICE") == pLine) { 
                    pLineList[KEY_ICE] = pLine;
                } else if (strstr(pLine, "TEMP") == pLine) { 
                    pLineList[KEY_TEMP] = pLine;
                } else if (strstr(pLine, "RAIN") == pLine) {
                    pLineList[KEY_RAIN] = pLine;
                    /*** old veg
                } else if (strstr(pLine, "DENS") == pLine) {
                    pLineList[KEY_DENS] = pLine;
                    ***/
                } else if (strstr(pLine, "ANPP") == pLine) {
                    pLineList[KEY_ANPP] = pLine;
                }
            }
        }
    } else {
        if (strstr(m_pLine, "GRID_TYPE") == m_pLine) {
            pLineList[KEY_GRID] = m_pLine;
            iResult = 0;
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// createEQGrid
//    pVL = EQsahedron::getLinkage()
//
IcoGridNodes *createEQGrid(VertexLinkage *pVL) {
    
    IcoGridNodes *pIGN = new IcoGridNodes();
    
    std::map<gridtype, Vec3D *>::const_iterator iti;
    for (iti = pVL->m_mI2V.begin(); iti != pVL->m_mI2V.end(); iti++) {
        Vec3D *pV = iti->second;

        // calc theta and phi
        double dPhi = 0;
        double dTheta = 0;
        if (pV != NULL) {
            double dArea = 0; // get it from a vertexlinkage thingy
            cart2Sphere(pV, &dTheta, &dPhi);
            IcoNode *pIN = new IcoNode(iti->first, dTheta, dPhi, dArea);
            intset vLinks = pVL->getLinksFor(iti->first);
            if (vLinks.size() > 6) {
                intset::iterator st;
                printf("Vertex #%d (%f,%f) has %zd links\n", iti->first, pV->m_fX, pV->m_fY,vLinks.size());
                for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                    printf("  %d", *st);
                }
                printf("\n");
            }
            
                    
            intset::iterator st;
            for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                Vec3D *pN = pVL->getVertex(*st);
                double dDist = spherdist(pV, pN, 1.0);
                pIN->m_dArea = pVL->calcArea(pIN->m_lID);

                pIN->addLink(*st, dDist);

            }
            pIGN->m_mNodes[iti->first] = pIN;
        } else {
            pIGN = NULL;
        }
    }

    return pIGN;
}


//-----------------------------------------------------------------------------
// setGrid
//
int GridFactory::setGrid(const char *pLine) {
    int iResult = -1;
    stringmap smSurfaceData;
    

    bool bFlat = false;
    bool bHex = false;

    bool bPeriodic = false;
    char sPeriodic[16];
    memset(sPeriodic, 0, 16);

    int iW = 0;
    int iH = 0;
    IcoGridNodes* pIGN = NULL;
    char sPar1[SHORT_INPUT];
    int iSubDivs = -1;
    int iGridType = GRID_TYPE_NONE;
    // selecting grid type
    
    if (sscanf(pLine, "GRID_TYPE ICO %s", sPar1) == 1) {
        iGridType = GRID_TYPE_ICO;
        char sRealFile[256];
        if (exists(sPar1, sRealFile)) {
            pIGN = new IcoGridNodes();
            iResult = pIGN->read(sRealFile);
            if(iResult==0) {
                m_iNumCells = (uint)pIGN->m_mNodes.size();
            } else {
                iResult = -1;
                printf("The file [%s] is not an IGN file\n", sRealFile); 
            }
        } else {
            // let see if it is a number
            if (strToNum(sPar1, &iSubDivs)) {
                EQsahedron *pEQNodes = EQsahedron::createInstance(iSubDivs, true, NULL);
                pEQNodes->relink();
                VertexLinkage *pVL = pEQNodes->getLinkage();
                pIGN = createEQGrid(pVL);
                m_iNumCells = (uint)pIGN->m_mNodes.size();

                //delete pEQNodes;
                iResult = 0;
            } else {
                iResult = -1;
                printf("The file [%s] (or [%s]) does not exist\n", sPar1, sRealFile); 
            }
        }
    } else if (sscanf(pLine, "GRID_TYPE %s %ix%i %s", sPar1, &iW, &iH, sPeriodic) == 4) {
            
        bFlat = true;
        if (!strcmp(sPeriodic, "PERIODIC")) {
            bPeriodic = true;
        } else {
            bPeriodic = false;
        }
        m_iNumCells = iW * iH;
        iResult = 0;
        
    } else if (sscanf(pLine, "GRID_TYPE %s %ix%i", sPar1, &iW, &iH) == 3) {
        
        bFlat = true;
        bPeriodic = false;
        m_iNumCells = iW * iH;
        iResult = 0;
        
    } else {
        fprintf(stderr,"[GridFactory:readDef] don't understand %s\n",pLine);
    }


    // connectivity check for flat grids
    if ((iResult == 0) && bFlat) {
        if (!strcmp(sPar1,"HEX")) {
            iGridType = GRID_TYPE_FLAT6;
            bHex = true;
            if ((iH%2) == 1) {
                fprintf(stderr,"[GridFactory::readDef] you need an even number of rows for a HEX grid\n");
                iResult = -1;
            }
        } else if (!strcmp(sPar1,"RECT")) {
            iGridType = GRID_TYPE_FLAT4;
            bHex = false;
        } else {
            fprintf(stderr,"[GridFactory::readDef] option %s not recognized\n", sPar1);
            iResult = -1;
        }
    }

    if (iResult == 0) {
        if (bFlat) {
            char sW[8];
            char sH[8];
            sprintf(sW, "%d", iW);
            sprintf(sH, "%d", iH);

            smSurfaceData[SURF_TYPE] = "LTC";
            smSurfaceData[SURF_LTC_W] = sW;
            smSurfaceData[SURF_LTC_H] = sH;
            smSurfaceData[SURF_LTC_LINKS] = (iGridType==GRID_TYPE_FLAT6)?"6":"4";
            smSurfaceData[SURF_LTC_PERIODIC] = bPeriodic?"1":"0";

            char* sPTData = new char[256];
            sprintf(sPTData, "%d [Linear] %f %f %d",
                    PR_LINEAR, 0.0, 0.0, 0);
            smSurfaceData[SURF_LTC_PROJ_TYPE] = sPTData;

            char* sPGData = new char[256];
            double dW = (bHex) ? (iW - 0.5) : (iW - 1);
            double dH = (bHex) ? (iH - 1) * 0.8660254 : (iH - 1);
            sprintf(sPGData,"%d %d %lf %lf %lf %lf %lf",
                    iW-1, iH-1, dW, dH, 0.0, 0.0, 1.0);
            smSurfaceData[SURF_LTC_PROJ_GRID] = sPGData;

            m_pCG = new SCellGrid(0, m_iNumCells, smSurfaceData);
            // initialize the cells depending for flat grids
            iResult = createCells(iW, iH, bPeriodic, bHex);
        } else {
            char sN[8];
            smSurfaceData[SURF_TYPE] = SURF_EQSAHEDRON;
            sprintf(sN, "%d", iSubDivs);
            smSurfaceData[SURF_IEQ_SUBDIVS] = sN;
            pIGN->setData(smSurfaceData);
        // here the SCellGrid class is created
            //        m_pCG = new SCellGrid(0, m_iNumCells, iGridType, iW, iH, bPeriodic);
            m_pCG = new SCellGrid(0, m_iNumCells, pIGN->getData());
            iResult = createCells(pIGN);
        }
    }
    if (iResult == 0) {
        geonumber dRadius = bFlat ? 0.0 : 6371.3;
        int iMaxNeigh = bFlat ? (bHex?6:4) : 6;
        m_pGeo = new Geography(m_iNumCells, iMaxNeigh, dRadius);  // create geography
        m_pCG->setGeography(m_pGeo);
        if (bFlat) {
            initializeGeography(iW, iH, bHex);
        } else {
            initializeGeography(pIGN);
        }
        m_pClimate = new Climate(m_iNumCells, 0, m_pGeo);
        m_pCG->setClimate(m_pClimate);
        m_pVeg = new Vegetation(m_iNumCells, 0, m_pGeo, m_pClimate);
        m_pCG->setVegetation(m_pVeg);

    }
    delete pIGN;
    return iResult;
}


//-----------------------------------------------------------------------------
// readDef
//
int GridFactory::readDef() {
    int iResult = -1;
    bool bInterpol = false;;
    std::string sLineMap[8];
    char sPar1[SHORT_INPUT];

    iResult=collectLines(sLineMap);
    printf("Collected lines: %d\n", iResult);

    if (iResult == 0) {
        //        printf("sLineMap[KEY_DDIR]: [%s]\n", sLineMap[KEY_DDIR].c_str());
        char sDataDirs[1024];
        *sDataDirs = '\0';
        if (sLineMap[KEY_DDIR].length() > 0) {
            int iRead = sscanf(sLineMap[KEY_DDIR].c_str(), "DATA_DIR %s", sDataDirs);
            printf("setting data dir to [%s]\n", sDataDirs);
            if ((iRead == 1) && (*sDataDirs != '\0')) {
                setDataDirs(sDataDirs);
            }
        }
    }

    if (iResult == 0) {
        if (sLineMap[KEY_GRID].length() > 0) {
            printf("creating grid [%s]\n", sLineMap[KEY_GRID].c_str());
            iResult = setGrid(sLineMap[KEY_GRID].c_str());
            if (iResult == 0) {
                printf("Grid OK\n");
                bInterpol = (m_pCG->m_iType==GRID_TYPE_ICO) || (m_pCG->m_iType==GRID_TYPE_IEQ);
            }
        }
    }


    
    if (iResult == 0) {
        if (sLineMap[KEY_ALT].length() > 0) {
            printf("sLineMap[KEY_ALT]: [%s]\n", sLineMap[KEY_ALT].c_str());
	    double dAlt;
            const char *pLine = sLineMap[KEY_ALT].c_str();
            if (sscanf(pLine, "ALT QMAP %s", sPar1) == 1 ||
                sscanf(pLine, "ALTITUDE QMAP %s", sPar1) == 1) {
                char sRealFile[512];
                if (exists(sPar1, sRealFile)) {
                    ValReader *pVRAlt  = QMapUtils::createValReader(sRealFile, bInterpol);  // prepare to read
                    iResult = setAltitude(pVRAlt);
                    delete pVRAlt;
                } else {
                    iResult = -1;
                    printf("File [%s] does not exist\n", sRealFile);
                }
	    } else if (sscanf(pLine, "ALT FLAT %lf", &dAlt) == 1) { 
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pGeo->m_adAltitude[i]    = dAlt;
                    }
            } else {
                printf("Invalid ALT line [%s]\n", pLine);
                iResult =-1;
            }
        }
    }
    if (iResult == 0) {
        if (sLineMap[KEY_ICE].length() > 0) {
            printf("sLineMap[KEY_ICE]: [%s]\n", sLineMap[KEY_ICE].c_str());
            const char *pLine = sLineMap[KEY_ICE].c_str();
            if (sscanf(pLine, "ICE QMAP %s", sPar1) == 1) {
                char sRealFile[512];
                if (exists(sPar1, sRealFile)) {
                    ValReader *pVRIce  = QMapUtils::createValReader(sRealFile, bInterpol);  // prepare to read
                    iResult = setIce(pVRIce);
                    delete pVRIce;
                } else {
                    iResult = -1;
                    printf("File [%s] does not exist\n", sRealFile);
                }
            } else {
                printf("Invalid ICE line [%s]\n", pLine);
                iResult =-1;
            }
        }
    }
    if (iResult == 0) {
        if ((sLineMap[KEY_TEMP].length() > 0) && (sLineMap[KEY_RAIN].length() > 0)) {
            printf("sLineMap[KEY_TEMP]: [%s]\n", sLineMap[KEY_TEMP].c_str());
            printf("sLineMap[KEY_RAIN]: [%s]\n", sLineMap[KEY_RAIN].c_str());
            char sRealFile[512];
            ValReader *pVRTemp = NULL;
            ValReader *pVRRain = NULL;

            if (strstr(sLineMap[KEY_TEMP].c_str(), "TEMP FLAT")== sLineMap[KEY_TEMP].c_str()) {
                double fTemp = 0;
                if (sscanf(sLineMap[KEY_TEMP].c_str(), "TEMP FLAT %lf", &fTemp) == 1) {
                    if (m_pClimate == NULL) {
                        m_pClimate = new Climate(m_iNumCells, 0, m_pGeo);
                    }
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pClimate->m_adActualTemps[i]    = fTemp;
                        m_pClimate->m_adAnnualMeanTemp[i] = fTemp;
                    }
                } 
                double fRain = 0;
                if (sscanf(sLineMap[KEY_TEMP].c_str(), "RAIN FLAT %lf", &fRain) == 1) {
                    if (m_pClimate == NULL) {
                        m_pClimate = new Climate(m_iNumCells, 0, m_pGeo);
                    }
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pClimate->m_adActualRains[i]    = fRain;
                        m_pClimate->m_adAnnualRainfall[i] = fRain;
                    }
                } 
            } else {

                if (sscanf(sLineMap[KEY_TEMP].c_str(), "TEMP QMAP %s", sPar1) == 1) {
                    if (exists(sPar1, sRealFile)) {
                        pVRTemp  = QMapUtils::createValReader(sRealFile, bInterpol); 
                    } 
                } 
                if (sscanf(sLineMap[KEY_RAIN].c_str(), "RAIN QMAP %s", sPar1) == 1) {
                    if (exists(sPar1, sRealFile)) {
                        pVRRain  = QMapUtils::createValReader(sRealFile, bInterpol); 
                    }
                }                    
                if ((pVRTemp != NULL) && (pVRRain != NULL)) {
                    uchar iNumSeasons = 0;
                    m_pClimate = new Climate(m_iNumCells, iNumSeasons, m_pGeo);
                    m_pClimate->updateAnnuals(pVRTemp, pVRRain);
                    printf("TEMP/RAIN OK\n");  
                    iResult =0;
                } else {
                    iResult = -1;
                    if (pVRTemp == NULL) {
                        printf("Couldn't open TEMP QMAP [%s]\n", sLineMap[KEY_TEMP].c_str());
                    }
                    if (pVRRain != NULL) {
                        printf("Couldn't open RAIN QMAP [%s]\n", sLineMap[KEY_RAIN].c_str());
                    }
                }
                // delte ValReaders
                if (pVRTemp != NULL) {
                    delete pVRTemp;
                }
                if (pVRRain != NULL) {
                    delete pVRRain;
                }
            }
        } else if ((sLineMap[KEY_TEMP].length() == 0) && (sLineMap[KEY_RAIN].length() == 0)) {
            printf("OK: no climate\n");
        } else {
            printf("Must specify TEMP and RAIN, or nothing\n");
        }
    }

    if (iResult == 0) {
        if (sLineMap[KEY_ANPP].length() > 0) {
            printf("sLineMap[KEY_ANPP]: [%s]\n", sLineMap[KEY_ANPP].c_str());
            iResult = -1;
            if (strstr(sLineMap[KEY_ANPP].c_str(), "ANPP YES") == sLineMap[KEY_ANPP].c_str()) {
                if (m_pVeg == NULL) {
                    m_pVeg = new Vegetation(m_iNumCells, 0, m_pGeo, m_pClimate);
                }
            } else  if (strstr(sLineMap[KEY_ANPP].c_str(), "ANPP FLAT") == sLineMap[KEY_ANPP].c_str()) {
                if (m_pVeg == NULL) {
                    m_pVeg = new Vegetation(m_iNumCells, 0, m_pGeo, m_pClimate);
                }
                double fVal = 0;
                if (sscanf(sLineMap[KEY_ANPP].c_str(), "ANPP FLAT %lf", &fVal) == 1) {
                    printf("Filling Veg arrays with %f\n", fVal); 
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pVeg->m_adBaseANPP[i] = fVal;
                        m_pVeg->m_adTotalANPP[i] = fVal;
                    }
                }
            }
            iResult = 0;
        }
    }

    /*@@@ for old vegi stuff
    if (iResult == 0) {
        printf("sLineMap[KEY_DENS]: [%s]\n", sLineMap[KEY_DENS].c_str());
        printf("sLineMap[KEY_ANPP]: [%s]\n", sLineMap[KEY_ANPP].c_str());
        if ((sLineMap[KEY_DENS].length() > 0) && (sLineMap[KEY_ANPP].length() > 0)) {
            iResult = -1;
            if ((strstr(sLineMap[KEY_DENS].c_str(), "DENS QMAP") == 0) &&
                (strstr(sLineMap[KEY_ANPP].c_str(), "ANPP QMAP") == 0)) {
                char sRealFile[512];
                printf("Splitting DENS line\n");
                char sPars[MAX_PATH];
                strcpy(sPars, sLineMap[KEY_DENS].c_str()+strlen("DENS QMAP"));
                // split 
                std::vector<ValReader *> vVRDs;
                std::vector<double>      vValDs;
                char *pD = strtok(sPars, " \t,;:");
                iResult = 0;
                if (pD != NULL) {
                    if (strcmp(pD, "QMAP") == 0) {
                        pD = strtok(NULL, " \t,;:");
                        while ((iResult == 0) && (pD != NULL)) {
                            if (strcmp(pD, "NONE") == 0) {
                                vVRDs.push_back(NULL);
                                vValDs.push_back(0.0);
                            } else {
                                double dVal;
                                if (strToNum(pD, &dVal)) {
                                    vVRDs.push_back(NULL);
                                    vValDs.push_back(dVal);
                                } else {
                                    if (exists(pD, sRealFile)) {
                                        vVRDs.push_back(QMapUtils::createValReader(sRealFile, bInterpol)); 
                                        vValDs.push_back(-1.0);
                                        if (vVRDs.back() == NULL) {
                                            printf("Not a qmap: [%s]\n", pD);
                                            iResult = -1;
                                        }
                                    } else {
                                        printf("File [%s] does not exist\n", sRealFile);
                                        iResult = -1;
                                    }
                                }
                            }
                            pD = strtok(NULL, " \t,;:");
                        }
                    } else {
                        printf("Expected QMAP in ANPP Line\n");
                        iResult = -1;
                    }
                } else {
                    printf("Expected parameters in ANPP Line\n");
                    iResult = -1;
                }

                printf("Splitting ANPP line\n");
                strcpy(sPars, sLineMap[KEY_ANPP].c_str()+strlen("ANPP"));
                // split 
                std::vector<ValReader *> vVRNs;
                std::vector<double>      vValNs;
                char *pN = strtok(sPars, " \t,;:");
                iResult = 0;
                if (pN != NULL) {
                    if (strcmp(pN, "QMAP") == 0) {
                        pN = strtok(NULL, " \t,;:");
                        while ((iResult == 0) && (pN != NULL)) {
                            if (strcmp(pN, "NONE") == 0) {
                                vVRNs.push_back(NULL);
                                vValNs.push_back(0.0);
                            } else {
                                double dVal;
                                if (strToNum(pN, &dVal)) {
                                    vVRNs.push_back(NULL);
                                    vValNs.push_back(dVal);
                                } else {
                                    if (exists(pN, sRealFile)) {
                                        vVRNs.push_back(QMapUtils::createValReader(sRealFile, bInterpol)); 
                                        vValNs.push_back(-1.0);
                                        if (vVRNs.back() == NULL) {
                                            iResult = -1;
                                            printf("Not a qmap: [%s]\n", pN);
                                        }
                                    } else {
                                        printf("File [%s] does not exist\n", sRealFile);
                                        iResult = -1;
                                    }
                                }
                            }
                            pN = strtok(NULL, " \t,;:");
                        }
                    } else {
                        printf("Expected QMAP in ANPP Line\n");
                        iResult = -1;
                    }
                } else {
                    printf("Expected parameters in ANPP Line\n");
                    iResult = -1;
                }

                printf("Splitting: res=%d\n", iResult);
                
                if (iResult == 0) {
                    if (vVRDs.size() == vVRNs.size()) {
                        if (m_pVeg == NULL) {
                            m_pVeg = new Vegetation(m_iNumCells, (uint)vVRDs.size(), m_pGeo, m_pClimate);
                        }
                        
                        for (uint i = 0;  i < vVRDs.size(); i++) {
                            if (vVRDs[i] != NULL) {
                                setDensity(i, vVRDs[i]);
                            } else {
                                setDensity(i, vValDs[i]);
                            }
                            if (vVRNs[i] != NULL) {
                                setANPP(i, vVRNs[i]);
                            } else {
                                setANPP(i, vValNs[i]);
                            }
                        }
                        printf("Veg has been read\n");
                    } else {
                        printf("Number of params for DENS and ANPP differ (%zd != %zd)\n", vVRDs.size(), vVRNs.size());
                    }
                }
                for (uint i = 0;  i < vVRDs.size(); i++) {
                    if (vVRDs[i] != NULL) {
                        delete vVRDs[i];
                    }
                    if (vVRNs[i] != NULL) {
                        delete vVRNs[i];
                    }
                }
            } else {
                printf("Need DENS and ANPP,or nothing at all\n");
            }
        }

    }
    @@@@*/

    return iResult;
}



//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int GridFactory::createCells(IcoGridNodes *pIGN) { // THIS IS FOR ICOSAHEDRON GRID
 
    LOG_STATUS("[GridFactory::createCells] allocating %d cells\n", m_iNumCells);
    
    int iC = 0;
    m_pCG->m_aCells = new SCell[m_iNumCells];
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); ++it) {
        m_pCG->m_mIDIndexes[it->first]=iC;
        m_pCG->m_aCells[iC].m_iGlobalID    = it->first;
        m_pCG->m_aCells[iC].m_iNumNeighbors = (uchar)it->second->m_iNumLinks;
        //        pCF->setGeography(m_pGeography, iC, it->second);
        iC++;
    }

    LOG_STATUS("[GridFactory::createCells] linking cells\n");

    // linking and distances
    for (uint i =0; i < m_iNumCells; ++i) {
        // get link info from IcCell
        IcoNode *pIN = pIGN->m_mNodes[m_pCG->m_aCells[i].m_iGlobalID];
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = m_pCG->m_mIDIndexes[pIN->m_aiLinks[j]];
        }
        for (int j = pIN->m_iNumLinks; j < MAX_NEIGH; ++j) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    return 0;
}
 
//-----------------------------------------------------------------------------
// createCellsHexPeriodic
//   create cells
//   link cells
//
int GridFactory::createCellsHexPeriodic(uint iX, uint iY) {

    for(uint i=0; i<m_iNumCells; i++) {
        m_pCG->m_aCells[i].m_iNumNeighbors = 0;

        for (int j = 0; j < MAX_NEIGH; j++) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }

        //                  1 2           NW NE
        //                 0 X 3         W  X  E
        //                  5 4           SW SE
        
        //  . . . . 
        //  3rd row:     X X X X X X 
        //  2nd row:    X X X X X X 
        //  1st row:     X X X X X X
        //  0th row:    X X X X X X 

        // let's first deal with boundary rows and north-south neighbors:
        
        if (i < iX) { // first row
            
            if (i==0) {
                m_pCG->m_aCells[i].m_aNeighbors[1] = 2*iX - 1;       // NW                
                m_pCG->m_aCells[i].m_aNeighbors[2] = iX;             // NE               
                m_pCG->m_aCells[i].m_aNeighbors[4] = iX * (iY-1);    // SE             
                m_pCG->m_aCells[i].m_aNeighbors[5] = iX * iY - 1;    // SW
            } else {
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX - 1;     // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i + iX * (iY-1); // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i + iX - 1;     // SW
            } 
            
        } else if (i >= (iY-1)*iX) { // last row
            
            if (i== (iX*iY - 1)) {
                m_pCG->m_aCells[i].m_aNeighbors[1] = iX - 1;         // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = 0;              // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = iX*(iY-2);      // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = iX*(iY-1) - 1;  // SW
            } else { 
                m_pCG->m_aCells[i].m_aNeighbors[1] = i - iX*(iY-1);  // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i - iX*(iY-1) + 1; // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - 2*iX + 1;   // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            }
            
        } else if ( (i/iY)%2 == 0 ) { // rows 0, 2, 4, 6...
            if ( (i%iX) == 0 ) { // first column
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + 2*iX - 1;   // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX;         // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - 1;          // SW
            } else {
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX - 1;     // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX;         // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX - 1;     // SW
            }
            
        } else { // rows 1, 3, 5...
            if ( (i%iX) == iX-1 ) { // last column
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX;         // NW          
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + 1;          // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - 2*iX + 1;   // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            } else {
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX;         // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX + 1;     // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX + 1;     // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            }
        }
        
        
        // now deal with boundary columns and left-right neighbors:
        
        if ( (i%iX) == 0) { // first column
            m_pCG->m_aCells[i].m_aNeighbors[0] = i + iX - 1;      // W                 
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + 1;           // E
            
        } else if ( (i%iX) == iX-1 ) { // last column 
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;           // W                 
            m_pCG->m_aCells[i].m_aNeighbors[3] = i - iX + 1;      // E
            
        } else {
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;   // W                 
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + 1;   // E
        }               
    }
    return 0;    
}

//-----------------------------------------------------------------------------
// createCellsHexNonPeriodic
//   create cells
//   link cells
//
int GridFactory::createCellsHexNonPeriodic(uint iX, uint iY) {

    for(uint i=0; i<m_iNumCells; i++) {

        for (int j = 0; j < MAX_NEIGH; j++) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
  
        //                  1 2           NW NE
        //                 0 X 3         W  X  E
        //                  5 4           SW SE

        //  . . . . 
        //  3rd row:     X X X X X X 
        //  2nd row:    X X X X X X 
        //  1st row:     X X X X X X
        //  0th row:    X X X X X X 

        m_pCG->m_aCells[i].m_iNumNeighbors = 0;

        // let's first deal with boundary rows and north-south neighbors:

        if (i < iX) { // first row
            
            if (i==0) {
                m_pCG->m_aCells[i].m_iNumNeighbors += 1;
                m_pCG->m_aCells[i].m_aNeighbors[2] = iX;             // NE               
            } else {
                m_pCG->m_aCells[i].m_iNumNeighbors += 2;
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX - 1;     // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
            } 
            
        } else if (i >= (iY-1)*iX) { // last row
            
            if (i== (iX*iY - 1)) {
                m_pCG->m_aCells[i].m_iNumNeighbors += 1;
                m_pCG->m_aCells[i].m_aNeighbors[5] = iX*(iY-1) - 1;  // SW
            } else { 
                m_pCG->m_aCells[i].m_iNumNeighbors += 2;
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - 2*iX + 1;   // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            }
            
        } else if ( (i/iY)%2 == 0 ) { // rows 0, 2, 4, 6...
            if ( (i%iX) == 0 ) { // first column
                m_pCG->m_aCells[i].m_iNumNeighbors += 2;
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX;         // SE
            } else {
                m_pCG->m_aCells[i].m_iNumNeighbors += 4;
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX - 1;     // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX;         // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX;         // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX - 1;     // SW
            }
            
        } else { // rows 1, 3, 5...
            if ( (i%iX) == iX-1 ) { // last column
                m_pCG->m_aCells[i].m_iNumNeighbors += 2;
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX;         // NW          
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            } else {
                m_pCG->m_aCells[i].m_iNumNeighbors += 4;
                m_pCG->m_aCells[i].m_aNeighbors[1] = i + iX;         // NW
                m_pCG->m_aCells[i].m_aNeighbors[2] = i + iX + 1;     // NE
                m_pCG->m_aCells[i].m_aNeighbors[4] = i - iX + 1;     // SE
                m_pCG->m_aCells[i].m_aNeighbors[5] = i - iX;         // SW
            }
        }
        
        
        // now deal with boundary columns and left-right neighbors:
        
        if ( (i%iX) == 0) { // first column
            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + 1;           // E
            
        } else if ( (i%iX) == iX-1 ) { // last column 
            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;           // W                 
            
        } else {
            m_pCG->m_aCells[i].m_iNumNeighbors += 2;
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;   // W                 
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + 1;   // E
        }               
    }
    return 0;
}

//-----------------------------------------------------------------------------
// createCellsRectPeriodic
//   link cells
//
int GridFactory::createCellsRectPeriodic(uint iX, uint iY) {
    for(uint i=0; i<m_iNumCells; i++) {

        m_pCG->m_aCells[i].m_iNumNeighbors = (uchar)4;
        for (int j = 0; j < MAX_NEIGH; j++) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }

        //    1
        //  0 X 2
        //    3

        // let's first deal with boundary rows and up-down neighbors:

        if (i<iX) { // first row

            m_pCG->m_aCells[i].m_aNeighbors[1] = ((iY - 1) * iX) + i;  // up
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + iX;               // down

        } else if (i >= (iY-1) * iX) {  // last row
                    
            m_pCG->m_aCells[i].m_aNeighbors[1] = i - iX;               // up
            m_pCG->m_aCells[i].m_aNeighbors[3] = i - ((iY - 1) * iX);  // down
                    
        } else {

            m_pCG->m_aCells[i].m_aNeighbors[1] = i - iX;  // up
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + iX;  // down
        }


        // now we deail with boundary columns and left-right neighbors:

        if ( (i%iX) == 0) { // first column

            m_pCG->m_aCells[i].m_aNeighbors[0] = i + iX - 1;  // left
            m_pCG->m_aCells[i].m_aNeighbors[2] = i + 1;       // right

        } else if ( (i%iX) == (iX-1) ) { // last column

            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;       // left
            m_pCG->m_aCells[i].m_aNeighbors[2] = i - iX + 1;  // right

        } else {

            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;   // left
            m_pCG->m_aCells[i].m_aNeighbors[2] = i + 1;   // right
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// createCellsRectNonPeriodic
//   link cells
//
int GridFactory::createCellsRectNonPeriodic(uint iX, uint iY) {
    for(uint i=0; i<m_iNumCells; i++) {
                
        m_pCG->m_aCells[i].m_iNumNeighbors = 0;

        //    1
        //  0 X 2
        //    3

        // let's first deal with boundary rows and up-down neighbors:

        if (i < iX) { // first row: no up

            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[1] = -1;      //  no up
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + iX;  // down

        } else if(i >= (iY-1)*iX ) { // last row: no down

            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[1] = i - iX;  // up
            m_pCG->m_aCells[i].m_aNeighbors[3] = -1;      // no down

        } else {

            m_pCG->m_aCells[i].m_iNumNeighbors += 2;
            m_pCG->m_aCells[i].m_aNeighbors[1] = i - iX;  // up
            m_pCG->m_aCells[i].m_aNeighbors[3] = i + iX;  // down                    
        }


        // now we deail with boundary columns and left-right neighbors:

        if ( (i%iX) == 0) { // first column: no left
                    
            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[0] = -1;       // no left
            m_pCG->m_aCells[i].m_aNeighbors[2] = i + 1;    // right

        } else if ( (i%iX) == (iX-1) ) { // last column: no right

            m_pCG->m_aCells[i].m_iNumNeighbors += 1;
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;    // left
            m_pCG->m_aCells[i].m_aNeighbors[2] = -1;    // no right

        } else {

            m_pCG->m_aCells[i].m_iNumNeighbors += 2;
            m_pCG->m_aCells[i].m_aNeighbors[0] = i - 1;   // left
            m_pCG->m_aCells[i].m_aNeighbors[2] = i + 1;   // right
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int GridFactory::createCells(int iX, int iY, bool bPeriodic, bool bHex) {  // THIS IS FOR RECT OR HEX GRID

    int iResult = 0;
    m_pCG->m_aCells = new SCell[m_iNumCells];
    
    // use simplest ID scheme
    for(uint i=0; i<m_iNumCells; ++i) {
        m_pCG->m_aCells[i].m_iGlobalID = i;
        m_pCG->m_mIDIndexes[i] = i;
    }
    
    if (bPeriodic) {
        if (bHex) {
            iResult = createCellsHexPeriodic(iX, iY);
        } else {
            iResult = createCellsRectPeriodic(iX, iY);
        }
    } else {
        if (bHex) {
            iResult = createCellsHexNonPeriodic(iX, iY);
        } else {
            iResult = createCellsRectNonPeriodic(iX, iY);
        }
    }
    
    
    return iResult;
 
}
//-----------------------------------------------------------------------------
// initializeGeography
// for HEX and RECT grids
int GridFactory::initializeGeography(int iX, int iY, bool bHex) {
    int iResult = 0;

    float fDeltaY = bHex ? 0.8660254 : 1;
    float fOddRowOffset = bHex ? 0.5 : 0;

    for (uint i=0; i<m_iNumCells; i++) {
        unsigned int y = i / iX;
        unsigned int x = i % iX;
        m_pGeo->m_adLongitude[i] = (double)x + (y % 2) * fOddRowOffset;;
        m_pGeo->m_adLatitude[i] = (double)y * fDeltaY;
        // the neighbor arrays are arranged sequentially into a big 1-d array
        int i0 = m_pGeo->m_iMaxNeighbors*i;
        for (uint j=0; j< m_pGeo->m_iMaxNeighbors; j++) {
                m_pGeo->m_adDistances[i0+j] = 1;
        }

        m_pGeo->m_adAltitude[i] = 0;
        m_pGeo->m_adArea[i] = 1;
        m_pGeo->m_abIce[i] = false;
        m_pGeo->m_adWater[i] = 0;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initializeGeography
//  create Geography, set
//   Longitude
//   Latitude
//   Distances
//   Area
//
int GridFactory::initializeGeography(IcoGridNodes *pIGN) {

    int iResult = 0;

    bool bDeg2Rad = true;
    // rectangular grids with linear "projection" should not 
    // have their coordinates modified
    printf("Testing type of IGN surface:[%s]\n", m_pCG->m_smSurfaceData[SURF_TYPE].c_str());
    if (m_pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
        printf("  --> is lattice\n");
        iResult = -1;
        char sPT[256];
        strcpy(sPT, m_pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE].c_str());
        printf("PROJ type  --> [%s]\n", sPT);

        char *p = strtok(sPT, " ");
        if (p != NULL) {
            char *pEnd;
            int iPT = (int)strtol(p, &pEnd, 10);
            printf("First word [%s]\n", p);
            if (*pEnd == '\0') {
                iResult = 0;
                if (iPT == PR_LINEAR) {
                    printf("have LINEAR\n");
            
                    bDeg2Rad = false;
                }
            }
        }
    }
    
    if (iResult == 0) {
        for (uint i=0; i<m_iNumCells; ++i) {
            gridtype iIndex = m_pCG->m_aCells[i].m_iGlobalID;  // for each cell find its ID
            IcoNode* pIN = pIGN->m_mNodes[iIndex];           // get the corresponding iconode in pIGN

       
            if(pIN != NULL) {
                m_pGeo->m_adAltitude[iIndex] = 0;

                m_pGeo->m_adLatitude[iIndex]  =  pIN->m_dLat;
                m_pGeo->m_adLongitude[iIndex] =  pIN->m_dLon;
                if (bDeg2Rad) {
                    m_pGeo->m_adLatitude[iIndex]  *=  180/M_PI;
                    m_pGeo->m_adLongitude[iIndex] *=  180/M_PI;
                }
                // the neighbor arrays are arranged sequentially into a big 1-d array
                int i0 = m_pGeo->m_iMaxNeighbors*iIndex;
                for (int j = 0; j < pIN->m_iNumLinks; j++) {
                    m_pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
                }
                m_pGeo->m_adArea[iIndex] = pIN->m_dArea;
                m_pGeo->m_abIce[i] = false;
                m_pGeo->m_adWater[i] = 0;
                m_pGeo->m_abCoastal[i] = false;

            } else {
                fprintf(stderr,"[GridFactory::setGeography] node of index %d not found\n",iIndex);
                iResult = -1;
            }
        }
    } else {
        fprintf(stderr,"[GridFactory::setGeography] couldn't read projection details\n");
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// setAltitude
//
// for HEX and RECT grids
int GridFactory::setAltitude(ValReader *pVRAlt) {
    int iResult = 0;
    
    for (uint i=0; i<m_iNumCells; i++) {
        m_pGeo->m_adAltitude[i] = pVRAlt->getDValue(m_pGeo->m_adLongitude[i], m_pGeo->m_adLatitude[i]);
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// setIce
//
// for HEX and RECT grids
int GridFactory::setIce(ValReader *pVRIce) {
    int iResult = 0;
    
    for (uint i=0; i<m_iNumCells; i++) {
        m_pGeo->m_abIce[i] = (pVRIce->getDValue(m_pGeo->m_adLongitude[i], m_pGeo->m_adLatitude[i])>0);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// setDensity
//
 int GridFactory::setDensity(int iIndex, ValReader *pVRD) {
    int iResult = 0;
    
    // nothing to do
    
    return iResult;
}

//-----------------------------------------------------------------------------
// setANPP
//
 int GridFactory::setANPP(int iIndex, ValReader *pVRN) {
    int iResult = 0;
    
    // nothing to do
    
    return iResult;
}

//-----------------------------------------------------------------------------
// setDensity
//
 int GridFactory::setDensity(int iIndex, double dD) {
    int iResult = 0;

    // nothing to do
    
    return iResult;
}

//-----------------------------------------------------------------------------
// setANPP
//
 int GridFactory::setANPP(int iIndex, double dN) {
    int iResult = 0;

    // nothing to do
    

    return iResult;
}



