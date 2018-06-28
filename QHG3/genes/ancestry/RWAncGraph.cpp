#include <stdio.h>
#include <string.h>

#include <set>
#include <map>
#include <vector>

#include "utils.h"
#include "colors.h"
#include "LineReader.h"
#include "BufReader.h"
#include "BufWriter.h"
#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "RWAncGraph.h"


//----------------------------------------------------------------------------
// merge
//   
int  AncestorNode::merge(AncestorNode *pAN) {
    int iResult = 0;
    
    // 
    if (m_aParents[MOM] < 0) {
        m_aParents[MOM] = pAN->m_aParents[MOM];
    } else {
        if ((pAN->m_aParents[MOM] > 0) && (m_aParents[MOM] != pAN->m_aParents[MOM])) {
            printf("MomProb: %ld <-> %ld\n", m_aParents[MOM], pAN->m_aParents[MOM]);
            iResult = -1;
        }
    }
    if (m_aParents[DAD] < 0) {
        m_aParents[DAD] = pAN->m_aParents[DAD];
    } else {
        if ((pAN->m_aParents[DAD] > 0) && (m_aParents[DAD] != pAN->m_aParents[DAD])) {
            printf("DadProb: %ld <-> %ld\n", m_aParents[DAD], pAN->m_aParents[DAD]);
            iResult = -1;
        }
    }
    if (iResult == 0) {
        m_sChildren.insert(pAN->m_sChildren.begin(), pAN->m_sChildren.end());
    }

    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//   
RWAncGraph::RWAncGraph() 
    : m_lListOffset(0) {

}


//----------------------------------------------------------------------------
// destructor
//   
RWAncGraph::~RWAncGraph() {
    ancnodelist::iterator it;
    for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
        delete it->second;
    }
}


//----------------------------------------------------------------------------
// addSelected
//   add ID to set of selected
//
void RWAncGraph::addSelected(idtype iSelectedID) {
    m_sSelected.insert(iSelectedID);
}


//----------------------------------------------------------------------------
// setSelected
//  add set of IDs to set of selected
//
void RWAncGraph::setSelected(const idset &sSelected) {
    m_sSelected.clear();
    m_sSelected.insert(sSelected.begin(), sSelected.end());
}





