#include <stdio.h>
#include <math.h>

#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include "types.h"
#include "EQTriangle.h"
#include "EQsahedron.h"
#include "EQNodeClassificator.h"

//----------------------------------------------------------------------------
// indexFor 
//
int indexFor(int r, int c) {
    return r*(r+1)/2+c;
}


//----------------------------------------------------------------------------
// rowColFor
//
void rowColFor(int iIndex, int *pr, int *pc) {
    *pr = floor((sqrt(8*iIndex+1)-1)/2);
    *pc = iIndex - *pr*(*pr+1)/2;
}


//----------------------------------------------------------------------------
// showIntSet
//
void showIntSet(const char *pCaption, intset s) {
    intset_cit it;
    printf("%s", pCaption);
    for (it = s.begin(); it != s.end(); ++it) {
        printf(" %d", *it);
    }
    printf("\n");
}


//----------------------------------------------------------------------------
// constructor
//
EQNodeClassificator::EQNodeClassificator()
    : m_iSubDivNodes(-1),
      m_iSubDivTiles(-1),
      m_iNumTiles(-1),
      m_iRatio(-1),
      m_asBasicElements(NULL),
      m_asAllElements(NULL),
      m_asFinal(NULL),
      m_pEQCur(NULL) {

}


//----------------------------------------------------------------------------
// createInstance
//
EQNodeClassificator *EQNodeClassificator::createInstance(int iSubDivNodes, int iSubDivTiles) {
    EQNodeClassificator *pENC = new EQNodeClassificator();
    int iResult = pENC->init(iSubDivNodes, iSubDivTiles);
    if (iResult != 0) {
        delete pENC;
        pENC = NULL;
    }
    return pENC;
}


//----------------------------------------------------------------------------
// destructor
//
EQNodeClassificator::~EQNodeClassificator() {
    if (m_asBasicElements != NULL) {
        for (int i = 0; i < NUM_TYPES; i++) {
            if (m_asBasicElements[i] != NULL) {
                delete[] m_asBasicElements[i];
            }
        }
        delete[] m_asBasicElements;
    }

    if (m_asAllElements != NULL) {
        for (int i = 0; i < NUM_TYPES; i++) {
            if (m_asAllElements[i] != NULL) {
                delete[] m_asAllElements[i];
            }
        }
        delete[] m_asAllElements;
    }

    if (m_asFinal != NULL) {
        delete[] m_asFinal;
    }
}


