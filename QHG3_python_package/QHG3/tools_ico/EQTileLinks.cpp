#include <stdio.h>
#include <string.h>

#include <map>
#include <deque>

#include "types.h"
#include "utils.h"
#include "strutils.h"
#include "LineReader.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "EQTileLinks.h"

#define KEY_NUMTILES    "NUMTILES :"
#define KEY_BORDERLINKS "BORDERLINKS"
#define KEY_HALOLINKS   "HALOLINKS"

//----------------------------------------------------------------------------
// createInstance
//
EQTileLinks *EQTileLinks::createInstance(IcoGridNodes **apIGN, int iNumTiles) {
    EQTileLinks *pEQTL = new EQTileLinks(apIGN, iNumTiles);
    int iResult = pEQTL->init();
    if (iResult != 0) {
        delete pEQTL;
        pEQTL = NULL;
    }
    return pEQTL;
}
    
//----------------------------------------------------------------------------
// createInstance
//
EQTileLinks *EQTileLinks::createInstance(const char *pInput) {
    EQTileLinks *pEQTL = new EQTileLinks();
    int iResult = pEQTL->read(pInput);
    if (iResult != 0) {
        delete pEQTL;
        pEQTL = NULL;
    }
    return pEQTL;
}

    
//----------------------------------------------------------------------------
// constructor
//
EQTileLinks::EQTileLinks() 
    : m_apIGN(NULL),
      m_iNumTiles(0) {
    
}
    
    
//----------------------------------------------------------------------------
// constructor
//
EQTileLinks::EQTileLinks(IcoGridNodes **apIGN, int iNumTiles) 
    : m_apIGN(apIGN),
      m_iNumTiles(iNumTiles) {
    
}
    
//----------------------------------------------------------------------------
// init
//
int EQTileLinks::init() {
    int iResult = 0;
    
    for (int iTile = 0; iTile < m_iNumTiles; iTile++) {
        std::map<gridtype, IcoNode*>::const_iterator it;
        for (it = m_apIGN[iTile]->m_mNodes.begin(); it != m_apIGN[iTile]->m_mNodes.end(); ++it) {
            IcoNode *pIN = it->second;
            if (pIN->m_iZone == ZONE_HALO) {
                m_mmLinkUnits[0][pIN->m_iRegionID][iTile].insert(pIN->m_lID);
                m_mmLinkUnits[1][iTile][pIN->m_iRegionID].insert(pIN->m_lID);
            }
        }
    }

    return iResult;
}




//----------------------------------------------------------------------------
// write
//
int EQTileLinks::write(const char *pOut) {
    int iResult = -1;

    FILE *fOut = fopen(pOut, "wt");
    if (fOut != NULL) {

        iResult = 0;
        fprintf(fOut, "# link info\n");
        fprintf(fOut, "%s%d\n", KEY_NUMTILES, m_iNumTiles);

        for (int i = 0; i < m_iNumTiles; i++) {
            const tTileCommUnits &tcuB = m_mmLinkUnits[0][i];
            tTileCommUnits::const_iterator ittcuB;
            for (ittcuB = tcuB.begin(); ittcuB != tcuB.end(); ++ittcuB) {
                fprintf(fOut, "%s %d %d : ", KEY_BORDERLINKS, i, ittcuB->first);
                intset_cit it;
                for (it = ittcuB->second.begin(); it != ittcuB->second.end(); ++it) {
                fprintf(fOut, " %d", *it);
                }
                fprintf(fOut, "\n");
            }
            
            const tTileCommUnits &tcuH = m_mmLinkUnits[1][i];
            tTileCommUnits::const_iterator ittcuH;
            for (ittcuH = tcuH.begin(); ittcuH != tcuH.end(); ++ittcuH) {
                fprintf(fOut, "%s %d %d : ", KEY_HALOLINKS, i, ittcuH->first);
                intset_cit it;
                for (it = ittcuH->second.begin(); it != ittcuH->second.end(); ++it) {
                    fprintf(fOut, " %d", *it);
                }
                fprintf(fOut, "\n");
            }
        }
        fclose(fOut);
    } else {
        printf("Couldn't open [%s] for writing\n", pOut);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// read
//
int EQTileLinks::read(const char *pInput) {
    int iResult = -1;

    LineReader *pLR = LineReader_std::createInstance(pInput, "rt");
    if (pLR != NULL) {
        iResult = 0;
        char* pLine = pLR->getNextLine();

        // first line should be NUMTILES line
        char sPattern[128];
        sprintf(sPattern, "%s %%d", KEY_NUMTILES);
        int iRead = sscanf(pLine, sPattern, &m_iNumTiles);
        pLine = pLR->getNextLine();
        if (iRead == 1) {
            // the following lines are either BORDERLINK or HALOLINK lines
            while ((iResult == 0) && !pLR->isEoF()) {
                char *p0 = strtok(pLine, " :\t\n");
                if (p0 != NULL) {
                    int iIndex = -1;
                    if (strcmp(p0, KEY_BORDERLINKS) == 0) {
                        iIndex = 0;
                    } else if (strcmp(p0, KEY_HALOLINKS) == 0) {
                        iIndex = 1;
                    } else {
                        iResult = -1;
                    }
                    std::deque<int> vVals;
                    if (iResult == 0) {
                        // collect all numbers that follow the key
                        p0 = strtok(NULL, " :\t\n");
                        while ((iResult == 0) && (p0 != NULL)) {
                            int i;
                            if (strToNum(p0, &i)) {
                                vVals.push_back(i);
                                p0 = strtok(NULL, " :\t\n");
                            } else {
                                iResult = -1;
                            }
                        }

                        if ((iResult == 0) && (vVals.size() > 2)) {
                            // the first two numbers are the tiles
                            int iTile1 = vVals[0];
                            vVals.pop_front();
                            int iTile2 = vVals[0];
                            vVals.pop_front();
                            // the rest go to the intset
                            m_mmLinkUnits[iIndex][iTile1][iTile2].insert(vVals.begin(), vVals.end());
                            
                        } else {
                            printf("Not enough values in line\n");
                            iResult = -1;
                        }
                    }
                } else {
                    printf("Bad line [%s]\n", pLine);
                }

                pLine = pLR->getNextLine();
            }
        } else {
            printf("Expected \"NUMTILES\" line\n");
            iResult = -1;
        }
        
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pInput);
    }
    return iResult;
}