//----------------------------------------------------------------------------
// merge
//   The argument pAG must have a later set of selected IDs than this
//
int RWAncGraph::merge(RWAncGraph *pAG, bool bSets) {
    int iResult = 0;
    const ancnodelist &mOther = pAG-> getMap();
    ancnodelist::const_iterator it;
    for (it = mOther.begin(); (iResult == 0) && (it != mOther.end()); ++it) {
        AncestorNode *pAN = getAncestorNode(it->first);
        iResult = pAN->merge(it->second);
        delete it->second;
    }
    if (iResult == 0) {
        if (bSets) {
            m_sProgenitors.clear();
            m_sRoots.clear();
            m_sLeaves.clear();
            setSelected(pAG->getSelected());
            prune(false);
        } else {
            m_sProgenitors.insert(pAG->getProgenitors().begin(), pAG->getProgenitors().end());
            m_sSelected.insert(pAG->getSelected().begin(), pAG->getSelected().end());
            m_sRoots.insert(pAG->getRoots().begin(), pAG->getRoots().end());
            m_sLeaves.insert(pAG->getLeaves().begin(), pAG->getLeaves().end());
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getAncestorNode
//   
AncestorNode *RWAncGraph::getAncestorNode(idtype iID) {
    AncestorNode *pFN = NULL;
    ancnodelist::const_iterator it = m_mIndex.find(iID);
    if (it == m_mIndex.end()) {
        pFN = new AncestorNode(iID);
        m_mIndex[iID] = pFN;
    } else {
        pFN = it->second;
    }
    return pFN;
}


//----------------------------------------------------------------------------
// showTree
//   
void RWAncGraph::showTree() {
    ancnodelist::const_iterator it;
    for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
        printf("  %ld [%ld,%ld] %d{", 
               it->first, 
               it->second->getMom(), 
               it->second->getDad(),
               it->second->m_iGender);
        idset_cit it2;
        char s[2];
        s[0] = '\0';
        s[1] = '\0';
        for (it2 = it->second->m_sChildren.begin(); it2 != it->second->m_sChildren.end(); ++it2) {
            printf("%s%ld", s, *it2);
            *s = ',';
        }
        printf("}\n");
    }
}

    
//----------------------------------------------------------------------------
// showAncestorInfo
//   
void RWAncGraph::showAncestorInfo(bool bFull) { 
    if (bFull) {
        showTree();
    }
    idset_cit it3;
    printf("Graph has %zd nodes\n", getMap().size());
    // show progenitors
    printf("                   %zd progenitors: ", m_sProgenitors.size());
    for (it3 = m_sProgenitors.begin(); it3 != m_sProgenitors.end(); ++it3) {
        printf(" %ld",*it3);
    }
    printf("\n");

    // show (part of) selected
    printf("                   %zd selected:", m_sSelected.size());
    uint iNumS = (uint)m_sSelected.size();
    if (!bFull && (iNumS > 16)) {
        iNumS = 16;
    }
    uint iC = 0;
    for (it3 = m_sSelected.begin(); (iC < iNumS) && (it3 != m_sSelected.end()); ++it3) {
        printf(" %ld",*it3);
        iC++;
    }
    if (m_sSelected.size() > iNumS) {
        printf(" ...");
    }
    printf("\n");

    // show roots
    printf("                   %zd roots: ", m_sRoots.size());
    for (it3 = m_sRoots.begin(); it3 != m_sRoots.end(); ++it3) {
        printf(" %ld",*it3);
    }
    printf("\n");

    // show (part of) leaves
    printf("                   %zd leaves:", m_sLeaves.size());
    uint iNumE = (uint)m_sLeaves.size();
    if (!bFull && (iNumE > 16)) {
        iNumE = 16;
    }
    iC = 0;
    for (it3 = m_sLeaves.begin(); (iC < iNumE) && (it3 != m_sLeaves.end()); ++it3) {
        printf(" %ld",*it3);
        iC++;
    }
    if (m_sLeaves.size() > iNumE) {
        printf(" ...");
    }
    printf("\n");

}
    

//----------------------------------------------------------------------------
// markSurvivingLine
//   
void RWAncGraph::markSurvivingLine(idtype iID) {
    AncestorNode *pFN = getAncestorNode(iID);
    if (!pFN->m_bSelected) {
        pFN->m_bSelected = true;
        idtype iMom = pFN->getMom();
        if (iMom > 0) {
            markSurvivingLine(iMom);
        } 
        idtype iDad = pFN->getDad();
        if (iDad > 0) {
            markSurvivingLine(iDad);
        }
        if ((iDad <= 0) && (iMom <= 0)) {
            m_sProgenitors.insert(iID);
        }
    }
}




//----------------------------------------------------------------------------
// prune
//   
int RWAncGraph::prune(bool bCut) {
    m_sProgenitors.clear();
    m_sRoots.clear();

    printf("Making surviving lines...\n");
    // mark all ancestors of the survivors to find the progenitors
    idset_it it;
    for (it = m_sSelected.begin(); it != m_sSelected.end(); ++it) {
        markSurvivingLine(*it);
    }


    
    if (bCut) {
        printf("Collecting dead ends...\n");
        // find all nodes that do not lead to survivors
        idset sDeadEnds;
        ancnodelist::iterator it2;
        for (it2 = m_mIndex.begin(); it2 != m_mIndex.end(); ++it2) {
            if (!it2->second->m_bSelected) {
                // remove from mother's children
                if (it2->second->m_aParents[MOM] > 0) {
                    AncestorNode *pFN = findAncestorNode(it2->second->m_aParents[MOM]);
                    if (pFN != NULL) {
                        pFN->m_sChildren.erase(it2->first);
                    } else {
                        //                        printf("no node for %d\n", it2->second->m_aParents[MOM]);
                    }
                }
                // remove from father's children
                if (it2->second->m_aParents[DAD] > 0) {
                    AncestorNode *pFN = findAncestorNode(it2->second->m_aParents[DAD]);
                    if (pFN != NULL) {
                        pFN->m_sChildren.erase(it2->first);
                    } else {
                        //                        printf("no node for %d\n", it2->second->m_aParents[DAD]);
                    }
                }
                // remember for deletion from lists
                sDeadEnds.insert(it2->first);
            }
        }

        printf("Removing %zd dead ends...\n", sDeadEnds.size());
        // remove the dead branches from the tree and the list of originators
        idset_it it3;
        for (it3 = sDeadEnds.begin(); it3 != sDeadEnds.end(); ++it3) {
            delete getAncestorNode(*it3);
            m_mIndex.erase(*it3);
            m_sProgenitors.erase(*it3);
            
        }
        printf("Remaining nodes:  %zd\n", m_mIndex.size());
    }
    printf("Determining roots and leaves...\n");
    // collect all roots (no parents) and all leaves (no children)
    collectRootsLeaves();
    
    
    return (int)m_mIndex.size();
}    


//----------------------------------------------------------------------------
// collectRootsLeaves
//   
void RWAncGraph::collectRootsLeaves() {
    m_sRoots.clear();
    m_sLeaves.clear();
    ancnodelist::iterator it;
    for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
        if ((it->second->getMom() == 0) && (it->second->getDad() == 0)){
            m_sRoots.insert(it->first);
        } else if (((it->second->getMom() != 0) && (it->second->getDad() == 0)) ||
                   ((it->second->getMom() == 0) && (it->second->getDad() != 0))) {
            printf("SinglePArent node: %ld\n", it->second->m_iID);
        }
        if (it->second->m_sChildren.size() == 0) {
            m_sLeaves.insert(it->first);
        }
    }
}

/*
//----------------------------------------------------------------------------
// saveNode
//
int RWAncGraph::saveNode(BufWriter *pBW, AncestorNode *pAN) {
    char sNodeBuf[5*sizeof(int)];
    int iNumChildren = (int) pAN->m_sChildren.size();
    char *p = sNodeBuf;
    //            printf("ID %d; mom %d; dad %d; nc %d\n", it->first, it->second->m_aParents[MOM], it->second->m_aParents[DAD], iNumChildren);
    p = putMem(p, &pAN->m_iID, sizeof(int));
    p = putMem(p, &pAN->m_aParents[MOM], sizeof(int));
    p = putMem(p, &pAN->m_aParents[DAD], sizeof(int));
    p = putMem(p, &pAN->m_iGender,       sizeof(int));
    p = putMem(p, &iNumChildren, sizeof(int));

    pBW->addChars(sNodeBuf, 5*sizeof(int));        
    std::set<int>::const_iterator it2;
    for (it2 = pAN->m_sChildren.begin(); it2 != pAN->m_sChildren.end(); ++it2) {
        int iC = *it2;
        pBW->addChars((char *) & iC, sizeof(int));
    }
    
    if (iNumChildren == 0) {
        //    printf("Adding leaf %d\n", pAN->m_iID);
        m_sLeaves.insert(pAN->m_iID);
    }
    
    return 0;
}
*/

//----------------------------------------------------------------------------
// saveBin
//   Format:
//     File         ::= <Header><NodeList><SpecialLists>
//     SpecialLists ::= <Progenitors><Selected><Roots><Leaves>
//     Header       ::= "AGRB"<ListOffset><NumNodes> 
//     NodeList     ::= <NodeData>*
//     NodeData     ::= <ID><MomID><DadID><Gender><Children>
//     Children     ::= <IDList>
//     Progenitors  ::= <IDList>
//     Selected     ::= <IDList>
//     Roots        ::= <IDList>
//     Leaves       ::= <IDList>
//     IDList       ::= <NumIDs><ID>*
//   NumNodes : int
//   ID       : int
//   MomID    : int
//   DadID    : int
//   Gender   : int
//   NumIDs   : int
//
int RWAncGraph::saveBin(BufWriter *pBW, bool bWriteHeader, bool bWriteNodes, bool bWriteFooter) {
    int iResult = 0;

    if (bWriteHeader) {
        pBW->addLine("AGRB");
        long lDummy=0;
        pBW->addChars((char *)&lDummy, sizeof(long));        
        long iNumNodes = (long)  m_mIndex.size();
        pBW->addChars((char *)&iNumNodes, sizeof(long));        
    }
    
    if (bWriteNodes) {
        printf("Writing %zd nodes\n", m_mIndex.size());
        ancnodelist::const_iterator it;
        for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
            saveNode(pBW, it->second);

            if (it->second->m_sChildren.empty()) {
                m_sLeaves.insert(it->first);
            }
        }
    }

    if (bWriteFooter) {
        m_lListOffset = pBW->getPos();
        printf("Writing %zd progs, %zd sels, %zd roots %zd leaves\n", 
               m_sProgenitors.size(), m_sSelected.size(), m_sRoots.size(), m_sLeaves.size());
        idset_cit it2;
        int iNumProgs = (int)m_sProgenitors.size();
        pBW->addChars((char *) &iNumProgs, sizeof(int));
        for (it2 = m_sProgenitors.begin(); it2 != m_sProgenitors.end(); ++it2) {
            idtype iP = *it2;
            pBW->addChars((char *) &iP, sizeof(int));
        }
        int iNumSelected = (int)m_sSelected.size();
        pBW->addChars((char *) &iNumSelected, sizeof(int));
        for (it2 = m_sSelected.begin(); it2 != m_sSelected.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    
        int iNumRoots = (int)m_sRoots.size();
        pBW->addChars((char *) &iNumRoots, sizeof(int));
        for (it2 = m_sRoots.begin(); it2 != m_sRoots.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    
        int iNumLeaves = (int)m_sLeaves.size();
        pBW->addChars((char *) &iNumLeaves, sizeof(int));
        for (it2 = m_sLeaves.begin(); it2 != m_sLeaves.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    }    
    return iResult;
}

//----------------------------------------------------------------------------
// saveBin
//   Format:
//     File         ::= <Header><NodeList><SpecialLists>
//     SpecialLists ::= <Progenitors><Seleced><Roots><Leaves>
//     Header       ::= "AGRB" 
//     NodeList     ::= <NumNodes><NodeData>*
//     NodeData     ::= <ID><MomID><DadID><Gender><Children>
//     Children     ::= <IDList>
//     Progenitors  ::= <IDList>
//     Selected     ::= <IDList>
//     Roots        ::= <IDList>
//     Leaves       ::= <IDList>
//     IDList       ::= <NumIDs><ID>*
//   NumNodes : int
//   ID       : int
//   MomID    : int
//   DadID    : int
//   Gender   : int
//   NumIDs   : int
//
int RWAncGraph::saveBin(const char *pFileName) {
    int iResult = -1;

    BufWriter *pBW = BufWriter::createInstance(pFileName);
    if (pBW != NULL) {
        iResult = saveBin(pBW, true, true, true);
        delete pBW;   
        iResult = writeListOffset(pFileName, m_lListOffset);
    } else {
        printf("%sCouldn't open [%s] for writing\n", RED, pFileName);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// loadBin
//   
int RWAncGraph::loadBin(const char *pFileName) {
    int iResult = -1;
    
    BufReader *pBR = BufReader::createInstance(pFileName, 2048);
    if (pBR != NULL) {
        char sMagic[3];
        // read and check magic
        iResult = pBR->getBlock(sMagic, 4);
        if ((iResult == 0) && (strncmp(sMagic, "AGRB", 4) == 0)) {
            long lListOffset = -1;
            iResult = pBR->getBlock((char*)&lListOffset, sizeof(long));
            if (iResult == 0) {
                // read number of nodes
                long iNumNodes;
                iResult = pBR->getBlock((char*)&iNumNodes, sizeof(long));
                if (iResult == 0) {
                    
                    // read nodes
                    for (int i = 0; (iResult == 0) && (i < iNumNodes); i++) {
                        if (readNode(pBR) == NULL) {
                            iResult = -1;
                        }
                    }
                } else {
                    printf("%sCouldn't read number of nodes\n", RED);
                }
            } else {
                printf("%sCouldn't read list offset\n", RED);
            }
            
            if (iResult == 0) {
                //@@                printf("now reading progenitors\n");
                iResult = readIDSetBin(pBR, m_sProgenitors);
            }
             
            if (iResult == 0) {
                //@@                printf("now reading selected\n");
                iResult = readIDSetBin(pBR, m_sSelected);
            }
            if (iResult == 0) {
                //@@                printf("now reading roots\n");
                iResult = readIDSetBin(pBR, m_sRoots);
            }
            if (iResult == 0) {
                //@@                printf("now reading leaves\n");
                iResult = readIDSetBin(pBR, m_sLeaves);
            }

            if (iResult == 0) {
                //                printf("Successfully loaded [%s]\n", pFileName);
            }
        
        } else {
            iResult = 1;
            printf("%sExpected Magic (FTR)\n", RED);
        }
        delete pBR;
    } else {
        printf("%sCouldn't open [%s] for reading\n", RED, pFileName);
    }

    return iResult;
}
