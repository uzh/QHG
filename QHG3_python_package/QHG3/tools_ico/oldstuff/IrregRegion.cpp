#include <stdio.h>
#include <map>
#include <set>

#include "icoutil.h"
#include "types.h"
#include "IcoNode.h"
#include "IrregRegion.h"


bool g_bVerbose = false;
//-----------------------------------------------------------------------------
// constructor
//
IrregRegion::IrregRegion(nodelist &listNodes) 
    : m_listNodes(listNodes) {

    // set all regionIDs to -1
    nodelist::const_iterator it;
    for (it = m_listNodes.begin(); it != m_listNodes.end(); it++) {
        it->second->m_iRegionID = -1;
    }

}

//-----------------------------------------------------------------------------
// add
//
int IrregRegion::add(gridtype iRegion, gridtype iNode) {
    int iResult = 0;

    m_curBorders[iRegion].insert(iNode);
    m_curNodes[iRegion].clear();
    m_newBorders[iRegion].clear();

    return iResult;
}

//-----------------------------------------------------------------------------
// add
//
int IrregRegion::add(gridtype iRegion, nodeset &sN) {
    int iResult = 0;
    nodeset::const_iterator it;
    for (it = sN.begin(); (iResult == 0) && (it != sN.end()); it++) {
        iResult = add(iRegion, *it);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// unmarkBorderNodes
//
void IrregRegion::unmarkBorderNodes(gridtype iRegion) {
    nodeset::const_iterator it;
    for (it = m_curBorders[iRegion].begin(); it != m_curBorders[iRegion].end(); it++) {
        m_listNodes[*it]->m_iMarked=0;
    }
}

//-----------------------------------------------------------------------------
// markBorderNodes
//
void IrregRegion::markBorderNodes(gridtype iRegion) {
    nodeset::const_iterator it;
    for (it = m_curBorders[iRegion].begin(); it != m_curBorders[iRegion].end(); it++) {
        m_listNodes[*it]->m_iMarked++;
    }
}


//-----------------------------------------------------------------------------
// collectFreeNeighbors
//
int IrregRegion::collectFreeNeighbors(gridtype iNode, nodeset &sNeighbors) {
    int iResult = 0;
    nodelist::const_iterator it = m_listNodes.find(iNode);
    if (it != m_listNodes.end()) {
        IcoNode *pIN = it->second;
        for (int i = 0; i < pIN->m_iNumLinks; i++) {
            if (m_listNodes[pIN->m_aiLinks[i]]->m_iRegionID < 0) {
                sNeighbors.insert(pIN->m_aiLinks[i]);
            }
        }

        iResult = (int)sNeighbors.size();
    } else {
        printf("Unregistered node: %d\n", iNode);
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// collectFreeNeighbors
//
int IrregRegion::collectFreeNeighbors(nodeset &sNodes, nodeset &sNeighbors) {
    int iResult = 0;
    nodeset::const_iterator it;
    for (it = sNeighbors.begin(); (iResult >= 0) && (it != sNeighbors.end()); it++) {
        iResult = collectFreeNeighbors(*it, sNeighbors);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// grow
// - unmark allborders (just to be sure)
// - let all regions mark their border nodes
// - for each region:
//   - add all non-conflicting border nodes region and
//     add their neighbors to new border list
//   - add all conflicting nodes to conflict list
// - resolve conflicts (adding decided nodes to new border list)
// - move contents of new border to cur border and clear new borders
//
int IrregRegion::grow() {
    ulong iResult = 0;
    regionnodes::iterator it;
    regionnodes::iterator itR;
    
    // first: clear all border marks
    for (it = m_curBorders.begin(); it != m_curBorders.end(); it++) {
        unmarkBorderNodes(it->first);
    }

    // every region marks its border nodes
    for (it = m_curBorders.begin(); it != m_curBorders.end(); it++) {
        markBorderNodes(it->first);
        //        printf("Bordersize %lld: %zd\n", it->first,m_curBorders[it->first].size());
    }

    // every region adds singly-marked border nodes ot itself,
    // and saves multiply marked border nodes
    for (itR = m_curBorders.begin(); itR != m_curBorders.end(); itR++) {
        nodeset::iterator itN;
        for (itN = itR->second.begin();itN != itR->second.end(); itN++) {
             gridtype iNode = *itN;                                   
             IcoNode *pIN = m_listNodes[iNode];

             if (pIN->m_iRegionID < 0){
                 if (pIN->m_iMarked == 1) {
                     pIN->m_iRegionID = itR->first;
                     m_curNodes[itR->first].insert(iNode);
                     collectFreeNeighbors(iNode, m_newBorders[itR->first]);
                 } else {
                     m_curConflicts[iNode].insert(itR->first);
                 }
             }
        }
        itR->second.clear();
        // if iResult == 0, nothing has been added to any of the regions
        iResult += m_newBorders[itR->first].size();
        //        printf("newBordersize %lld: %zd\n", itR->first,m_newBorders[itR->first].size());
    }


    // resolve conflicts
    conflictnodes::iterator itC;
    for (itC = m_curConflicts.begin(); itC != m_curConflicts.end(); itC++) {
        if (g_bVerbose) printf("conflict for %d: regs ", itC->first);
        gridtype  iRegMin = -1;
        size_t  lMin = 100000000;
        std::set<gridtype>::iterator itX;
        for (itX = itC->second.begin(); itX != itC->second.end(); itX++) {
            if (g_bVerbose) printf(" %d (%zd)", *itX, m_curNodes[*itX].size());
            if (m_curNodes[*itX].size() < lMin) {
                iRegMin = *itX;
                lMin = m_curNodes[*itX].size();
            }
        }
        if (g_bVerbose) printf("-> %d\n", iRegMin);
        collectFreeNeighbors(itC->first, m_newBorders[iRegMin]);
        m_curNodes[iRegMin].insert(itC->first);
        m_listNodes[itC->first]->m_iRegionID = iRegMin;
    }
    m_curConflicts.clear();

    // update borders
    for (itR = m_newBorders.begin(); itR != m_newBorders.end(); itR++) {
        //        printf("Inserting %zd elements to border %lld\n", m_newBorders[itR->first].size(),itR->first); 
        m_curBorders[itR->first].insert(itR->second.begin(),itR->second.end());
        itR->second.clear();
    }
    return (int)iResult;
}


//-----------------------------------------------------------------------------
// listN
//
void IrregRegion::listN(bool bWithLinks) {
    regionnodes::const_iterator it;
    for (it = m_curNodes.begin();it != m_curNodes.end(); ++it) {
        printf("R %d (%zd):", it->first, it->second.size());
        nodeset::const_iterator itN;
        for (itN = it->second.begin();itN != it->second.end(); ++itN) {
            printf(" %d", *itN);
            if (bWithLinks) {
                printf(" (");
                IcoNode *pIN = m_listNodes[*itN];
                for (int i = 0; i < pIN->m_iNumLinks; i++) {
                    printf("%d ", pIN->m_aiLinks[i]);
                }
                printf(") ");
            }
        }
        printf("\n");
    }
}
//-----------------------------------------------------------------------------
// listSizes
//
void IrregRegion::listSizes() {
    regionnodes::const_iterator it;
    for (it = m_curNodes.begin();it != m_curNodes.end(); ++it) {
        char c = '#';
        if (it->first < 10) {
            c = (char)('0'+ it->first);
        } else if (it->first < 36) {
            c = (char)('A'+it->first-10);
        } else if (it->first < 62) {
            c = (char)('a'+it->first-36);
        } 
        printf("%d(%c):%zd ", it->first, c, it->second.size());
    }
    printf("\n");
}

//-----------------------------------------------------------------------------
// collectNeighbors
//
int IrregRegion::collectNeighbors(gridtype iRegion, gridtype iNode) {
    int iResult = 0;
    nodelist::const_iterator it = m_listNodes.find(iNode);
    if (it != m_listNodes.end()) {
        IcoNode *pIN = it->second;
        for (int i = 0; i < pIN->m_iNumLinks; i++) {
            gridtype iOtherID = pIN->m_aiLinks[i];
            gridtype iOtherRegion = m_listNodes[iOtherID]->m_iRegionID;
            if (iOtherRegion != iRegion) {
                m_wNeighbors[iRegion][iOtherRegion][iOtherID]++;
            }
        }
    } else {
        printf("Unregistered node: %d\n", iNode);
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// collectNeighborsPerRegion
//
int IrregRegion::collectNeighborsPerRegion() {
    int iResult = 0;
    m_wNeighbors.clear();
    regionnodes::const_iterator itR;
    for (itR = m_curNodes.begin(); itR != m_curNodes.end(); itR++) {
        nodeset::const_iterator itN;
        for (itN = itR->second.begin(); itN != itR->second.end(); itN++) {
            iResult = collectNeighbors(itR->first, *itN);
        }
    }


    return iResult;
}

//-----------------------------------------------------------------------------
// equalize
//
int IrregRegion::equalize(bool bHigher) {
    int iResult = 0;

    size_t iNormSize = m_listNodes.size()/m_curNodes.size()+(bHigher?1:0);
 
    iResult = collectNeighborsPerRegion();
    //    listWN();

    // find largest region
    int iMaxRegionID=-1;
    size_t iMaxSize = 0;
    int iMinRegionID=-1;
    size_t iMinSize = 1000000;
    m_Unused.clear();
    regionnodes::const_iterator itR;
    for (itR = m_curNodes.begin(); itR != m_curNodes.end(); itR++) {
        if (itR->second.size() > iMaxSize) {
            iMaxRegionID = itR->first;
            iMaxSize = itR->second.size();
        }
        if (itR->second.size() < iMinSize) {
            iMinRegionID = itR->first;
            iMinSize = itR->second.size();
        }
        m_Unused[itR->first] = true;
    }
    printf("Largest Region: %d (%zd)\n", iMaxRegionID, iMaxSize);
    printf("Smallest region: %d (%zd)\n", iMinRegionID, iMinSize);


    // determine number of nodes to be swapped
    int iOverFlow  = (int)(m_curNodes[iMaxRegionID].size() - iNormSize);
    int iUnderFlow = (int)(iNormSize - m_curNodes[iMinRegionID].size());

    printf("Overflow: %d\n", iOverFlow);
    printf("Underflow: %d\n", iUnderFlow);

    int iSendNum = 0;
    if (iOverFlow > iUnderFlow) {
        iSendNum = iUnderFlow;
    } else {
        iSendNum = iOverFlow;
    }
    printf("Num to swap: %d\n", iSendNum);
    iResult = iSendNum;

    // find Path R[0], ... R[m-1] with
    // - R[i] is neighbor with R[i+1]
    // - R[0] = iMaxRegionID;
    // - R[m-1] = iMinRegionID;
    std::vector<gridtype> vPath;
    vPath.push_back(iMaxRegionID);
    findPathDijk(iMaxRegionID, iMinRegionID, vPath);
    std::queue<gridtype> qPath;
    if (g_bVerbose) printf("Path:\n");
    for (unsigned int i = 0; i < vPath.size(); i++) {
        if (g_bVerbose) printf("  R%d: %d\n",i, vPath[i]);
        qPath.push(vPath[i]);
    }
    while (qPath.size() > 1) {
        if (g_bVerbose) printf("qPath size: %zd\n", qPath.size());
        iSendNum = swapNodes(iSendNum, qPath);
        collectNeighborsPerRegion();
    }

    printf("finally exchanged %d\n", iResult - iSendNum);
    collectNeighborsPerRegion();
    //    listWN();
    //    listN();
    return iResult;
}

//-----------------------------------------------------------------------------
// swapNodes
//   returns the number of actually swapped nodes
//
int IrregRegion::swapNodes(int iSendNum, std::queue<gridtype> &qPath) {
    gridtype iRPrev = qPath.front();
    qPath.pop();
    gridtype iRCur = qPath.front();
    // find best iSendNum nodes of iRPrev in iRCur's neighborhood
    nodeset sN;
    findBestTransferNodes(iRPrev, iRCur, iSendNum, sN);
    
    int iActualSwaps= 0;
    if (g_bVerbose) printf("best nodes %d ->%d: ", iRPrev, iRCur);
    nodeset::iterator itN;
    for (itN = sN.begin(); itN != sN.end(); itN++) {
        nodeset::iterator itN2 = m_curNodes[iRPrev].find(*itN);
        if (itN2 != m_curNodes[iRPrev].end()) {
            if (g_bVerbose) printf(" %d", *itN);
            m_curNodes[iRPrev].erase(itN2);
            m_curNodes[iRCur].insert(*itN);
            m_listNodes[*itN]->m_iRegionID = iRCur;
            iActualSwaps++;
        } else {
            printf("[IrregRegion::swapNodes] bad id %d not in region %d - shouldn't happen\n", *itN, iRPrev);
        }
    }
    if (g_bVerbose)  printf("\n");
    return iSendNum-iActualSwaps;
}

//-----------------------------------------------------------------------------
// findBestTransferNodes
//
void IrregRegion::findBestTransferNodes(gridtype iRPrev, gridtype iRCur, int iSendNum, nodeset &sN) {
    nodecounts &sNC = m_wNeighbors[iRCur][iRPrev];

    bool bHasExceeders = true;
    int iToDo = iSendNum;
    // counts exceeding 1
    while ((iToDo > 0) && bHasExceeders) {
        nodecounts::const_iterator itN;
        int iMax = 1;
        gridtype iMaxID=-1;
        for (itN = sNC.begin(); itN != sNC.end(); itN++) {
            if ((itN->second > iMax) && doesNotSplitRegion(iRCur, itN->first, 4)) {
                iMax = itN->second;
                iMaxID = itN->first;
            }
        }
        if (iMaxID >= 0) {
            // removal of node must not split region!
            sN.insert(iMaxID);
            sNC[iMaxID] = 0; //don't use this one again
            iToDo--;;
            if (g_bVerbose) printf("  %d", iMaxID);
            bHasExceeders = true;
        } else {
            bHasExceeders = false;
        }

    }

    nodecounts::iterator itN;
    for (itN = sNC.begin(); (iToDo > 0) && (itN != sNC.end()); itN++) {
        if (itN->second > 0) {
            if (doesNotSplitRegion(iRCur, itN->first, 4)) {
                // removal of node must not split region!
                sN.insert(itN->first);
                if (g_bVerbose) printf("  %d", itN->first);
                itN->second = 0;
                iToDo--;
            }
        }
    }

}

//-----------------------------------------------------------------------------
// collectNodeNeighbors
//
int IrregRegion::collectNodeNeighbors(gridtype iNode, nodeset &sN, gridtype iAvoid) {
    IcoNode *pIN = m_listNodes[iNode];
    for (int i = 0; i < pIN->m_iNumLinks; i++) {
        if ( pIN->m_aiLinks[i] != iAvoid) {
            sN.insert(pIN->m_aiLinks[i]);
        }
    }
    return (int)sN.size();
}

//-----------------------------------------------------------------------------
// removeBadNeighbors
//  good neighbors have 2 or more neighbors
// 
int IrregRegion::removeBadNeighbors(nodeset &sN, gridtype iRegion) {
    nodeset sBad;
    // find the bad guys
    nodeset::const_iterator it;
    for (it = sN.begin(); it != sN.end(); it++) {

        int iNumNeigh = 0;
        nodeset::const_iterator it2;
        for (it2 = sN.begin(); it2 != sN.end(); it2++) {
            bool bSearch = true;
            IcoNode *pIN2 = m_listNodes[*it2];
            for (int i = 0; bSearch && (i < pIN2->m_iNumLinks); i++) {
                if (pIN2->m_aiLinks[i] == *it) {
                    bSearch = false;
                    iNumNeigh++;
                }
            }
        }
        
        
        if (iNumNeigh < 2) {
            sBad.insert(*it);
        }
    }
    for (it = sN.begin(); it != sN.end(); it++) {
        if (iRegion != m_listNodes[*it]->m_iRegionID) {
            sBad.insert(*it);
        }
    }

    // remove the bad guys
    for (it = sBad.begin(); it != sBad.end(); it++) {
        nodeset::iterator it2 = sN.find(*it);
        if (it2 != sBad.end()) {
            sN.erase(*it2);
        }
    }
    
    return (int)sN.size();
}

//-----------------------------------------------------------------------------
// countComponents
//  
// 
int IrregRegion::countComponents(nodeset &sN) {
    IcoNode *pINFront;
    IcoNode *pINBack;

    nodeset sUsed;
    int iNumComponents = 0;
    while (sUsed.size() < sN.size()) {
        /*
        nodeset::const_iterator it0;
        printf("sN :");
        for (it0 = sN.begin();  (it0 != sN.end()); it0++) {
            printf("  %lld", *it0);
        }
        printf("\n");
        printf("sU :");
        for (it0 = sUsed.begin();  (it0 != sUsed.end()); it0++) {
            printf("  %lld", *it0);
        }
        printf("\n");
        */
        
        
        // find an unused node
        bool bSearchUnused  =true;
        nodeset::const_iterator it;
        for (it = sN.begin(); bSearchUnused && (it != sN.end()); ) {
            nodeset::const_iterator itu = sUsed.find(*it);
            if (itu == sUsed.end()) {
                bSearchUnused = false;
            } else {
                ++it;
            }
        }

        if (!bSearchUnused) {
            
            sUsed.insert(*it);
            pINFront = m_listNodes[*it];
            pINBack  = m_listNodes[*it];
            
            //            printf("Component for %lld\n", *it);
            bool bFoundSomething = true;
            while (bFoundSomething) {
                nodeset::const_iterator it2;
                for (it2 = sN.begin();  (it2 != sN.end()); it2++) {
                    bFoundSomething = false;
                    nodeset::const_iterator itu = sUsed.find(*it2);
                    if (itu== sUsed.end()) {
                        IcoNode *pINCur = m_listNodes[*it2];
                        //                        printf("  testing %lld\n", *it2);
                        bool bLinkSearch = true;
                        for (int i = 0; bLinkSearch && (i < pINCur->m_iNumLinks); i++) {
                            gridtype iCur = pINCur->m_aiLinks[i];
                            // connected to front?
                            if (pINFront->m_lID == iCur) {
                                bLinkSearch = false;
                                // new becomes front
                                pINFront = pINCur;
                                // new is now used
                                sUsed.insert(*it2);
                                //                                printf("  front %lld\n", *it2);
                            }
                            if (bLinkSearch) {
                                // connected to back?
                                if (pINBack->m_lID == iCur) {
                                    bLinkSearch = false;
                                    // new becomes back
                                    pINBack = pINCur;
                                    // new is now used
                                    sUsed.insert(*it2);
                                    //                                    printf("  back %lld\n", *it2);
                                }
                            }
                        }
                        bFoundSomething = !bLinkSearch;
                        /*
                        if (bLinkSearch) {
                            printf("   nope\n");
                        }
                        */
                    } // it2 used
                }
            }
            iNumComponents++;
            //            printf("finished component\n");
        }
    }
    return iNumComponents;
}

void showNodes(const char *pCaption, nodeset &sN) {
    nodeset::const_iterator it0;
    printf("%s: %zd\n", pCaption, sN.size());
    for (it0 = sN.begin();  it0 != sN.end(); it0++) {
        printf("  %d", *it0);
    }
    printf("\n");

}

//-----------------------------------------------------------------------------
// doesNotSplitRegion
//
bool IrregRegion::doesNotSplitRegion(gridtype iRegion, gridtype iNode, int iConnectivity) {
    bool bNoSplit = true;
    nodeset sN;
    printf("checking %d\n", iNode);
    // collect all immediate neighbors
    collectNodeNeighbors(iNode, sN, iNode);
        showNodes("Direct", sN);

    if (iConnectivity <= 4) {
        nodeset sN2;
        nodeset::const_iterator it;
        for (it = sN.begin(); it != sN.end(); it++) {
            collectNodeNeighbors(*it, sN2, iNode);
        }
                showNodes("L1", sN2);
        if (iConnectivity < 4) {
            nodeset sN3;
            nodeset::const_iterator it2;
            for (it2 = sN2.begin(); it2 != sN2.end(); it2++) {
                collectNodeNeighbors(*it2, sN3, iNode);
            }
            
            sN2.insert(sN3.begin(), sN3.end());
                        showNodes("L2", sN3);
                        
        }
                
        sN.insert(sN2.begin(), sN2.end());
    }

        showNodes("total", sN);
    // remove bad neighbors: must have at least 2 neighbors in set, and must have region iRegion
    removeBadNeighbors(sN, iRegion);
            showNodes("after remove", sN);
  

    int iC = countComponents(sN);
    bNoSplit = (iC == 1);
        printf("%d componentsssssss\n", iC);
    return bNoSplit;
}

//-----------------------------------------------------------------------------
// findPathDijk
//  use dijkstra's algorithm to find path from region iStart to region iEnd 
//  in the neighborhood graph of regions
//   
//  
bool IrregRegion::findPathDijk(gridtype iStart, gridtype iEnd, std::vector<gridtype> &vPath0) {
    int iNumR = (int)m_curNodes.size();
    int iMaxDist = iNumR + 1;
    std::map<gridtype, bool> mAlive;           // is region still a candidate?
    std::map<gridtype, int> mDistances;        // min distance to region
    std::map<gridtype, gridtype> mPreviousRegs;  // where min distance cam from

    // initialize auxiliary structs
    regionnodes::iterator it;
    for (it = m_curNodes.begin(); it != m_curNodes.end(); ++it) {
        mAlive[it->first] = true;         // everybody is alive
        mDistances[it->first] = iMaxDist; // all distances are inifinity
        mPreviousRegs[it->first] = -1;    // no arrival yet
    }
    
    // start set distance to first node to 0
    mDistances[iEnd] = 0;
    while (iNumR > 0) {
        //  from live candidates select Region with minimum distance value
        int iMinDist = iMaxDist;
        gridtype iMinReg = -1;
        for (it = m_curNodes.begin(); it != m_curNodes.end(); ++it) {
            if (mAlive[it->first]){
                if  (mDistances[it->first] < iMinDist) {
                    iMinDist = mDistances[it->first];
                    iMinReg = it->first;
                }
            }
        }

        // loop through all (live) neighboring regions and update distance values,
        // if iMinDist+1 < current dist
        regionnodecounts::iterator it2;
        for (it2 = m_wNeighbors[iMinReg].begin();it2 != m_wNeighbors[iMinReg].end(); ++it2) {
            if (mAlive[it2->first]){
                if (mDistances[it2->first] > mDistances[iMinReg]+1) {
                    mDistances[it2->first] = mDistances[iMinReg]+1;
                    mPreviousRegs[it2->first] = iMinReg;
                }
            }
        }
        // iMinReg has done its job, kill it
        mAlive[iMinReg] = false;
        iNumR--;
    }

    // now construct the path from the info contained in mPreviousRegs
    int iCur = iStart;
    while (iCur != iEnd) {
        vPath0.push_back(iCur);
        iCur = mPreviousRegs[iCur];
    }
    vPath0.push_back(iEnd);


    
    return true;
}

//-----------------------------------------------------------------------------
// listWN
//
void IrregRegion::listWN() {
    printf("++++\n");
    weightedneighbors::const_iterator itW;
    for (itW = m_wNeighbors.begin(); itW != m_wNeighbors.end(); itW++) {
        printf("R %d\n", itW->first);
        regionnodecounts::const_iterator itR;       
        for (itR = itW->second.begin(); itR != itW->second.end(); itR++) {
            printf("  R' %d:", itR->first);
            nodecounts::const_iterator itC;
            for (itC = itR->second.begin(); itC != itR->second.end(); itC++) {
                printf(" %d:%d", itC->first, itC->second);
            }
            printf("\n");
        }
    }
}

