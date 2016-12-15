#include <math.h>
#include <strings.h>

#include <set>
#include <map>

#include "utils.h"
#include "geomutils.h"
#include "Vec3D.h"
#include "GridProjection.h"
#include "Projector.h"
#include "Region.h"
#include "icoutil.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "IcoSpiral.h"
#include "VertexLinkage.h"
#include "GridZones.h"


static bool s_bVerbose = false;

#define DBGPRINT if(s_bVerbose)printf

const char *s_asZoneNames[] = {"NONE", "CORE", "EDGE", "HALO"};

//-----------------------------------------------------------------------------
// create
//
GridZones *GridZones::create(VertexLinkage *pVL, PolarConv *pPC, GridProjection *pGP, bool bNodeOrder, int iUseMask) {
    printf("Creating gridsplitterGridZones with VL %p (%lld verts) and PolarConv %p\n", pVL, pVL->getNumVertices(), pPC);
    GridZones *pGZ = new GridZones((int)pVL->getNumVertices(), iUseMask);
    int iResult = pGZ->init(pVL, pPC, pGP);
    if (iResult == 0) {
        if (bNodeOrder) {
            iResult = pGZ->orderNodes();
        }
    }
    if (iResult != 0) {
        delete pGZ;
        pGZ = NULL;
    }
    printf("returning GridZone %p\n", pGZ);
    return pGZ;
}

//-----------------------------------------------------------------------------
// destructor
//
GridZones::~GridZones() {
    if (m_pNodes != NULL) {
        for (int i =0; i < m_iNumNodes; i++) {
            if (m_pNodes[i] != NULL) {
                delete m_pNodes[i];
            }
        }
        delete[] m_pNodes;
    }
}

    
//-----------------------------------------------------------------------------
// constructor
//
GridZones::GridZones(int iNumNodes,int iUseMask) 
    : m_iNumNodes(iNumNodes),
      m_pNodes(NULL),
      m_iUseMask(iUseMask) {

    m_pNodes = new IcoNode*[m_iNumNodes];
    bzero(m_pNodes, m_iNumNodes*sizeof(IcoNode *));
}

//-----------------------------------------------------------------------------
// init
//
int GridZones::init(VertexLinkage *pVL, PolarConv *pPC, GridProjection *pGP) {
    int iResult = 0;
    int iC = 0;
   
    int iDistMode = DIST_ICO;
    if (pGP != NULL) {
        if (pGP->getProjector()->getID() == PR_LINEAR) {
            iDistMode = DIST_FLAT_LINEAR;
        } else {
            iDistMode = DIST_FLAT_SPHERE;
        }
    }
    printf("init with %d\n", m_iNumNodes);
    std::map<gridtype, Vec3D *>::const_iterator iti;
    for (iti = pVL->m_mI2V.begin(); iti != pVL->m_mI2V.end(); iti++) {
        Vec3D *pV = iti->second;

        // calc theta and phi
        double dPhi = 0;
        double dTheta = 0;
        if (pV != NULL) {
            double dArea = 0; // get it from a vertexlinkage thingy
            pPC->conv2Polar(pV, &dTheta, &dPhi);
            IcoNode *pIN = new IcoNode(iti->first, dTheta, dPhi, dArea);
            intset vLinks = pVL->getLinksFor(iti->first);
            if (s_bVerbose) {
                if (vLinks.size() > 6) {
                    intset::iterator st;
                    printf("Vertex #%d (%f,%f) has %zd links\n", iti->first, pV->m_fX, pV->m_fY,vLinks.size());
                    for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                        printf("  %d", *st);
                    }
                    printf("\n");
                }
            }
                    
            intset::iterator st;
            for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                Vec3D *pN = pVL->getVertex(*st);
                double dDist = -1;
                switch (iDistMode) {
                case DIST_ICO:
                    dDist = spherdist(pV, pN, 1.0);
                    pIN->m_dArea = pVL->calcArea(pIN->m_lID);
                    break;
                case DIST_FLAT_SPHERE:{
                    double dLon;
                    double dLat;
                    pGP->gridToSphere(pN->m_fX, pN->m_fY, dLon, dLat);
                    dDist = spherdist(dTheta, dPhi, dLon, dLat, 1);
                }
                    break;
                case DIST_FLAT_LINEAR:
                    dDist = pV->dist(pN);
                    break;
                }

                pIN->addLink(*st, dDist);

            }
            m_pNodes[iC] = pIN;
            m_mID2Index[iti->first] = iC;
            iC++;
        } else {
            printf("NULL Vertex for ID %d\n", iti->first);
            iResult = -1;
        }
    }
    m_iNumNodes = iC;
    printf("[GridZones::init] processed %d nodes - res %d\n", iC, iResult);
    return iResult;
}

