#include <stdio.h>
#include <string.h>
#include <map>

#include "types.h"
#include "strutils.h"

#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "SCell.h"
#include "Geography.h"
#include "Climate.h"
#include "MoveStats.h"
#include "Navigation.h"
#include "SCellGrid.h"
#include "MessLogger.h"


/* not used?
static const char *sGridTypeNames[] = {
    "RECT",
    "HEX",
    "ICO",
    "IEQ",
};

const char *SCellGrid::getGridSType(int iType) {
    const char *p =NULL;
    if ((iType > 0) && (iType <= GRID_TYPE_IEQ)) {
        p = sGridTypeNames[iType];
    }
    return p;
}

int SCellGrid::getGridType(const char *pType) {
    int iType = -1;
    for (int i =0; i < GRID_TYPE_IEQ;i++) {
        if (strcasecmp(pType, sGridTypeNames[i]) == 0) {
            iType = i;
        }
    }
    return iType;
}
*/

/*
//-----------------------------------------------------------------------------
// constructor
//
SCellGrid::SCellGrid(int iID, int iNumCells, int iType, int iW, int iH, bool bPeriodic) 
    : m_iID(iID),
      m_iNumCells(iNumCells),
      m_aCells(NULL),
      m_iType(iType),
      m_iW(iW),
      m_iH(iH),
      m_bPeriodic(bPeriodic),
      m_pGeography(NULL),
      m_pClimate(NULL),
      m_pVegetation(NULL),
      m_pMoveStats(NULL) {
    
    m_pMoveStats = new MoveStats(m_iNumCells);
    // nothing to do here

}
*/


//-----------------------------------------------------------------------------
// constructor
//
SCellGrid *SCellGrid::createInstance(IcoGridNodes *pIGN) {
    SCellGrid *pCG = NULL;

 
    pCG = new SCellGrid(0, (uint) pIGN->m_mNodes.size(), pIGN->getData());
    int iC = 0;
    pCG->m_aCells = new SCell[pCG->m_iNumCells];
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); ++it) {
        pCG->m_mIDIndexes[it->first]=iC;
        pCG->m_aCells[iC].m_iGlobalID    = it->first;
        pCG->m_aCells[iC].m_iNumNeighbors = (uchar)it->second->m_iNumLinks;
        iC++;
    }

    LOG_STATUS("[GridFactory::createCells] linking cells\n");

    // linking and distances
    for (uint i =0; i < pCG->m_iNumCells; ++i) {
        // get link info from IcCell
        IcoNode *pIN = pIGN->m_mNodes[pCG->m_aCells[i].m_iGlobalID];

        // in partial grids not all neighbors are present
        int iActualNeighbors = 0;
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            std::map<gridtype, int>::const_iterator it2 = pCG->m_mIDIndexes.find(pIN->m_aiLinks[j]);
            if (it2 != pCG->m_mIDIndexes.end()) {
                // neighbor here: increase actual neighbor count
                pCG->m_aCells[i].m_aNeighbors[iActualNeighbors++] = it2->second;
            } else {
                // neighnor not here: decrease neighbor count
                pCG->m_aCells[i].m_iNumNeighbors--;
            }
        }
        for (int j = iActualNeighbors; j < MAX_NEIGH; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    
    return pCG;
}

//-----------------------------------------------------------------------------
// constructor
//
SCellGrid::SCellGrid(int iID, uint iNumCells, const stringmap &smSurfaceData) 
    : m_iID(iID),
      m_iNumCells(iNumCells),
      m_aCells(NULL),
      m_smSurfaceData(smSurfaceData),
      m_iConnectivity(6),
      m_pGeography(NULL),
      m_pClimate(NULL),
      m_pVegetation(NULL),
      m_pMoveStats(NULL),
      m_pNavigation(NULL) {

    m_mIDIndexes[-1]=-1;
    stringmap::const_iterator it = m_smSurfaceData.find(SURF_TYPE);
    if (it  != m_smSurfaceData.end()) {
        if (it->second == SURF_LATTICE) {
            it = m_smSurfaceData.find(SURF_LTC_LINKS);
            if (it  != m_smSurfaceData.end()) {
                // let's hope its 4 or 6 ...
                m_iConnectivity = atoi(it->second.c_str());
                if (m_iConnectivity == 4) {
                    m_iType = GRID_TYPE_FLAT4;
                } else {
                    m_iType = GRID_TYPE_FLAT6;
                }
            }
        } else if (it->second == SURF_EQSAHEDRON) {
            m_iConnectivity = 6;
            m_iType = GRID_TYPE_IEQ;
        } else if (it->second == SURF_ICOSAHEDRON) {
            m_iConnectivity = 6;
            m_iType = GRID_TYPE_ICO;
        }
    }

    m_pMoveStats = new MoveStats(m_iNumCells);

}


    
//-----------------------------------------------------------------------------
// destructor
//
SCellGrid::~SCellGrid() {

    if (m_aCells != NULL) {
        delete[] m_aCells;
    }

    if (m_pMoveStats != NULL) {
        delete m_pMoveStats;
    }

}

//-----------------------------------------------------------------------------
// setMoveStats
//
void SCellGrid::setMoveStats(MoveStats* pMS) { 
    if (m_pMoveStats != NULL) {
        delete m_pMoveStats;
    }
    m_pMoveStats = pMS;
}




//-----------------------------------------------------------------------------
// setGeography
//
void SCellGrid::setGeography(Geography* pGeo) { 
    m_pGeography = pGeo;
    if (m_pClimate != NULL) {
        m_pClimate->m_pGeography = pGeo;
    }
    if (m_pVegetation != NULL) {
        //m_pVegetation->m_pGeography = pGeo;
    }
};

//-----------------------------------------------------------------------------
// setClimate
//
void SCellGrid::setClimate(Climate* pClim) { 
    m_pClimate = pClim;
    if (m_pVegetation != NULL) {
        //m_pVegetation->m_pClimate = pClim;
    }
};

//-----------------------------------------------------------------------------
// setVegetation
//
void SCellGrid::setVegetation(Vegetation* pVeg) {
    m_pVegetation = pVeg; 
};


//-----------------------------------------------------------------------------
// setNavigation
//
void SCellGrid::setNavigation(Navigation* pNav) {
    m_pNavigation = pNav; 
};


//-----------------------------------------------------------------------------
// display
//
void SCellGrid::display() {
    for (uint i =0; i < m_iNumCells; i++) {
        printf("%d[%d]:\n", i, m_aCells[i].m_iGlobalID);
        for (int j = 0; j <  m_aCells[i].m_iNumNeighbors; j++) {
            printf("  %d[%d]", m_aCells[i].m_aNeighbors[j], m_aCells[m_aCells[i].m_aNeighbors[j]].m_iGlobalID);
        }
                printf("\n");
    }
}


//-----------------------------------------------------------------------------
// isCartesian
//
bool SCellGrid::isCartesian() {
    return (m_smSurfaceData["SURF_TYPE"] == "LTC");
}
