#include <stdio.h>
#include <math.h>
#include <string.h>

#include <algorithm>

#include "types.h"
#include "strutils.h"
#include "AncestorNode.h"
#include "RWAncGraph.h"

#define DEF_DEPTH 4

//----------------------------------------------------------------------------
// showProgenitors
//
void showProgenitors(RWAncGraph *pAG) {
    const idset sProg = pAG->getProgenitors();
    idset::const_iterator it;
    for (it = sProg.begin(); it != sProg.end(); ++it) {
        printf(" %ld", *it);
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// showSelected
//
void showSelected(RWAncGraph *pAG) {
    const idset sSel = pAG->getSelected();
    idset_cit it;
    for (it = sSel.begin(); it != sSel.end(); ++it) {
        printf(" %ld", *it);
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// showLeaves
//
void showLeaves(RWAncGraph *pAG) {
    const idset sLvs = pAG->getLeaves();
    idset_cit it;
    for (it = sLvs.begin(); it != sLvs.end(); ++it) {
        printf(" %ld", *it);
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// showRoots
//
void showRoots(RWAncGraph *pAG) {
    const idset sRoots = pAG->getRoots();
    idset_cit it;
    for (it = sRoots.begin(); it != sRoots.end(); ++it) {
        printf(" %ld", *it);
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// showNumbers
//
void showNumbers(RWAncGraph *pAG) {
    printf("Total       %zd\n", pAG->getMap().size());
    printf("Progenitors %zd\n", pAG->getProgenitors().size());
    printf("Selected    %zd\n", pAG->getSelected().size());
    printf("Roots       %zd\n", pAG->getRoots().size());
    printf("Leaves      %zd\n", pAG->getLeaves().size());
}

//----------------------------------------------------------------------------
// showNodes
//
void showNodes(RWAncGraph *pAG, idset &sIDs) {
    for (idset_cit it=sIDs.begin(); it != sIDs.end(); ++it) {
        AncestorNode *pAN =  pAG->findAncestorNode(*it);
        if (pAN != NULL) {
            printf("Node       %ld\n", pAN->m_iID);
            printf("Parents   [%ld,%ld]\n", pAN->getMom(), pAN->getDad());
            printf("Gender      %d\n", pAN->m_iGender);
            printf("Children   ");
            idset_cit it1;
            for (it1 =  pAN->m_sChildren.begin(); it1 !=  pAN->m_sChildren.end(); ++it1) {
                printf("%ld  ", *it1);
            }
            printf("\n");
        } else {
            printf("No node with ID [%ld] found\n", *it);
        }
    }
}

//----------------------------------------------------------------------------
// showMatriline
//
void showMatriline(RWAncGraph *pAG, idtype iID) {
    AncestorNode *pAN = pAG->findAncestorNode(iID);
    while (pAN != NULL) {
        if (pAN->m_iID > 0) {
            printf("Node  %9ld [%9ld,%9ld]\n", pAN->m_iID, pAN->getMom(), pAN->getDad());
            pAN = pAG->findAncestorNode(pAN->getMom());
        } else {
            pAN = NULL;
        }
    }
}

//----------------------------------------------------------------------------
// showPatriline
//
void showPatriline(RWAncGraph *pAG, idtype iID) {
    AncestorNode *pAN = pAG->findAncestorNode(iID);
    while (pAN != NULL) {
        if (pAN->m_iID > 0) {
            printf("Node  %9ld [%9ld,%9ld]\n", pAN->m_iID, pAN->getMom(), pAN->getDad());
            pAN = pAG->findAncestorNode(pAN->getDad());
        } else {
            pAN = NULL;
        }
    }
}

//----------------------------------------------------------------------------
// checkGenders
//
void checkGenders(RWAncGraph *pAG) {
    idset sMothers;
    idset sFathers;
    const ancnodelist& mIndex = pAG->getMap();
    ancnodelist::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        idtype iMom = it->second->getMom();
        if (iMom != 0) {
            sMothers.insert(iMom);
        }
        idtype iDad = it->second->getDad();
        if (iDad != 0) {
            sFathers.insert(iDad);
        }
    }
    std::vector<idtype> v(sMothers.begin(), sMothers.end());
    std::vector<idtype>::iterator it2;
    it2=std::set_intersection(sMothers.begin(), sMothers.end(), sFathers.begin(), sFathers.end(), v.begin());
    v.resize(it2-v.begin());
    if (v.size() > 0) {
        bool bOne = (v.size() == 1);
        printf("Have %zd mother%s who %s father%s\n", v.size(), bOne?"":"s", bOne?"is a":"are", bOne?"":"s");
        uint iS = 10;
        if (v.size() < 10) {
            iS = (int)v.size();
        }
        for (uint i = 0; i < iS; ++i) {
            printf("  %ld", v[i]);
        }
        if (iS < v.size()) {
            printf("  ...");
        }
        printf("\n");
    } else {
        printf("Genders seem to be separated\n");
    }
    
}

// type needed by checkChildren()
typedef struct relinfo {
    relinfo() : bNoMom(false), bNoDad(false), bBadMom(false), bBadDad(false) {};
    bool bNoMom;
    bool bNoDad;
    
    bool bBadMom;
    bool bBadDad;
    
    std::vector<idtype> vBadChildren;
} relinfo;

//----------------------------------------------------------------------------
// checkChildren
//
void checkChildren(RWAncGraph *pAG) {


    std::map<idtype, relinfo> mRelInfo;

    const ancnodelist& mIndex = pAG->getMap();
    ancnodelist::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        bool bIsRoot = (pAG->getRoots().find(it->first) != pAG->getRoots().end());
        AncestorNode *pANMom = pAG->findAncestorNode(it->second->getMom());
        if (pANMom != NULL) {
            idset_cit itc = pANMom->m_sChildren.find(it->first);
            if (itc == pANMom->m_sChildren.end()) {
                mRelInfo[it->first].bBadMom = true;
            }
        } else {
            if (!bIsRoot) {
                mRelInfo[it->first].bNoMom = true;
            }
        }
        AncestorNode *pANDad = pAG->findAncestorNode(it->second->getDad());
        if (pANDad != NULL) {
            idset_cit itc = pANDad->m_sChildren.find(it->first);
            if (itc == pANDad->m_sChildren.end()) {
                mRelInfo[it->first].bBadDad = true;
            }
        } else {
            if (!bIsRoot) {
                mRelInfo[it->first].bNoDad = true;
            }
        }

        for(idset_cit itc1 = it->second->m_sChildren.begin(); itc1 != it->second->m_sChildren.end(); ++itc1) {
            AncestorNode *pANC = pAG->findAncestorNode(*itc1);
            if (pANC != NULL) {
                if ((pANC->getMom() != it->first) && (pANC->getDad() != it->first)) {
                    mRelInfo[it->first].vBadChildren.push_back(*itc1);
                }
            } else {
                mRelInfo[it->first].vBadChildren.push_back(*itc1);
            }
        }
    }
    
    if (mRelInfo.size() == 0) {
        printf("parent-child relationships OK\n");
    } else {
        printf("Found %zd problematic ID%s:\n", mRelInfo.size(), (mRelInfo.size()!=1)?"s":"");
        std::map<idtype, relinfo>::const_iterator itr;
        for (itr = mRelInfo.begin(); itr != mRelInfo.end(); ++itr) {
            printf("  ID %ld: ", itr->first);
            bool bHasOne = false;
            if (itr->second.bNoMom) {
                printf("%s no mom", bHasOne?",":"");
                bHasOne = true;
            }
            if (itr->second.bBadMom) {
                printf("%s bad mom", bHasOne?",":"");
                bHasOne = true;
            }
            if (itr->second.bNoDad) {
                printf("%s no dad", bHasOne?",":"");
                bHasOne = true;
            }
            if (itr->second.bBadDad) {
                printf("%s bad dad", bHasOne?",":"");
                bHasOne = true;
            }
            if (itr->second.vBadChildren.size() > 0) {
                printf("%s bad children: ", bHasOne?",":"");
                for (uint i = 0; i < itr->second.vBadChildren.size(); ++i) {
                    printf(" %ld", itr->second.vBadChildren[i]);
                }
                bHasOne = true;
            }
            printf("\n");
        }
    }
}

//----------------------------------------------------------------------------
// avergaes
//
void averages(RWAncGraph *pAG) {

    float fMalesForFemales = 0;
    float fFemalesForMales = 0;
    float fChildrenForMother = 0;
    float fChildrenForFather = 0;
    uint iMaxMfF = 0;
    uint iMaxFfM = 0;
    uint iMaxCfM = 0;
    uint iMaxCfF = 0;
    int iCM = 0;
    int iCD = 0;

    const ancnodelist& mIndex = pAG->getMap();
    ancnodelist::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        AncestorNode *pAN = it->second;
        if (pAN->m_iGender == 0) {
            iCM++;
        } else {
            iCD++;
        }

        idset sPartners;
        for(idset_cit itc1 = pAN->m_sChildren.begin(); itc1 != pAN->m_sChildren.end(); ++itc1) {
            AncestorNode *pAnc = pAG->findAncestorNode(*itc1);
            if (pAN->m_iGender == 0) {
                sPartners.insert(pAnc->getDad());
            } else {
                sPartners.insert(pAnc->getMom());
            }
        }
   
        if (pAN->m_iGender == 0) {
            fMalesForFemales   += (float)sPartners.size();
            fChildrenForMother += (float)pAN->m_sChildren.size();

            if (sPartners.size() > iMaxMfF) {
                iMaxMfF = (uint)sPartners.size();
            }
            if (pAN->m_sChildren.size() > iMaxCfF) {
                iMaxCfF = (uint)pAN->m_sChildren.size();
            }
            
        } else {
            fFemalesForMales   += (float)sPartners.size();
            fChildrenForFather += (float)pAN->m_sChildren.size();

            if (sPartners.size() > iMaxFfM) {
                iMaxFfM = (uint)sPartners.size();
            }
            if (pAN->m_sChildren.size() > iMaxCfM) {
                iMaxCfM = (uint)pAN->m_sChildren.size();
            }

        }
    }
    fMalesForFemales /= (float)iCM;
    fFemalesForMales /= (float)iCD;
    fChildrenForMother /= (float)iCM;
    fChildrenForFather /= (float)iCD;

    printf("%d females, %f males per female (max %d);  %f children per female max (%d)\n", iCM, fMalesForFemales, iMaxMfF, fChildrenForMother, iMaxCfF);
    printf("%d males,   %f females per males (max %d); %f children per male (max %d)\n",   iCD, fFemalesForMales, iMaxFfM, fChildrenForFather, iMaxCfM);
 
}




//----------------------------------------------------------------------------
// drawTree
//
void drawTree(RWAncGraph *pAG, AncestorNode *pAN, const char *pPrefix, bool bHasNext, int iMaxDepth, bool bWide) {
    if (bWide) {
        printf("%s|   \n", pPrefix);
    }
    printf("%s+---%ld\n", pPrefix, pAN->m_iID);
    uint iNumC = (uint)pAN->m_sChildren.size();
    char sPrefix2[1024];
    if (iMaxDepth > 0) {
        uint iC = 0;
        idset_cit it;
        for (it =  pAN->m_sChildren.begin(); it !=  pAN->m_sChildren.end(); ++it) {
            if (bHasNext) {
                sprintf(sPrefix2, "%s|   ", pPrefix);
            } else {
                sprintf(sPrefix2, "%s    ", pPrefix);
            }
            bool bHasNext2 = (iC < iNumC-1);

            AncestorNode *pANC = pAG->findAncestorNode(*it);
	    if (pANC != NULL) {
                drawTree(pAG, pANC, sPrefix2, bHasNext2, iMaxDepth-1, bWide);
	    } else {
                if (bWide) {
                    printf("%s|   \n", sPrefix2);
                }
                printf("%s+---(%ld)\n", sPrefix2, *it);
	    }   
            iC++;
        }
    }
}

//----------------------------------------------------------------------------
// listNodes
//
void listNodes(RWAncGraph *pAG) {
    const ancnodelist& mIndex = pAG->getMap();
    ancnodelist::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        printf("Node       %ld\n", it->second->m_iID);
        printf("Parents   [%ld,%ld]\n", it->second->getMom(), it->second->getDad());
        printf("Children   ");
        idset_cit it2;
        for (it2 =  it->second->m_sChildren.begin(); it2 !=  it->second->m_sChildren.end(); ++it2) {
            printf("%ld  ", *it2);
        }
        printf("\n");
        printf("-----\n");
    }
}
//----------------------------------------------------------------------------
// getMaxChildren
//
void getMaxChildren(RWAncGraph *pAG) {
    const ancnodelist& mIndex = pAG->getMap();
    uint   iMax = 0;
    idtype iMaxNode = 0;
    double dSum = 0.0;
    ancnodelist::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        dSum += (double)(it->second->m_sChildren.size());
        if (it->second->m_sChildren.size() > iMax) {
            iMax = (uint)it->second->m_sChildren.size();
            iMaxNode = it->first;
        }
    }
    dSum /= (double)mIndex.size();
    printf("Maximum number of children  found:\n  Node %ld has %u children\n  Average number of children %3.2f\n", iMaxNode, iMax, dSum);
}

//----------------------------------------------------------------------------
// showTree
//
void showTree(RWAncGraph *pAG, idtype iID, int iDepth, bool bWide) {
    AncestorNode *pAN = pAG->findAncestorNode(iID);
    if (pAN != NULL) {
        char sPrefix[1024];
        strcpy(sPrefix, "  ");
        drawTree(pAG, pAN, sPrefix, false, iDepth, bWide);
    } else {
        printf("No node with ID [%ld] found\n", iID);
    }
}


//----------------------------------------------------------------------------
// findTreeR
//
void findTreeR(RWAncGraph *pAG, AncestorNode *pAN, idtype iIDFind, int iCurDepth, int *piDepth, bool *pbSearching) {

        if (pAN->m_iID == iIDFind) {
            *pbSearching = false;
            *piDepth = iCurDepth;
        } else {
            idset_cit it;
           
            int iDepthMin = 0;
            for (it =  pAN->m_sChildren.begin();  (it !=  pAN->m_sChildren.end()); ++it) {

                AncestorNode *pANC = pAG->findAncestorNode(*it);
                if (pANC != NULL) {
                    int iDepthCur = 0;
                    findTreeR(pAG, pANC, iIDFind, iCurDepth+1, &iDepthCur, pbSearching);
                    if (iDepthCur > 0) {
                        if ((iDepthMin == 0) || (iDepthCur < iDepthMin)) {
                            iDepthMin = iDepthCur;
                        }                                
                    }
                }
                
	    }
            if (iDepthMin > 0) {
                *piDepth += iDepthMin;
            }
        }

}

//----------------------------------------------------------------------------
// findTree
//
void findTree(RWAncGraph *pAG, idtype iIDStart, idtype iIDFind) {
    AncestorNode *pAN = pAG->findAncestorNode(iIDStart);
    if (pAN != NULL) {
        int iDepth = 0;
        bool bSearching = true;
        findTreeR(pAG, pAN, iIDFind, 0, &iDepth, &bSearching);
        if (!bSearching) {
            printf("found %ld at %d generations after %ld\n", iIDFind, iDepth, iIDStart);
        } else {
            printf("%ld is not a descendant of %ld\n", iIDFind, iIDStart);
        }
    } else {
        printf("No node with ID [%ld] found\n", iIDStart);
    }
}


//----------------------------------------------------------------------------
// prune
//
void prune(RWAncGraph *pAG) {
    int iResult = pAG->prune(true);
    printf("Result: %d\n", iResult);
}


//----------------------------------------------------------------------------
// showCommands
//
void showCommands() {
    printf("Commands are:\n");
    printf("  roots (r)\n");
    printf("    show ids of roots\n");
    printf("  progenitors (p)\n");
    printf("    show ids of progenitors\n");
    printf("  leaves (l)\n");
    printf("    show ids of leaves (nodes without children; attention: there might be a lot!)\n");
    printf("  selected (s)\n");
    printf("    show ids of selected (attention: there might be a lot!)\n");
    printf("  num (n)\n");
    printf("    show number of progenitors, selected, and total number of nodes\n");
    printf("  genders (g)\n");
    printf("    checks gender separation\n");
    printf("  children (c)\n");
    printf("    checks child relation consistence (x's mother/father has x as child)\n");
    printf("  maxchildren (M)\n");
    printf("    maximum number of children by a node\n");
    printf("  display <id> (d)\n");
    printf("    show details of node with given id\n");
    printf("  matriline <id> (m)\n");
    printf("    show maternal line of ancestors of node <id>\n");
    printf("  patriline <id> (f)\n");
    printf("    show paternal line of ancestors of node <id>\n");
    printf("  tree <id> <depth> (t)\n");
    printf("    show tree of descendats of node <id> down to given depth (default %d)\n", DEF_DEPTH);
    printf("  find <idstart> <idfind> (F)\n");
    printf("    find <idfind> in tree of descendats of node <idstart>\n");
    printf("  prune (x)\n");
    printf("    prune\n");
    printf("  list (L)\n");
    printf("    list all nodes\n");
    printf("  averages (a)\n");
    printf("    show some averages\n");
    printf("  quit (q)\n");
    printf("    exit application\n");
}

//----------------------------------------------------------------------------
// action
//
bool isShortCut(const char *pCommand, const char c) {
    return (*pCommand == c) && ((pCommand[1] == '\0') || (pCommand[1] == '\n') || (pCommand[1] == ' '));
}

//----------------------------------------------------------------------------
// action
//
bool action(RWAncGraph *pAG, char *pCommand) {
    bool bGoOn = true;
    if (pCommand != NULL) {
        if ((strstr(pCommand, "progenitors") == pCommand) || isShortCut(pCommand, 'p')){
            showProgenitors(pAG);
        } else if ((strstr(pCommand, "selected") == pCommand) || isShortCut(pCommand, 's')) {
            showSelected(pAG);
        } else if ((strstr(pCommand, "roots") == pCommand) || isShortCut(pCommand, 'r')) {
            showRoots(pAG);
        } else if ((strstr(pCommand, "leaves") == pCommand) || isShortCut(pCommand, 'l')) {
            showLeaves(pAG);
        } else if ((strstr(pCommand, "num") == pCommand) || isShortCut(pCommand, 'n')) {
            showNumbers(pAG);
        } else if ((strstr(pCommand, "list") == pCommand) || isShortCut(pCommand, 'L')) {
            listNodes(pAG);
        } else if ((strstr(pCommand, "genders") == pCommand) || isShortCut(pCommand, 'g')) {
            checkGenders(pAG);
        } else if ((strstr(pCommand, "children") == pCommand) || isShortCut(pCommand, 'c')) {
            checkChildren(pAG);
        } else if ((strstr(pCommand, "maxchildren") == pCommand) || isShortCut(pCommand, 'M')) {
            getMaxChildren(pAG);
        } else if ((strstr(pCommand, "averages") == pCommand) || isShortCut(pCommand, 'a')) {
            averages(pAG);
        } else if ((strstr(pCommand, "display") == pCommand)  || isShortCut(pCommand, 'd')){
            pCommand = strpbrk(pCommand, " \t\n:");
            if (pCommand != NULL) {
                idset sIDs;
                int iResult = 0;
                char *p0 = strtok(pCommand, " .;\t\n");
                while ((iResult == 0) &&(p0 != NULL)) {
                    int iID;
                    if (strToNum(trim(p0), &iID)) {
                        sIDs.insert(iID);
                        p0 = strtok(NULL, " .;\t\n");
                    } else {
                        printf("expected number [%s]\n", pCommand);
                        iResult = -1;
                    }
                }
                
                if (iResult == 0) {
                    showNodes(pAG, sIDs);
                }
            } else {
                printf("expected number [%s]\n", pCommand);
            }
        } else if ((strstr(pCommand, "matriline") == pCommand) || isShortCut(pCommand, 'm')) {
            pCommand = strpbrk(pCommand, " \t\n:");
            int iID=-1;
            if (strToNum(trim(pCommand), &iID)) {
                showMatriline(pAG, iID);
            } else {
                printf("expected number [%s]\n", pCommand);
            }
        } else if ((strstr(pCommand, "patriline") == pCommand) || isShortCut(pCommand, 'f')) {
            pCommand = strpbrk(pCommand, " \t\n:");
            int iID=-1;
            if (strToNum(trim(pCommand), &iID)) {
                showPatriline(pAG, iID);
            } else {
                printf("expected number [%s]\n", pCommand);
            }
        } else if ((strstr(pCommand, "tree") == pCommand) || isShortCut(pCommand, 't')){
            pCommand = strpbrk(pCommand, " \t:");
            int iID=-1;
            int iDepth=DEF_DEPTH;
            bool bWide = false;
            int iResult = -1;
            char *p0 = strtok(pCommand, " .;\t\n");
            if (p0 != NULL) {
                if (strToNum(trim(p0), &iID)) {
                    iResult = 0;
                    p0 = strtok(NULL, " .;\t\n");
                    if (p0 != NULL) {
                        if (strToNum(trim(p0), &iDepth)) {
                            iResult = 0;
                            p0 = strtok(NULL, " .;\t\n");
                            if (p0 != NULL) {
                                if (*p0 == 'w') {
                                    bWide = true;
                                }
                            }
                        } else {
                            printf("expected number for Depth [%s]\n", pCommand);
                            iResult = -1;
                        }
                    } else {
                        // iDepth default
                    }
                } else {
                    printf("expected number for ID [%s]\n", pCommand);
                }
                if (iResult == 0) {
                    showTree(pAG, iID, iDepth, bWide);
                }
            } else {
                printf("expected number (ID)\n");
            }
        } else if ((strstr(pCommand, "find") == pCommand) || isShortCut(pCommand, 'F')){
            pCommand = strpbrk(pCommand, " \t:");
            int iIDStart = -1;
            int iIDFind  = -1;
            int iResult = -1;
            char *p0 = strtok(pCommand, " .;\t\n");
            if (p0 != NULL) {
                if (strToNum(trim(p0), &iIDStart)) {
                    iResult = 0;
                    p0 = strtok(NULL, " .;\t\n");
                    if (p0 != NULL) {
                        if (strToNum(trim(p0), &iIDFind)) {
                            iResult = 0;
                        } else {
                            printf("expected number for ID to find [%s]\n", pCommand);
                            iResult = -1;
                        }
                    } else {
                        iResult = -1;
                        printf("expected number for ID to find [%s]\n", pCommand);
                    }
                } else {
                    printf("expected number for ID to start from  [%s]\n", pCommand);
                }
                if (iResult == 0) {
                    findTree(pAG, iIDStart, iIDFind);
                }
            } else {
                printf("expected number (ID)\n");
            }
        } else if ((strstr(pCommand, "prune") == pCommand) || isShortCut(pCommand, 'x')) {
            prune(pAG);
        } else if ((strstr(pCommand, "refresh") == pCommand) || isShortCut(pCommand, 'R')) {
            pAG->collectRootsLeaves();
        } else if ((strstr(pCommand, "quit") == pCommand) || isShortCut(pCommand, 'q')) {
            bGoOn = false;
        } else if ((strstr(pCommand, "help") == pCommand) || isShortCut(pCommand, 'h') || (*pCommand == '?')){
            showCommands();
        } else if (*(trim(pCommand)) == '\0') {
            /// do nothing
        } else {
            printf("???\n");
        }
    }
    return bGoOn;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sAGFile[1024];
    char sCommand[1024];
    *sCommand = '\0';

    if (iArgC > 1) {
        strcpy(sAGFile, apArgV[1]);
        if (iArgC > 2) {
            strcpy(sCommand, apArgV[2]);
        }
        RWAncGraph *pAG = new RWAncGraph();
        iResult = pAG->loadBin(sAGFile);
        if (iResult == 0) {
            pAG->collectRootsLeaves();
            if (*sCommand != '\0') {
                action(pAG, sCommand);
            } else {
            
                bool bGoOn = true;
                printf("Enter a command at the prompt\n");
                while (bGoOn) {
                    printf("> ");
                    char sCommand[256];
                    char *p = fgets(sCommand, 256, stdin);
                    if (p != NULL) {
                        bGoOn = action(pAG, p);
                    }
                }
            }
        } else {
            iResult = -1;
            printf("Couldn't read AncestorGraph from [%s]\n", sAGFile);
        }
    } else {
        iResult = -1;
        printf("%s - inspect ancestor graph\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <ancestorgraphfile>\n", apArgV[0]);
        printf("      starts an interactive loop\n");
        printf("      with various commands\n");
        printf("or\n");
        printf("  %s <ancestorgraphfile> \"<command>\"\n", apArgV[0]);
        printf("      will execute the given command\n");
        printf("\n");
        showCommands();
    }
    return iResult;
}
