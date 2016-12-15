#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <set>
#include <string>
#include <vector>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "dbgprint.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "StatusWriter.h"
#include "FaceChecker.h"
#include "EQSplitter.h"
#include "EQGridCreator.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "EQTriangle.h"
#include "EQsahedron.h"
#include "EQNodeClassificator.h"
#include "EQSplitter.h"
#include "EQTiling.h"
#include "LonLatSplitter.h"
#include "LonLatTiling.h"
#include "EQTileLinks.h"


#define MODE_NONE -1
#define MODE_MERID 2
#define MODE_ICO   3
#define MODE_RECT  4

#define KEY_MERID "merid"
#define KEY_ICO   "ico"
#define KEY_RECT  "rect"
//----------------------------------------------------------------------------
// 
//



//----------------------------------------------------------------------------
// addGeography
//
int addGeography(int iTile, SCellGrid *pCG, IcoGridNodes *pIGN) {
    int iResult = 0;
    Geography *pGeo = new Geography( pCG->m_iNumCells, 6, 1, 0);
    for (uint i=0; i < pCG->m_iNumCells; ++i) {
        int iIndex = pCG->m_aCells[i].m_iGlobalID; 
        IcoNode* pIN = pIGN->m_mNodes[iIndex];           // get the corresponding iconode in pIGN
       
        if(pIN != NULL) {
            pGeo->m_adAltitude[i] = 3*iTile;
            
            pGeo->m_adLatitude[i]  =  pIN->m_dLat*180/M_PI;
            pGeo->m_adLongitude[i] =  pIN->m_dLon*180/M_PI;
            
            // the neighbor arrays are arranged sequentially into a big 1-d array
            int i0 = pGeo->m_iMaxNeighbors*i;
            for (int j = 0; j < pIN->m_iNumLinks; j++) {
                pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
            }
            pGeo->m_adArea[i] = pIN->m_dArea;
            pGeo->m_abIce[i] = (pIN->m_iZone == ZONE_HALO);
            pGeo->m_adWater[i] = (pIN->m_iZone == ZONE_HALO);
            
        } else {
            fprintf(stderr,"addGeography] node of index %d not found\n",iIndex);
            iResult = -1;
        }
    }
    if (iResult == 0) {
        pCG->setGeography(pGeo);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// splitGrid
//
int splitGrid(BasicTiling *pBT, const char *sQDFBody, const char *sIGNBody, int iVerbosity) {
    int iResult   = 0;
    int iTotalIGN = 0;

    stringmap smSurfaceHeaders;
    smSurfaceHeaders[SURF_TYPE] = SURF_EQSAHEDRON;
    char sd[8];
    sprintf(sd, "%d", pBT->getSubDivNodes());
    smSurfaceHeaders[SURF_IEQ_SUBDIVS] = sd;
    printf("--- split ----\n");


    for (int i = 0; i < pBT->getNumTiles(); i++) {
        IcoGridNodes *pIGN = pBT->getIGN(i);
        pIGN->setData(smSurfaceHeaders);
        iTotalIGN +=  pIGN->m_mNodes.size();

        // write a QDF
        if (iResult == 0)  {
            if (sQDFBody != NULL) {
                SCellGrid *pCG = SCellGrid::createInstance(pIGN);
                // add basic geography data (Lon, Lat, distances to neighbors)
                iResult = addGeography(i, pCG, pIGN);
                if (iResult == 0) {
                    
                    // write QDF
                    char sQDFOut[512];
                    sprintf(sQDFOut, "%s_%d.qdf", sQDFBody, i);
                    
                    std::vector<PopBase *> vDummy;
                    StatusWriter *pSW = StatusWriter::createInstance(pCG, vDummy);
                    if (pSW != NULL) {
                        iResult = pSW->write(sQDFOut, -1, WR_GRID |WR_GEO);
                        if (iResult != 0) {
                            printf("Couldn't save QDF file [%s]\n", sQDFOut);
                            iResult = -1;
                        }
                        
                        delete pSW;
                    } else {
                        printf("Couldn't create status writer\n");
                    }
                }
                delete pCG->m_pGeography;
                delete pCG;
            }
        }
            
        // write a IGN
        if (iResult == 0) {
            if (sIGNBody != NULL) {
                if (iVerbosity > LL_INFO) printf("IGN size %d: %zd\n", i, pIGN->m_mNodes.size());
                
                char sIGNOut[512];
                sprintf(sIGNOut, "%s_%d.ign", sIGNBody, i);
                // printf("Writing IGN file: [%s]\n", sIGNOut);
                iResult = pIGN->write(sIGNOut, 6, false, smSurfaceHeaders);  // false: no tiling info
                
                if (iResult != 0) {
                    printf("Couldn't save IGN file [%s]\n", sIGNOut);
                    iResult = -1;
                }
                
            }
        }
    }

    if (iResult == 0) {
        printf("+++ success +++\n");
        printf("Created %d tile%s with a total of %d Nodes\n", pBT->getNumTiles(), (pBT->getNumTiles() == 1)?"":"s", iTotalIGN);
    } else {
            printf("--- failure ---\n");
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// calcTileLinks
//
int calcTileLinks(BasicTiling *pBT, char *pOut) {
    int iResult = -1;
    EQTileLinks *pEQTL = EQTileLinks::createInstance(pBT->getIGNs(), pBT->getNumTiles());
    if (pEQTL != NULL) {
        iResult = 0;
        printf("----\n");
        printf("---- link info ----\n");
        printf("----\n");
        printf("Border links: for each neighboring tile a list of the border elements connecting to it\n");
        printf("Halo links:   for each neighboring tile a list of the halo elements it connects to\n");
        printf("number of tiles: %d\n", pEQTL->getNumTiles());
        for (int i = 0; i < pEQTL->getNumTiles(); i++) {
            printf("--- Tile %d ---\n", i);
            printf("  Border links:\n");
            const tTileCommUnits &tcuB = pEQTL->getBorderUnits(i);
            tTileCommUnits::const_iterator ittcuB;
            for (ittcuB = tcuB.begin(); ittcuB != tcuB.end(); ++ittcuB) {
                printf("    %3d:", ittcuB->first);
                intset_cit it;
                for (it = ittcuB->second.begin(); it != ittcuB->second.end(); ++it) {
                    printf(" %d", *it);
                }
                printf("\n");
            }
            printf("  Halo links:\n");
            const tTileCommUnits &tcuH = pEQTL->getHaloUnits(i);
            tTileCommUnits::const_iterator ittcuH;
            for (ittcuH = tcuH.begin(); ittcuH != tcuH.end(); ++ittcuH) {
                printf("    %3d:", ittcuH->first);
                intset_cit it;
                for (it = ittcuH->second.begin(); it != ittcuH->second.end(); ++it) {
                    printf(" %d", *it);
                }
                printf("\n");
            }
        }

        // test reading and writing
        pEQTL->write(pOut);


        delete pEQTL;
        
        EQTileLinks *pEQTL2 = EQTileLinks::createInstance(pOut);
        if (pEQTL2 != NULL) {
            pEQTL2->write("lalalank");
            // the two files should be equal
            delete pEQTL2;
        }
        
    } else {
        printf("COuldn't create EQTileLinks\n");
    }
    return iResult;
}

/*
//----------------------------------------------------------------------------
// split2
//
int split2(int iSubDivNodes, int iSubDivTiles, const char *sQDFBody, const char *sIGNBody, int iVerbosity) {
    int iResult = -1;
    int iTotalIGN = 0;


    stringmap smSurfaceHeaders;
    smSurfaceHeaders[SURF_TYPE] = SURF_EQSAHEDRON;
    char sd[8];
    sprintf(sd, "%d", iSubDivNodes);
    smSurfaceHeaders[SURF_IEQ_SUBDIVS] = sd;
    printf("--- split ----\n");

    EQTiling *pEQT = EQTiling::createInstance(iSubDivNodes, iSubDivTiles, iVerbosity);
    if (pEQT != NULL) {
        iResult = 0;


        EQTileAnalyze *pEQTA = new EQTileAnalyze();

        for (int i = 0; i < pEQT->getNumTiles(); i++) {
            IcoGridNodes *pIGN = pEQT->getIGN(i);
            pIGN->setData(smSurfaceHeaders);
            iTotalIGN +=  pIGN->m_mNodes.size();

            // write a QDF
            if (iResult == 0)  {
                if (sQDFBody != NULL) {
                    SCellGrid *pCG = SCellGrid::createInstance(pIGN);
                    // add basic geography data (Lon, Lat, distances to neighbors)
                    iResult = addGeography(pCG, pIGN);
                    if (iResult == 0) {
                        
                        // write QDF
                        char sQDFOut[512];
                        sprintf(sQDFOut, "%s_%d.qdf", sQDFBody, i);
                        
                        std::vector<PopBase *> vDummy;
                        StatusWriter *pSW = StatusWriter::createInstance(pCG, vDummy);
                        if (pSW != NULL) {
                            iResult = pSW->write(sQDFOut, -1, WR_GRID |WR_GEO);
                            if (iResult != 0) {
                                printf("Couldn't save QDF file [%s]\n", sQDFOut);
                                iResult = -1;
                            }
                            
                            delete pSW;
                        } else {
                            printf("Couldn't create status writer\n");
                        }
                    }
                    delete pCG->m_pGeography;
                    delete pCG;
                }
            }
            
            // write a IGN
            if (iResult == 0) {
                if (sIGNBody != NULL) {
                    if (iVerbosity > LL_INFO) printf("IGN size %d: %zd\n", i, pIGN->m_mNodes.size());
                   
                    char sIGNOut[512];
                    sprintf(sIGNOut, "%s_%d.ign", sIGNBody, i);
                    // printf("Writing IGN file: [%s]\n", sIGNOut);
                    iResult = pIGN->write(sIGNOut, 6, false, smSurfaceHeaders);  // false: no tiling info
                    
                    if (iResult != 0) {
                        printf("Couldn't save IGN file [%s]\n", sIGNOut);
                        iResult = -1;
                    } else {
                        iResult = pEQTA->addIGN(i, sIGNOut);
                    }
                    
                }
            }
   

        }


        if (iResult == 0) {
            printf("+++ success +++\n");
            printf("Created %d tile%s with a total of %d Nodes\n", pEQT->getNumTiles(), (pEQT->getNumTiles() == 1)?"":"s", iTotalIGN);
        } else {
            printf("--- failure ---\n");
        }
        
    } else {
        printf("Couldn't creaetEQTiling\n");
    }
    return iResult;
}
*/


//---------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - tiling for EQsahedron\n", pApp);
    printf("usage:\n");
    printf("  %s \"ico\" -n <subdivnodes> -t <subdivtiles> [-q <qdfbody>] [-i <ignbody>] [-v <verbosity>]\n", pApp);
    printf("  (creates a tiling of the grid based on a coarse subdivision of the EQsahedron)\n");
    printf("or\n");
    printf("  %s \"rect\" -n <subdivnodes> -t <nLon>\"x\"<nLat>\":\"<maxLat> [-q <qdfbody>] [-i <ignbody>] [-v <verbosity>]\n", pApp);
    printf("  (creates Lon-Lat rectangular tiles plus two caps at the poles)\n");
    printf("where\n");
    printf("  subdivnodes  subdivision of nodes\n");
    printf("  subdivtiles  subdivision of tiles (use 0 for an untiled grid)\n");
    printf("  nLon         number of tiles in longitudinal direction\n");
    printf("  nLat         number of tiles in latudinal direction\n");
    printf("  maxLat       maximum latitude (N and S) for tiles; caps start above this latitude\n");
    printf("  qdfbody      body for QDF output (no qdf output if omitted\n");
    printf("  ignbody      body for IGN output (no ign output if omitted\n");
    printf("  verbosity    verbosity level - 0 : NONE, 1 : TOPLEVEL, 2 : INFO, 3 : DETAIL (default: \"INFO\")\n");
    printf("\n");
    printf("NOTE: subdivnodes > 0,  subdivtiles >= 0\n");
    printf("NOTE: (subdivnodes + 1) must be divisible by (subdivtiles + 1)\n");
    printf("      i.e. tile grid nodes must be a subset of the grid nodes\n");
    printf("NOTE: (subdivnodes + 1)/(subdivtiles + 1) must be greater than 2\n");
    printf("      i.e. avoid \"faceless\" tiles\n");
}

//---------------------------------------------------------------------------
// doIcoSplit
//
int doIcoSplit(int iSubDivNodes, char *sSubDivTiles, char *sQDFBody, char *sIGNBody, int iVerbosity) {
    int iResult =-1;
    int iSubDivTiles = -1;
    EQsahedron *pEQNodes = NULL;

    if (strToNum(sSubDivTiles, &iSubDivTiles)) {

        if ((iSubDivNodes > 0) && (iSubDivTiles >= 0)) {
            if ((iSubDivNodes+1)%(iSubDivTiles+1) == 0) {

                if (((iSubDivNodes == 0) && (iSubDivTiles == 0)) ||
                    ((iSubDivNodes+1)/(iSubDivTiles+1) > 2)) {
                    
                    iResult = 0;
                } else {
                    printf("Incompatible subdivs: (<subdivnodes>+1)/(<subdivtiles>+1) must be greater than 2, or both must be 0\n");
                }
            } else {
                printf("Incompatible subdivs: <subdivnodes>+1 must be divisible by <subdivtiles>+1\n");
            }


            if (iResult == 0) {
                printf("Creating EQTilin instance\n");

                EQTiling *pEQT = EQTiling::createInstance(iSubDivNodes, iSubDivTiles, iVerbosity);
                if (pEQT != NULL) {

                    if (iResult == 0) {
                        printf("Doing calcTileLinks\n");
                        char sOut[128];
                        sprintf(sOut, "LinkInfo_ico_%d.%d", iSubDivNodes, iSubDivTiles);
                        iResult = calcTileLinks(pEQT, sOut);
                    }

                    if (iResult == 0) {
                        if ((sQDFBody != NULL) || (sIGNBody != NULL)) {
                            printf("Doing splitGrid\n");
                            iResult = splitGrid(pEQT, sQDFBody, sIGNBody, iVerbosity);
                        }
                    }
                } else {
                    printf("Couldn't create EQTiling\n");
                }
            }
       
            if (pEQNodes != NULL) {
                delete pEQNodes;
            }
        } else {
            printf("Incompatible subdivs: <subdivnodes> must be positive and <subdivtiles> must be negative\n");
        }


    } else {
        printf("subdivtiles given is no number: [%s]\n", sSubDivTiles);
    }    
    return iResult;
}
               

//---------------------------------------------------------------------------
// doRectSplit
//
int doRectSplit(int iSubDivNodes, char *sFormat, char *sQDFBody, char *sIGNBody, int iVerbosity) {
    int iResult = 0;
    int iNLon   = 0;
    int iNLat   = 0;
    double dMaxLat = 0;
    int iRead = sscanf(sFormat, "%dx%d:%lf", &iNLon, &iNLat, &dMaxLat);
    if (iRead == 3) {
        if ((dMaxLat > 0) && (iNLon > 2) && (iNLon > 1)) {
            LonLatTiling *pLLT = LonLatTiling::createInstance(iSubDivNodes, iNLon, iNLat, dMaxLat, iVerbosity);
            if (pLLT != NULL) {

                    if (iResult == 0) {
                        printf("Doing calcTileLinks\n");
                        char sOut[128];
                        sprintf(sOut, "LinkInfo_rect_%d.%dx%d", iSubDivNodes, iNLat,iNLat);
                        iResult = calcTileLinks(pLLT, sOut);
                    }

                    if (iResult == 0) {
                        if ((sQDFBody != NULL) || (sIGNBody != NULL)) {
                            printf("Doing splitGrid\n");
                            iResult = splitGrid(pLLT, sQDFBody, sIGNBody, iVerbosity);
                        }
                    }
                } else {
                    printf("Couldn't create EQTiling\n");
                }
            /*
            LonLatSplitter *pLLS = new LonLatSplitter(iNLon, iNLat, dMaxLat);
            m_pEQNodes = EQsahedron::createInstance(iSubDivNodes, true, NULL);
            if (m_pEQNodes != NULL) {
                
                delete m_pEQNodes;
            }
            delete pLLS;
            */
        } else {
            if (dMaxLat <= 0) {
                printf("<maxLat> must be positive\n");
            }
            if (iNLon <= 2) {
                printf("<nLon> must be greater than 2\n");
            }
            if (iNLat <= 1) {
                printf("<nLat> must be greater than 1\n");
            }
        }
    } else {
        printf("Invalid format string - should be [<nLon>\"x\"<nLat>\":\"<maxLat>]\n");
    }
    return iResult;

}
     
//---------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    int iSubDivNodes = -1;

    int iVerbosity   = LL_INFO+1;
    int iMode = MODE_NONE;
    //    char *sMode    = NULL;
    char *sQDFBody = NULL;
    char *sIGNBody = NULL;
    char *sSubDivTiles = NULL;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(5,
                               "-n:i!",    &iSubDivNodes,
                               "-t:S!",    &sSubDivTiles,
                               "-q:S",     &sQDFBody,
                               "-i:S",     &sIGNBody,
                               "-v:i",     &iVerbosity);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (iResult == PARAMREADER_ERR_FREE_PARAMS) {
                iResult = 0;
                std::vector<std::string> vFree;
                pPR->getFreeParams(vFree);
                std::string &sMode = vFree[0];
                if (strcmp(sMode.c_str(), apArgV[1]) == 0) {
                    if (strncasecmp(sMode.c_str(), KEY_ICO, strlen(KEY_ICO)) == 0) {
                        iMode = MODE_ICO;
                    } else if (strncasecmp(sMode.c_str(), KEY_RECT, strlen(KEY_RECT)) == 0) {
                        iMode = MODE_RECT;
                    } else if (strncasecmp(sMode.c_str(), KEY_MERID, strlen(KEY_MERID)) == 0) {
                        iMode = MODE_MERID;
                    } else {
                        printf("Mode descriptor must be one of required: \"ico\", \"rect\", \"merid\"\n ");
                        iMode = MODE_NONE;
                        iResult = -1;
                    }
                } else {
                    printf("%s\n", pPR->getErrorMessage(iResult).c_str());
                    iResult = -1;
                }
            } else {
                if (iResult == 0) {
                    printf("Mode descriptor required: \"ico\", \"rect\", \"merid\"\n ");
                    iResult = -1;
                } else {
                    printf("%s\n", pPR->getErrorMessage(iResult).c_str());
                }
            }
 
            if (iResult == 0) {
                iResult = -1;
                
                switch (iMode) {
                case MODE_ICO:
                    iResult = doIcoSplit(iSubDivNodes, sSubDivTiles, sQDFBody, sIGNBody, iVerbosity);
                    break;
                case MODE_RECT:
                    iResult = doRectSplit(iSubDivNodes, sSubDivTiles, sQDFBody, sIGNBody, iVerbosity);
                    break;
                case MODE_MERID:
                    //                    iResult = doMeridSplit(iSubDivNodes, sSubDivTiles, sQDFBody, sIGNBody, iVerbosity);
                    break;
                default:
                    iResult =-1;
                }
            }
        } else {
            printf("%s\n", pPR->getErrorMessage(iResult).c_str());
            printf("-----\n");
            usage(apArgV[0]);
        }

    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;


    return iResult;
}
