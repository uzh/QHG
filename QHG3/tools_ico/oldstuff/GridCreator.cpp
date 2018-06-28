
#include <stddef.h>
#include <algorithm>

#include "utils.h"
#include "Vec3D.h"
#include "icoutil.h"
#include "GridZones.h"
#include "VertexLinkage.h"
#include "Region.h"
#include "RegionSplitter.h"
#include "GridCreator.h"

#include "IcoGridNodes.h"
#include "IcoNode.h"

static bool s_bVerbose = true;
#define DBGPRINT if(s_bVerbose)printf

//-----------------------------------------------------------------------------
// constructor
//
GridCreator::GridCreator(int iHalo,bool bSuperCells) 
    : m_iHalo(iHalo),
      m_iNumTiles(0),
      m_pVL(NULL),
      m_pGP(NULL),
      m_pGZ(NULL),
      m_asNodeIDs(NULL),
      m_bSuperCells(bSuperCells),
      m_iUseMask(4),
      m_bSanityCheck(false) {
}



//-----------------------------------------------------------------------------
// destructor
//
GridCreator::~GridCreator() {
   

    if (m_pGZ != NULL) {
        delete m_pGZ;
    }

    if (m_asNodeIDs != NULL) {
        delete[] m_asNodeIDs;
    }

}

//-----------------------------------------------------------------------------
// createZones
//  create the GridZones object to identify Edge Halo and internal nodes
//
void GridCreator::createZones(Region **apTiles, PolarConv *pPC, bool bNodeOrder) {
    //    VertexLinkage *pVL = m_pI->getLinkage(); // pVL will be deleted by Icosahedron
    printf("got vertexlinkage %p\n", m_pVL);
    
    m_asNodeIDs = new intcoll[m_iNumTiles];

    m_pGZ = GridZones::create(m_pVL, pPC, m_pGP, bNodeOrder, m_iUseMask); //false: no ico spirals
    
    if (m_pGZ != NULL) {
    
        printf("creating %d nod sets\n", m_iNumTiles);
        
        for (int i = 0; i < m_iNumTiles; i++) {
            //            printf("Using region %d:  %p\n", i, apTiles[i]);
            m_pGZ->findEdgeHaloForRegion(apTiles[i], m_iHalo, m_asNodeIDs[i]);
            //            printf("---------\n");
        }
        
        
        
        //sanity check
        //@@ takes a long time!!!      
        if (m_bSanityCheck) {
            zoneSanity();
        }

        /*
          if (s_bVerbose) {
          for (int i = 0; i < m_iNumTiles; i++) {
          m_pGZ->showSets(m_asNodeIDs[i]);   
          printf("---------\n");     
          }
          }
        */
        if (m_bSuperCells) {
            // find number of bytes needed for ids
            int iShift =2+(int)log2(m_pVL->getNumVertices());
            printf("[ GridCreator::createZones] Shift for %lld nodes: %d\n", m_pVL->getNumVertices(), iShift);
            superCellOrder(iShift);
        }
    } else {
        printf("ERROR couldn't create GridZones\n");
    }
}

//-----------------------------------------------------------------------------
// setChecks
//   
//
void GridCreator::setChecks(int iUseMask, bool bSanityCheck) {
    m_iUseMask = iUseMask;
    m_bSanityCheck = bSanityCheck;
}

//-----------------------------------------------------------------------------
// zoneSanity
//   check if every node is in exactly 1 core set or in exactly 1 edge set
//
bool GridCreator::zoneSanity() {
    bool bPrevVerb = s_bVerbose;
    s_bVerbose = true;
    bool bOK = true;
    printf("ZoneSanity for %lld nodes ...\n", m_pVL->getNumVertices());
    for (int i = 0; i < m_pVL->getNumVertices(); ++i) {
        intset sC;
        intset sE;
        intset sH;
        bool bSubOK = false;
        intset::iterator it;
        //DBGPRINT("node %03d: ", i);
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
            if (false && s_bVerbose) {
                if (sC.size() == 1) {
                    printf("C%d ", *(sC.begin()));
                    // also check: edge region must not be any halo region
                    it = find(sH.begin(), sH.end(), *(sC.begin()));
                    bSubOK = (it == sH.end());
                } else {
                    printf("E%d ", *(sE.begin()));
                    // also check: edge region must not be any halo region
                    it = find(sH.begin(), sH.end(), *(sE.begin()));
                    bSubOK = (it == sH.end());
                }
                printf(" H: ");
                for (it = sH.begin();it != sH.end(); it++) {
                    printf("%d ", *it);
                }
                printf("\n");
            }
        }
        if (!bSubOK && s_bVerbose) {
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
    printf("ZoneSanity returns %s\n", bOK?"OK":"FAIL");
    s_bVerbose = bPrevVerb;

    return bOK;
}



//-----------------------------------------------------------------------------
// getGrid
//   create and return IcoGrid with given ID
//   It is the callers responibility to delete this IcoGrid!!
//
IcoGridNodes *GridCreator::getGrid(int iID) {
    if (s_bVerbose) {
        printf("-- Region %d --\n", iID);
    }
    IcoGridNodes *pIGN = m_pGZ->createGridForNodes(m_asNodeIDs[iID]);
    
    if (m_bSuperCells) {
        if (s_bVerbose) {
            printf("  changing IDs\n");
        }
        std::map<gridtype, IcoNode*>::iterator it;
        for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); it++) {
            it->second->m_lTID = m_mID2RegID[it->first];
        }
    }

    if (s_bVerbose) {
        std::map<gridtype, IcoNode*>::iterator it2;
        for (it2 = pIGN->m_mNodes.begin(); it2 != pIGN->m_mNodes.end(); it2++) {
            printf("  %d", it2->second->m_lID);
        }
        printf("\n");
    }
    return pIGN;
}


//-----------------------------------------------------------------------------
// superCellOrder
//   modify IDs:
//     in each region: CORE and EDGE get ids from 0 to N
//                     HALO get an ID of the form RegionID<<32+localID
//
void GridCreator::superCellOrder(int iShift) {

    m_mID2RegID.clear();
    for (int i = 0; i < m_iNumTiles; i++) {
        gridtype iPrefix = (i+7)<<iShift;
        intset::const_iterator it;
        gridtype iC = 0;
        for (it = m_asNodeIDs[i][ZONE_CORE].begin(); it != m_asNodeIDs[i][ZONE_CORE].end(); it++) {
            m_mID2RegID[*it] = iPrefix + iC;
            iC++;
        }
        for (it = m_asNodeIDs[i][ZONE_EDGE].begin(); it != m_asNodeIDs[i][ZONE_EDGE].end(); it++) {
            m_mID2RegID[*it] = iPrefix + iC;
            iC++;
        }
        

    }
    if (true && s_bVerbose) {
        std::map<gridtype, gridtype>::const_iterator cit;
        for (cit=m_mID2RegID.begin(); cit != m_mID2RegID.end(); cit++) {
            printf("  %d -> %d (%08x)\n", cit->first, cit->second, cit->second);
        }
    }
}
