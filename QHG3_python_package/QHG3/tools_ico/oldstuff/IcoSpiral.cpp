#include <stdio.h>
#include <math.h>

#include "utils.h"

#include "VertexLinkage.h"
#include "IcoNode.h"
#include "GridZones.h"
#include "IcoSpiral.h"


//-----------------------------------------------------------------------------
// constructor
//
IcoSpiral::IcoSpiral(int iNumNodes, IcoNode **m_apNodes)
    : m_iNumNodes(iNumNodes),
      m_apNodesIn(m_apNodes),
      m_apNodesOut(NULL) {

}

//-----------------------------------------------------------------------------
// destructor
//
IcoSpiral::~IcoSpiral() {
}


//-----------------------------------------------------------------------------
// createInstance
//
IcoSpiral *IcoSpiral::createInstance(int iNumNodes, IcoNode **m_apNodes) {
    IcoSpiral *pIS = new IcoSpiral(iNumNodes, m_apNodes);
    int iResult = pIS->init();
    if (iResult != 0) {
        delete pIS;
        pIS = NULL;
    }
    return pIS;
}

//-----------------------------------------------------------------------------
// init
//
int IcoSpiral::init() {
    int iResult = -1;
    if (m_iNumNodes > 0) {
        m_apNodesOut = new IcoNode *[m_iNumNodes];
        
        gridtype iIDHighest = fillMap();
        if (iIDHighest >= 0) {
            m_iCurIndex = 0;
            iResult = 0;
            while ((iResult == 0) && 
                   (m_iCurIndex < m_iNumNodes) &&
                   (iIDHighest >= 0)) {
                iResult = trackNorthChain(iIDHighest);
                if (iResult == 0) {
                    iIDHighest = findHighestUnmarked();
                }
            }
        } else {
             printf("Couldn'd find highest node (shouldn't happen)\n");
        }
    } else {
        printf("No nodes? Num = %d\n", m_iNumNodes);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// fillMap
//
gridtype IcoSpiral::fillMap() {
    gridtype idHighest = -1;
    double dLatMax = dNegInf;
    for (int i = 0; i < m_iNumNodes; i++) {
        IcoNode *p = m_apNodesIn[i];
        m_mID2IndexIn[p->m_lID] = i;
        //        printf("Map: %lld -> %d\n", p->m_lID, i);
        if (p->m_dLat > dLatMax) {
            dLatMax = p->m_dLat;
            idHighest = p->m_lID;
        } else if  (p->m_dLat == dLatMax) {
            if (p->m_dLon < m_apNodesIn[m_mID2IndexIn[idHighest]]->m_dLon) {
                dLatMax = p->m_dLat;
                idHighest = p->m_lID;
            }
        }
    }

    return idHighest;
}

//-----------------------------------------------------------------------------
// trackNorthChain
//
int IcoSpiral::trackNorthChain(gridtype iIDHighest) {
    /*
    printf("[IcoSpiral::trackNorthChain] highest:%lld\n", iIDHighest);
    printf("[IcoSpiral::trackNorthChain] index:%lld\n", m_mID2IndexIn[iIDHighest]);
    */
    IcoNode *pCur =  m_apNodesIn[m_mID2IndexIn[iIDHighest]];
    //    printf("[IcoSpiral::trackNorthChain] puttting id  %lld (lat %f) at index %d\n", iIDHighest, pCur->m_dLat, m_iCurIndex);

    m_apNodesOut[m_iCurIndex++] = pCur;
    pCur->m_iMarked = 1;

    while (pCur != NULL) {
        IcoNode *pNext = NULL;
        for (int i = 0; i < pCur->m_iNumLinks; i++) {
            IcoNode *pNeigh =  m_apNodesIn[m_mID2IndexIn[pCur->m_aiLinks[i]]];
            /*
            printf("cur %lld(%f,%f); neigh %lld(%f,%f); next %lld(%f,%f)\n", 
                   pCur->m_lID, pCur->m_dLon, pCur->m_dLat, 
                   pNeigh->m_lID, pNeigh->m_dLon, pNeigh->m_dLat, 
                   (pNext!= NULL)?pNext->m_lID:-1,(pNext!= NULL)?pNext->m_dLon:dNaN,(pNext!= NULL)?pNext->m_dLat:dNaN); 
            */

            if (pNeigh->m_iMarked == 0) {
                
                if (pNext == NULL) {
                    pNext = pNeigh;
                } else {
                    if (pNeigh->m_dLat > pNext->m_dLat) {
                        pNext = pNeigh;
                    } else if ((pNeigh->m_dLat == pNext->m_dLat) && ((pNeigh->m_dLon >= pCur->m_dLon) || (pNeigh->m_dLon + 2*M_PI >= pCur->m_dLon))) {
                        if  ((pNeigh->m_dLon <= pNext->m_dLon) || (pNeigh->m_dLon - 2*M_PI <= pNext->m_dLon)) {
                            pNext = pNeigh;
                        }
                    }
                }
            }
        }
        //        printf("winner: %lld(%f,%f)\n",                    (pNext!= NULL)?pNext->m_lID:-1,(pNext!= NULL)?pNext->m_dLon:dNaN,(pNext!= NULL)?pNext->m_dLat:dNaN); 
        pCur = pNext;
        if (pCur != NULL) {
            //            printf("[IcoSpiral::trackNorthChain] puttting id  %lld (lat %f) at index %d\n", pCur->m_lID, pCur->m_dLat, m_iCurIndex);
            m_apNodesOut[m_iCurIndex++] = pCur;
            pCur->m_iMarked = 1;
        }
    }    
    printf("Chain ends at index %d\n", m_iCurIndex);
    return 0;
}

//-----------------------------------------------------------------------------
// findHighestUnmarked
//
gridtype IcoSpiral::findHighestUnmarked() {
    gridtype idHighest = -1;
    double dLatMax = dNegInf;
    for (int i = 0; i < m_iNumNodes; i++) {
        IcoNode *p = m_apNodesIn[i];
        if (p->m_iMarked == 0) {
            if (p->m_dLat > dLatMax) {
                dLatMax = p->m_dLat;
                idHighest = p->m_lID;
            } else if  (p->m_dLat == dLatMax) {
                if (p->m_dLon < m_apNodesIn[m_mID2IndexIn[idHighest]]->m_dLon) {
                    dLatMax = p->m_dLat;
                    idHighest = p->m_lID;
                }
            }
        }
    }
    printf("Highest unmarked: %d\n", idHighest);
    return idHighest;
}

