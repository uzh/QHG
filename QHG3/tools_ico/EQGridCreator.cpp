#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "dbgprint.h"
#include "VertexLinkage.h"
#include "EQsahedron.h"
#include "EQZones.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "BasicTile.h"
#include "BasicSplitter.h"

#include "EQGridCreator.h"




//-----------------------------------------------------------------------------
// constructor
//  (bSuperCell = true)
//
EQGridCreator::EQGridCreator(int iHalo, int iVerbosity) 
    : m_pEQ(NULL), 
      m_iHalo(iHalo), 
      m_iNumTiles(0),
      m_pVL(NULL),
      m_pEQZ(NULL),
      m_asNodeIDs(NULL),
      m_bSanityCheck(false),
      m_iVerbosity(iVerbosity) {

    }


//-----------------------------------------------------------------------------
// destructor
//
EQGridCreator::~EQGridCreator() {
    dbgprintf(m_iVerbosity, LL_TOP, "deleting EQGridCreator %p\n", this);
    if (m_pEQZ != NULL) {
        delete m_pEQZ;
    }

    if (m_asNodeIDs != NULL) {
        delete[] m_asNodeIDs;
    }
}

//-----------------------------------------------------------------------------
// createInstance
//
EQGridCreator *EQGridCreator::createInstance(EQsahedron *pEQ, int iHalo, BasicSplitter *pBS, bool bSanityCheck, int iVerbosity) {
    EQGridCreator *pEGC = new EQGridCreator(iHalo, iVerbosity);
    pEGC->m_bSanityCheck = bSanityCheck;
    pEGC->m_pEQ = pEQ;
    pEGC->m_pVL = pEQ->getLinkage();
    int iResult = pEGC->init(pBS);
    if (iResult != 0) {
        delete pEGC;
        pEGC = NULL;
    }
    return pEGC;
}


//-----------------------------------------------------------------------------
// init
//
int EQGridCreator::init(BasicSplitter *pBS) {
    int iResult = 0;
    
    if (m_pEQ != NULL) {
     
        BasicTile **asTiles = pBS->createTiles(&m_iNumTiles);
        if (iResult == 0) {
            iResult = createZones(asTiles);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// createZones
//  create the GridZones object to identify Edge Halo and internal nodes
//
int EQGridCreator::createZones(BasicTile **apTiles) {
    int iResult = -1;
    dbgprintf(m_iVerbosity, LL_INFO, "got vertexlinkage %p\n", m_pVL);
    
    m_asNodeIDs = new intcoll[m_iNumTiles];

    m_pEQZ = EQZones::create(m_pVL, 4, m_iVerbosity); 
    
    if (m_pEQZ != NULL) {
    
        dbgprintf(m_iVerbosity, LL_INFO, "creating %d node sets\n", m_iNumTiles);
        
        for (int i = 0; i < m_iNumTiles; i++) {
            m_pEQZ->findEdgeHaloForRegion(apTiles[i], m_iHalo, m_asNodeIDs[i]);
        }
        
        
        iResult = 0;        
        //sanity check
        //@@ takes a long time!!!      
        if (m_bSanityCheck) {
            bool bOK = zoneSanity();
            if (bOK) {
                iResult = 0;
            }
        }

     
    } else {
        printf("ERROR couldn't create GridZones\n");
    }
    return iResult;
}
//-----------------------------------------------------------------------------
// zoneSanity
//   check if every node is in exactly 1 core set or in exactly 1 edge set
//
bool EQGridCreator::zoneSanity() {
    bool bOK = true;
    dbgprintf(m_iVerbosity, LL_INFO, "ZoneSanity for %lld nodes ...\n", m_pVL->getNumVertices());
    for (int i = 0; i < m_pVL->getNumVertices(); ++i) {
        intset sC;
        intset sE;
        intset sH;
        bool bSubOK = false;
        intset::iterator it;
        //DBGPRINT("node %03d: ", i);
        // edges and vertices should appear exactly once
        for (int j = 0; j < m_iNumTiles; j++) {
            it = find( m_asNodeIDs[j][ZONE_CORE].begin(),  m_asNodeIDs[j][ZONE_CORE].end(), i);
            if (it != m_asNodeIDs[j][ZONE_CORE].end()) {
                sC.insert(j);
            } else {
                it = find( m_asNodeIDs[j][ZONE_EDGE].begin(),  m_asNodeIDs[j][ZONE_EDGE].end(), i);
                if (it != m_asNodeIDs[j][ZONE_EDGE].end()) {
                    sE.insert(j);
                } else {
                    it = find( m_asNodeIDs[j][ZONE_HALO].begin(),  m_asNodeIDs[j][ZONE_HALO].end(), i);
                    if (it != m_asNodeIDs[j][ZONE_HALO].end()) {
                        sH.insert(j);
                    }
                }
            }
        }
        if ((sC.size() == 1) != (sE.size() == 1)) {
            bSubOK = true;
            if (true) {
                if (sC.size() == 1) {
                    dbgprintf(m_iVerbosity, LL_DETAIL, "C%d ", *(sC.begin()));
                    // also check: edge region must not be any halo region
                    it = find(sH.begin(), sH.end(), *(sC.begin()));
                    bSubOK = (it == sH.end());
                } else {
                    dbgprintf(m_iVerbosity, LL_DETAIL, "E%d ", *(sE.begin()));
                    // also check: edge region must not be any halo region
                    it = find(sH.begin(), sH.end(), *(sE.begin()));
                    bSubOK = (it == sH.end());
                }
                dbgprintf(m_iVerbosity, LL_DETAIL, " H: ");
                for (it = sH.begin();it != sH.end(); it++) {
                    dbgprintf(m_iVerbosity, LL_DETAIL, "%d ", *it);
                }
                dbgprintf(m_iVerbosity, LL_DETAIL, "\n");
                
            }
        }
        if (!bSubOK) {
            printf("node %03d: ", i);
            printf("bad sets: C ");
            for (it = sC.begin();it != sC.end(); it++) {
                printf("%d ", *it);
            }
            printf("\n                    E ");
            for (it = sE.begin();it != sE.end(); it++) {
                printf("%d ", *it);
                }
            printf("\n                    H ");
            for (it = sH.begin();it != sH.end(); it++) {
                printf("%d ", *it);
            }
            printf("\n");
        }
        bOK = bOK && bSubOK;

    }
    dbgprintf(m_iVerbosity, LL_TOP, "ZoneSanity returns %s\n", bOK?"OK":"FAIL");
 
    return bOK;
}



//-----------------------------------------------------------------------------
// getGrid
//   create and return IcoGrid with given ID
//   It is the callers responibility to delete this IcoGrid!!
//
IcoGridNodes *EQGridCreator::getGrid(int iID) {

    dbgprintf(m_iVerbosity, LL_INFO, "-- Region %d --\n", iID);
    
    IcoGridNodes *pIGN = m_pEQZ->createGridForNodes(m_asNodeIDs[iID]);
    
    
    dbgprintf(m_iVerbosity, LL_DETAIL, "nodes for tile %d:", iID);
    std::map<gridtype, IcoNode*>::iterator it2;
    for (it2 = pIGN->m_mNodes.begin(); it2 != pIGN->m_mNodes.end(); it2++) {
        dbgprintf(m_iVerbosity, LL_DETAIL, "  %d", it2->second->m_lID);
    }
    dbgprintf(m_iVerbosity, LL_DETAIL, "\n");
        
    return pIGN;
}