//-----------------------------------------------------------------------------
// orderNodes
//
int GridZones::orderNodes() {
    int iResult = 0;
    IcoSpiral *pIS =  IcoSpiral::createInstance(m_iNumNodes, m_pNodes);
    if (pIS != NULL) {
        delete[] m_pNodes;
        m_pNodes = pIS->getOrderedNodes();
        delete pIS;
    } else {
        printf("Couldn't order the nodes\n");
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// findEdgeHaloForRegion
//  - collects all nodes lying within the specified region
//  - adds halo cells and flag edge cells
//  - construts a grid
//
void GridZones::findEdgeHaloForRegion(Region *pR, int iHalo, intcoll &sNodeIDs) {
    intset sTempHalo1;
    intset sTempEdge1;
    
    if (s_bVerbose) {
        pR->display();
    }
    int iCountNULLs = 0;
    //    printf("findEdgeHaloForRegion for %d nodes\n", m_iNumNodes);
    for (int i = 0; i < m_iNumNodes; ++i) {
        //        DBGPRINT("finding region fo node #%03d (%f,%f) (in region %d: %s)\n", i, m_pNodes[i]->m_dLon, m_pNodes[i]->m_dLat, pR->m_iID, (pR->contains( m_pNodes[i]))?"yes":"no" );
        int iConnectivity = 0;
        IcoNode *pIN = m_pNodes[i];
        if (pIN != NULL) {


        // find number of neighbors contained in tile
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            if (pR->contains(m_pNodes[m_mID2Index[pIN->m_aiLinks[j]]])) {
                ++iConnectivity;
            }
        }

        //        DBGPRINT("  -> In-region connectivity: %d/%d\n", iConnectivity,  pIN->m_iNumLinks);
        if (iConnectivity > 0) {
            if (iConnectivity == pIN->m_iNumLinks) {
                // all neighbors are in the tile
                if (pR->contains(pIN)) {
                    pIN->m_iRegionID = pR->m_iID;
                    sNodeIDs[ZONE_CORE].insert(i);
                    DBGPRINT("  node #%03d ->internal region %d\n", i, pR->m_iID);
                } else {
                    DBGPRINT("  node #%03d ->special case: all links but not node in region %d\n", i, pR->m_iID);
                }
            } else if (pR->contains(pIN)) {
                pIN->m_iRegionID = pR->m_iID;
                if (iHalo > 0) {
                    DBGPRINT("  node #%03d ->edge region %d\n", i, pR->m_iID);
                    sTempEdge1.insert(i);
                } else {
                    DBGPRINT("  node #%03d ->internal region %d\n", i, pR->m_iID);
                    sNodeIDs[ZONE_CORE].insert(i);
                }
            } else {
                if (iHalo > 0) {
                    DBGPRINT("  node #%03d ->halo region %d\n", i, pR->m_iID);
                    sTempHalo1.insert(i);

                    // add this region's id to the list of destinations of the node i
                    pIN->m_sDests.insert(pR->m_iID);
                }
                
            }
        } else {
            DBGPRINT(" node #%03d -> no zone region %d\n", i, pR->m_iID);
        }
        } else {
            printf("node[%d]: NULL\n", i);
            iCountNULLs++;
        }
    }
    if (iCountNULLs > 0) {
        printf("Had %d NULLs\n", iCountNULLs);
    }
    if (iHalo > 1) {
        expandRange(iHalo, sNodeIDs[ZONE_HALO], sTempHalo1, sTempEdge1);
        expandRange(iHalo, sNodeIDs[ZONE_EDGE], sTempEdge1, sTempHalo1);
    } else {
        sNodeIDs[ZONE_EDGE].insert(sTempEdge1.begin(), sTempEdge1.end());
        sNodeIDs[ZONE_HALO].insert(sTempHalo1.begin(), sTempHalo1.end());
    }

}

 

//-----------------------------------------------------------------------------
// expandRange
//  
void GridZones::expandRange(int iHalo, intset &sTotal, intset &sStart, intset &sAvoid) {
    intset sBase = sStart;
    intset sZone = sAvoid;
    while (iHalo > 1) {
        intset sNew;
    
        intset::const_iterator it;
        for (it = sBase.begin(); it != sBase.end(); ++it) {
            IcoNode *pIN = m_pNodes[*it];
            for (int j = 0; j < pIN->m_iNumLinks; ++j) {
                int iLink = pIN->m_aiLinks[j];
                intset::const_iterator it2 = sBase.find(iLink);
                if (it2 == sBase.end()) {
                    it2 = sZone.find(iLink);
                    if (it2 == sZone.end()) {
                        sNew.insert(iLink);
                    }
                }
            }
        }
        sTotal.insert(sBase.begin(), sBase.end());
        
        sZone = sBase;
        sBase = sNew;
        --iHalo;
    }
    
    sTotal.insert(sBase.begin(), sBase.end());

}

//-----------------------------------------------------------------------------
// createGridForRegion
//  - collects all nodes lying within the specified region
//  - adds halo cells and flag edge cells
//  - construts a grid
//
IcoGridNodes *GridZones::createGridForNodes(intcoll &sNodeIDs) {

    IcoGridNodes *pIG = new IcoGridNodes();

  


    intcoll::const_iterator itm;
    intset::const_iterator  its;
    //    printf("[GridZones::createGridForNodes] sNodeIDs.size=%zd\n", sNodeIDs.size());fflush(stdout);
    for (itm = sNodeIDs.begin(); itm != sNodeIDs.end(); ++itm) {
        // we have to copy the nodes, not pass refereneces
        s_bVerbose = true;
        DBGPRINT("   ZONE %s:", s_asZoneNames[itm->first]);
        for (its = itm->second.begin(); its != itm->second.end(); ++its) {
            DBGPRINT("  %d", *its);
            if (itm->first == ZONE_HALO) {
                DBGPRINT("[%d]", m_pNodes[*its]->m_iRegionID);
            }
            IcoNode *pIN = new IcoNode(m_pNodes[*its]);
            pIN->m_iZone = itm->first;
            pIN->m_iRegionID =  m_pNodes[*its]->m_iRegionID;
            if (pIN->m_iZone < m_iUseMask) {
                pIG->m_mNodes[*its] = pIN;
                DBGPRINT("!");
            }
        }
        DBGPRINT("\n");
        s_bVerbose = false;
    }
    return pIG;
}


//-----------------------------------------------------------------------------
// flattenSets
//  creates and returns a buffer of Edge and Halo IDs suitable
//  for being sent via MPI
//  Format:
//   | NumEdgeIDs | NumHaloIDs | EdgeID_1 |...| EdgeID_N | HaloID_1 | HaloID_M |
//
//  (=> size of buffer = sum of first two elements plus 2)
//
gridtype *GridZones::flattenSets(intset &sEdge, intset &sHalo) {
    gridtype *pBuf = new gridtype[2+sEdge.size()+sHalo.size()];
    intset::const_iterator it;
    gridtype *pCur = pBuf;
    *pCur++ = (gridtype)sEdge.size();
    *pCur++ = (gridtype)sHalo.size();
    for (it = sEdge.begin(); it != sEdge.end(); ++it) {
        *pCur++ = *it;
    }
    for (it = sHalo.begin(); it != sHalo.end(); ++it) {
        *pCur++ = *it;
    }
    return pBuf;
}