//----------------------------------------------------------------------------
// init
//
int EQNodeClassificator::init(int iSubDivNodes, int iSubDivTiles) {
    int iResult = -1;
    if ((iSubDivNodes+1)%(iSubDivTiles+1) == 0) {
        m_iSubDivNodes = iSubDivNodes;
        m_iSubDivTiles = iSubDivTiles;
        m_iRatio = (m_iSubDivNodes+1)/(m_iSubDivTiles+1);

        m_iNumTiles = (m_iSubDivTiles+1)*(m_iSubDivTiles+1);
        m_asBasicElements = new intset*[NUM_TYPES];
        for (int i = 0; i < NUM_TYPES; i++) {
            m_asBasicElements[i] = new intset[m_iNumTiles];
        }

        m_asAllElements = new intset*[NUM_TYPES];
        for (int i = 0; i < NUM_TYPES; i++) {
            m_asAllElements[i] = new intset[ICOFACES*m_iNumTiles];
        }       

    
        classifyNodes();
 
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// classifySingleTile
//   in the tile iT classify all nodes relative to the tile as
//   vert (corner point of the tile), edge or face (neither edge nor vert)
//
void EQNodeClassificator::classifySingleTile(int rT, int cT, bool bUpper, int f) {
    //                        intset &sFaces, intset &sEdges, intset &sVerts) {
    int rN0 = m_iRatio*rT;
    int cN0 = m_iRatio*cT;

    
    //    printf("rN0 %d, cN0 %d, iRatio %d\n", rN0, cN0, m_iRatio);
    if (bUpper) {
        // upper
        for (int rN = 0; rN <= m_iRatio; rN++) {
            for (int cN = 0; cN <= rN; cN++) {
                int iIndex = indexFor(rN+rN0, cN+cN0);
                bool bHorizontalEdge = (rN == m_iRatio);
                bool bVerticalEdge   = (cN == 0);
                bool bDiagonalEdge   = (rN == cN);

                if ((bHorizontalEdge && bVerticalEdge) ||
                    (bHorizontalEdge && bDiagonalEdge) ||
                    (bDiagonalEdge && bVerticalEdge)) {
                    m_asBasicElements[TYPE_VERT][f].insert(iIndex);
                } else if (bHorizontalEdge || bVerticalEdge || bDiagonalEdge) {
                    m_asBasicElements[TYPE_EDGE][f].insert(iIndex);
                } else {
                    m_asBasicElements[TYPE_FACE][f].insert(iIndex);
                }
            }
        }
    } else {
        // lower
        for (int rN = 0; rN <= m_iRatio; rN++) {
            for (int cN = rN; cN <= m_iRatio; cN++) {
                int iIndex = indexFor(rN+rN0, cN+cN0);
                bool bHorizontalEdge = (rN == 0);
                bool bVerticalEdge   = (cN == m_iRatio);
                bool bDiagonalEdge   = (rN == cN);
  
                if ((bHorizontalEdge && bVerticalEdge) ||
                    (bHorizontalEdge && bDiagonalEdge) ||
                    (bDiagonalEdge && bVerticalEdge)) {
                    m_asBasicElements[TYPE_VERT][f].insert(iIndex);
                } else if (bHorizontalEdge || bVerticalEdge || bDiagonalEdge) {
                    m_asBasicElements[TYPE_EDGE][f].insert(iIndex);
                } else {
                    m_asBasicElements[TYPE_FACE][f].insert(iIndex);
                }
     
            }
        }
    }
}


//----------------------------------------------------------------------------
// classifyNodes
//   classify nodes in all tiles as vert, edge or face
//
void EQNodeClassificator::classifyNodes() {
    int f = 0;
    for (int iIndex = 0; iIndex < m_iNumTiles; iIndex++) {
        int rT = floor((sqrt(8*iIndex+1)-1)/2);
        int cT = iIndex - rT*(rT+1)/2;
        printf("I %d/%d : r %d c %d\n", iIndex, m_iNumTiles, rT, cT);
        if (rT <= m_iSubDivTiles) {
            // do upper triangle

            classifySingleTile(rT, cT, true, f);
            f++;

            // do lower triangle if exists 
            if (cT < rT) { 
                classifySingleTile(rT, cT, false, f);
                f++;
            }
        }
    }
}


//----------------------------------------------------------------------------
// getFaceNodeTile
//   get IDs of all tiles having the specified face (should be 1 id)
//
int  EQNodeClassificator::getFaceNodeTiles(int iFace, intset &sTiles) {

    for (int i = 0; i < m_iNumTiles; i++) {
        intset_cit it =  m_asBasicElements[TYPE_FACE][i].find(iFace);
        if (it !=  m_asBasicElements[TYPE_FACE][i].end()) {
            sTiles.insert(i);
        }
    }

    return sTiles.size();
}


//----------------------------------------------------------------------------
// getEdgeNodeTile
//   get IDs of all tiles having the specified edge (should be 2 ids)
//
int  EQNodeClassificator::getEdgeNodeTiles(int iEdge, intset &sTiles) {

    for (int i = 0; i < m_iNumTiles; i++) {
        intset_cit it = m_asBasicElements[TYPE_EDGE][i].find(iEdge);
        if (it != m_asBasicElements[TYPE_EDGE][i].end()) {
            sTiles.insert(i);
        }
    }

    return sTiles.size();
}


//----------------------------------------------------------------------------
// getVertNodeTile
//   get IDs of all tiles having the specified edge (should be 5 ids)
//
int  EQNodeClassificator::getVertNodeTiles(int iVert, intset &sTiles) {

    for (int i = 0; i < m_iNumTiles; i++) {
        intset_cit it = m_asBasicElements[TYPE_VERT][i].find(iVert);
        if (it != m_asBasicElements[TYPE_VERT][i].end()) {
            sTiles.insert(i);
        }
    }

    return sTiles.size();
}


//----------------------------------------------------------------------------
// assignEdges
//   - for each edge, find the two incident tiles
//   - assign each edge to one of its incident tiles
//
void EQNodeClassificator::assignEdges() {
    edgegroups eg;

    // group edges according to incidence
    for (int iTile0 = 0; iTile0 < ICOFACES*m_iNumTiles-1; ++iTile0) {
        for (int iTile1 = iTile0+1; iTile1 < ICOFACES*m_iNumTiles; ++iTile1) {
            intset &s0 = m_asAllElements[TYPE_EDGE][iTile0];
            intset &s1 = m_asAllElements[TYPE_EDGE][iTile1];
            int iS0 =  s0.size();
            int iS1 =  s1.size();
            std::vector<int> v((iS1>iS0)?iS1:iS0);
            std::vector<int>::iterator it = std::set_intersection (s0.begin(), s0.end(),
                                                                   s1.begin(), s1.end(),
                                                                   v.begin());
            v.resize(it-v.begin());
            for (uint i = 0; i < v.size(); ++i) {
                eg[std::pair<int,int>(iTile0, iTile1)].insert(v[i]);
            }
        }
    }
    printf("Grouped to %zd entries\n", eg.size());

    // loop through grouped edges and assign each set of edge vertexes to the smaller m:asFinal
    edgegroups::const_iterator it;
    for (it = eg.begin(); it != eg.end(); ++it) {
        int iS1 = m_asFinal[it->first.first].size();
        int iS2 = m_asFinal[it->first.second].size();
        int iT = (iS1 < iS2)?it->first.first:it->first.second;
        m_asFinal[iT].insert(it->second.begin(), it->second.end()); 
    }
}


//----------------------------------------------------------------------------
// assignVerts
//   assign each vert to the first tile in which it has more than neighbor.
//   for vert and the nodes to compare we find row and column in the basic
//   triangle where neighborhood is easily determined.
//
void EQNodeClassificator::assignVerts() {
    pointtiles pt;
    // collect points and their tiles  (plus the ico top face)
    for (int iTile0 = 0; iTile0 < ICOFACES*m_iNumTiles; ++iTile0) {
        int f = iTile0/m_iNumTiles;
        intset_cit it;
        for (it = m_asAllElements[TYPE_VERT][iTile0].begin(); it != m_asAllElements[TYPE_VERT][iTile0].end(); ++it) {
            pt[*it].insert(intpair(f, iTile0));
        }
    }
        
        
    pointtiles::const_iterator it;
    for (it = pt.begin(); it != pt.end(); ++it) {
        bool bSearching = true;
        int iNode = it->first;
        std::set<intpair>::const_iterator itTile;
        for (itTile = it->second.begin(); bSearching && (itTile != it->second.end()); ++itTile) {
            intset sneigh;
            intset &s = m_asFinal[itTile->second];
            int iFace = itTile->first;
            int iTile = itTile->second;
                
            int iLocNode0 = m_pEQCur->convertEQToTriangle(iNode, &iFace);
            int r0;
            int c0;
            rowColFor(iLocNode0, &r0, &c0);
            intset_cit itNode;
            for (itNode = s.begin(); itNode != s.end(); ++itNode) {
                int iLocNode1 = m_pEQCur->convertEQToTriangle(*itNode, &iFace);
                int r1;
                int c1;
                rowColFor(iLocNode1, &r1, &c1);
                //                printf("Neigh tile %d,face %d, nod %d: loc %d -> r %d c %d\n", itTile->second, iFace, *itNode, iLocNode1,r1, c1);
                if (((r0==r1) && (abs(c1-c0)==1)) ||
                    ((c0==c1) && (abs(r1-r0)==1)) ||
                    ((r1==r0+1) && (c1==c0+1)) ||
                    ((r1==r0-1) && (c1==c0-1))) {
                    sneigh.insert(*itNode);
                }
            }
            if (sneigh.size() > 1) {
                m_asFinal[iTile].insert(iNode);
                bSearching = false;
            }
        }
        if (bSearching) {
            printf("Couldn't assign %d\n", iNode);
        }
    }
}


//----------------------------------------------------------------------------
// distributeBorders
//  assign faces, edges and verts to the appropriate final set
//
void EQNodeClassificator::distributeBorders() {
    if (m_iSubDivTiles > 0) {
        distributeElements();
    } else {
        makeFullEQ();
    }
}

//----------------------------------------------------------------------------
// distributeElements
//
void EQNodeClassificator::distributeElements() {

    m_asFinal = new intset[ICOFACES*m_iNumTiles];


    // the face nodes belong to the tile containing them
    for (int iTile0 = 0; iTile0 < ICOFACES*m_iNumTiles; ++iTile0) {
        m_asFinal[iTile0].insert(m_asAllElements[TYPE_FACE][iTile0].begin(), m_asAllElements[TYPE_FACE][iTile0].end());
    }
    
    assignEdges();

    assignVerts();

    sanityCheck();
}

//----------------------------------------------------------------------------
// makeFullEQ
//  no tiling: everything goes into one tile
//
void EQNodeClassificator::makeFullEQ() {
    m_asFinal = new intset[1];

    // the face nodes belong to the tile containing them
    for (int iType = 0; iType < NUM_TYPES; ++iType) {
        for (int iTile0 = 0; iTile0 < ICOFACES*m_iNumTiles; ++iTile0) {
            m_asFinal[0].insert(m_asAllElements[iType][iTile0].begin(), m_asAllElements[iType][iTile0].end());
        }
    }
}

//----------------------------------------------------------------------------
// sanityCheck
//  make sure no node is left behind 
//  i.e. all have been classified and assigned
//
void EQNodeClassificator::sanityCheck() {
    int N = 12 + 30*m_iSubDivNodes+10*m_iSubDivNodes*(m_iSubDivNodes-1);
    intset sAll;
    for (int iTile0 = 0; iTile0 < ICOFACES*m_iNumTiles; ++iTile0) {
         sAll.insert(m_asFinal[iTile0].begin(), m_asFinal[iTile0].end());
    }
    printf("Calc: %d nodes, in final: %zd nodes\n", N, sAll.size());
    
}


//----------------------------------------------------------------------------
// applyToEQ
//   create face, edge and vertec list for every tile on every top face 
//   of the EQsahedron
//
int EQNodeClassificator::applyToEQ(EQsahedron *pEQNodes) {
    int iResult = -1;
    
    if (pEQNodes->getSubDivs() == m_iSubDivNodes) {
        iResult = 0;
        m_pEQCur = pEQNodes;
        // clean up potential leftovers
        for (int i = 0; i < ICOFACES*m_iNumTiles; i++) {
            for (int j = 0; j < NUM_TYPES; j++) {
                m_asAllElements[j][i].clear();
            }
        }
    
        for (int f = 0; (iResult == 0) && (f < ICOFACES); ++f) {
            //printf("---F %02d ---\n", f);
            EQTriangle *pEQT = m_pEQCur->getFaceTriangle(f);
            int iNumNodes = pEQT->getNumNodes();
            node *pNodes = pEQT->getNodes();
            
            for (int iNode = 0; (iResult == 0) && (iNode < iNumNodes); iNode++) {
                intset sFaceCommon;
                int sfnt = getFaceNodeTiles(iNode, sFaceCommon);
                if (sfnt > 0) {
                    for (intset_cit it = sFaceCommon.begin(); it != sFaceCommon.end(); ++it) {
                        m_asAllElements[TYPE_FACE][f*m_iNumTiles+*it].insert(pNodes[iNode].lID);
                    }
                } else {
                    intset sEdgeCommon;
                    int sent = getEdgeNodeTiles(iNode, sEdgeCommon);
                    if (sent > 0) {
                        for (intset_cit it = sEdgeCommon.begin(); it != sEdgeCommon.end(); ++it) {
                            m_asAllElements[TYPE_EDGE][f*m_iNumTiles+*it].insert(pNodes[iNode].lID);
                        }
                    } else {
                        intset sVertCommon;
                        int svnt = getVertNodeTiles(iNode, sVertCommon);
                        if (svnt > 0) {
                            for (intset_cit it = sVertCommon.begin(); it != sVertCommon.end(); ++it) {
                                m_asAllElements[TYPE_VERT][f*m_iNumTiles+*it].insert(/*f*iNumNodes+*/pNodes[iNode].lID);
                            }
                        } else {
                            // bad?
                            printf("Unassigned node [%d]\n", iNode);
                        }
                    }
                }
            }
        }
    } else {
        printf("EQsahedron's subdivisions should be [%d]\n", m_iSubDivNodes);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// showBasicElements
//
void EQNodeClassificator::showBasicElements() {
    for (int i  = 0; i < m_iNumTiles; i++) {
        printf("Tile %d\n", i);
        intset_cit it;
        showIntSet("  Faces: ", m_asBasicElements[TYPE_FACE][i]);
        showIntSet("  Edges: ", m_asBasicElements[TYPE_EDGE][i]);
        showIntSet("  Verts: ", m_asBasicElements[TYPE_VERT][i]);
    }
}


//----------------------------------------------------------------------------
// showAllClasses
//
void EQNodeClassificator::showAllClasses() {
    for (int f = 0; f < ICOFACES; ++f) {
        for (int i = 0; i < m_iNumTiles; i++) {
            printf("Face %02d, Tile %02d\n", f, f*m_iNumTiles+i);
            showIntSet("  Faces: ", m_asAllElements[TYPE_FACE][f*m_iNumTiles+i]);
            showIntSet("  Edges: ", m_asAllElements[TYPE_EDGE][f*m_iNumTiles+i]);
            showIntSet("  Verts: ", m_asAllElements[TYPE_VERT][f*m_iNumTiles+i]);
        }
    }
}


//----------------------------------------------------------------------------
// showFinal
//
void EQNodeClassificator::showFinal() {
    if (m_iSubDivTiles > 0) {
        for (int f = 0; f < ICOFACES; ++f) {
            for (int i = 0; i < m_iNumTiles; i++) {
                printf("Face %02d, Tile %02d\n", f, f*m_iNumTiles+i);
                showIntSet("  Nodes: ", m_asFinal[f*m_iNumTiles+i]);
            }
        }
    } else {
        printf("SingleTile\n");
        showIntSet("  Nodes: ", m_asFinal[0]);
    }
}
