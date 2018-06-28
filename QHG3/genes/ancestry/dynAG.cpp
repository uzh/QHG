#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <algorithm>

#include "types.h"
#include "strutils.h"
#include "AncestorNode.h"
#include "RWAncGraph.h"
#include "AGWindow.h"

#define DEF_OSIZE 1000000
#define DEF_DEPTH 5

//----------------------------------------------------------------------------
// isShortCut
//
bool isShortCut(const char *pCommand, const char c) {
    return (*pCommand == c) && ((pCommand[1] == '\0') || (pCommand[1] == '\n') || (pCommand[1] == ' '));
}

//----------------------------------------------------------------------------
// showCommands
//
void showCommands() {
    printf("Commands are:\n");
    /*
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
    */
    printf("  children (c)\n");
    printf("    checks child relation consistence (x's mother/father has x as child)\n");
    /*
    printf("  maxchildren (M)\n");
    printf("    maximum number of children by a node\n");
    */
    printf("  display <id> (d)\n");
    printf("    show details of node with given id\n");
    printf("  matriline <id> (m)\n");
    printf("    show maternal line of ancestors of node <id>\n");
    printf("  patriline <id> (f)\n");
    printf("    show paternal line of ancestors of node <id>\n");
    printf("  tree <id> <depth> (t)\n");
    printf("    show tree of descendats of node <id> down to given depth (default %d)\n", DEF_DEPTH);
    printf("  ancestors <id> <depth> (a)\n");
    printf("    show tree of descendats of node <id> down to given depth (default %d)\n", DEF_DEPTH);
    /*
    printf("  prune (x)\n");
    printf("    prune\n");
    */
    printf("  list (L)\n");
    printf("    list nodes in loaded block\n");
    printf("  help (h)\n");
    printf("    show commands\n");
    printf("  quit (q)\n");
    printf("    exit application\n");
}

//----------------------------------------------------------------------------
// showNodes
//
void showNodes(AGWindow *pAGW, intset &sIDs) {
    for (intset::const_iterator it=sIDs.begin(); it != sIDs.end(); ++it) {
        AncestorNode *pAN = pAGW->getNode(*it);
        if (pAN != NULL) {
            printf("Node       %d\n", pAN->m_iID);
            printf("Parents   [%d,%d]\n", pAN->getMom(), pAN->getDad());
            printf("Gender      %d\n", pAN->m_iGender);
            printf("Children   ");
            intset::const_iterator it1;
            for (it1 =  pAN->m_sChildren.begin(); it1 !=  pAN->m_sChildren.end(); ++it1) {
                printf("%d  ", *it1);
            }
            printf("\n");
        } else {
            printf("No node with ID [%d] found\n", *it);
        }
    }
}

//----------------------------------------------------------------------------
// showMatriline
//
void showMatriline(AGWindow *pAGW, int iID) {
    AncestorNode *pAN = pAGW->getNode(iID);
    while (pAN != NULL) {
        if (pAN->m_iID > 0) {
            printf("Node  %9d [%9d,%9d]\n", pAN->m_iID, pAN->getMom(), pAN->getDad());
            pAN = pAGW->getNode(pAN->getMom());
        } else {
            pAN = NULL;
        }
    }
}

//----------------------------------------------------------------------------
// showPatriline
//
void showPatriline(AGWindow *pAGW, int iID) {
    AncestorNode *pAN = pAGW->getNode(iID);
    while (pAN != NULL) {
        if (pAN->m_iID > 0) {
            printf("Node  %9d [%9d,%9d]\n", pAN->m_iID, pAN->getMom(), pAN->getDad());
            pAN = pAGW->getNode(pAN->getDad());
        } else {
            pAN = NULL;
        }
    }
}


//----------------------------------------------------------------------------
// drawTree
//
void drawTree(AGWindow *pAGW, AncestorNode *pAN, const char *pPrefix, bool bHasNext, int iMaxDepth, bool bWide) {
    if (bWide) {
        printf("%s|   \n", pPrefix);
    }
    printf("%s+---%d\n", pPrefix, pAN->m_iID);
    uint iNumC = pAN->m_sChildren.size();
    char sPrefix2[1024];
    if (iMaxDepth > 0) {
        uint iC = 0;
        // remember set of children: pAN may be deleted when a descendant requires a new block to be loaded
        intset sCur(pAN->m_sChildren.begin(),  pAN->m_sChildren.end());
        intset::const_iterator it;
        for (it =  sCur.begin(); it !=  sCur.end(); ++it) {
            if (bHasNext) {
                sprintf(sPrefix2, "%s|   ", pPrefix);
            } else {
                sprintf(sPrefix2, "%s    ", pPrefix);
            }
            bool bHasNext2 = (iC < iNumC-1);

            AncestorNode *pANC = pAGW->getNode(*it);
	    if (pANC != NULL) {
                drawTree(pAGW, pANC, sPrefix2, bHasNext2, iMaxDepth-1, bWide);
	    } else {
                if (bWide) {
                    printf("%s|   \n", sPrefix2);
                }
                printf("%s+---(%d)\n", sPrefix2, *it);
	    }   
            iC++;
        }
    }
}


//----------------------------------------------------------------------------
// showTree
//
void showTree(AGWindow *pAGW, int iID, int iDepth, bool bWide) {
    AncestorNode *pAN = pAGW->getNode(iID);
    if (pAN != NULL) {
        char sPrefix[1024];
        strcpy(sPrefix, "  ");
        drawTree(pAGW, pAN, sPrefix, false, iDepth, bWide);
    } else {
        printf("No node with ID [%d] found\n", iID);
    }
}

//----------------------------------------------------------------------------
// drawAncestors
//
void drawAncestors(AGWindow *pAGW, AncestorNode *pAN, const char *pPrefix, bool bHasNext, int iMaxDepth, bool bWide) {
    if (bWide) {
        printf("%s|   \n", pPrefix);
    }
    printf("%s+---%d\n", pPrefix, pAN->m_iID);
 
    char sPrefix2[1024];
    if (iMaxDepth > 0) {
 
        // remember set of parents: pAN may be deleted when a ancestors requires a new block to be loaded
        int aiParents[2];
        memcpy(aiParents, pAN->m_aParents, 2 *sizeof(int));
       
        for (int i = 0; i < 2; ++i) {
            if (bHasNext) {
                sprintf(sPrefix2, "%s|   ", pPrefix);
            } else {
                sprintf(sPrefix2, "%s    ", pPrefix);
            }
            bool bHasNext2 = (i == 0);

            AncestorNode *pANC = pAGW->getNode(aiParents[i]);
	    if (pANC != NULL) {
                drawAncestors(pAGW, pANC, sPrefix2, bHasNext2, iMaxDepth-1, bWide);
	    } else {
                if (bWide) {
                    printf("%s|   \n", sPrefix2);
                }
                printf("%s+---(%d)\n", sPrefix2, aiParents[i]);
	    }   
        }
    }
}


//----------------------------------------------------------------------------
// showAncestors
//
void showAncestors(AGWindow *pAGW, int iID, int iDepth, bool bWide) {
    AncestorNode *pAN = pAGW->getNode(iID);
    if (pAN != NULL) {
        char sPrefix[1024];
        strcpy(sPrefix, "  ");
        drawAncestors(pAGW, pAN, sPrefix, false, iDepth, bWide);
    } else {
        printf("No node with ID [%d] found\n", iID);
    }
}

//----------------------------------------------------------------------------
// listNodes
//
void listNodes(AGWindow *pAGW) {
    const std::map<int, AncestorNode *>& mIndex = pAGW->getMap();
    std::map<int, AncestorNode *>::const_iterator it;
    for (it = mIndex.begin(); it != mIndex.end(); ++it) {
        printf("Node       %d\n", it->second->m_iID);
        printf("Parents   [%d,%d]\n", it->second->getMom(), it->second->getDad());
        printf("Children   ");
        intset::const_iterator it2;
        for (it2 =  it->second->m_sChildren.begin(); it2 !=  it->second->m_sChildren.end(); ++it2) {
            printf("%d  ", *it2);
        }
        printf("\n");
        printf("-----\n");
    }
}


// type needed by checkChildren()
typedef struct relinfo {
    relinfo() : bNoMom(false), bNoDad(false), bBadMom(false), bBadDad(false) {};
    bool bNoMom;
    bool bNoDad;
    
    bool bBadMom;
    bool bBadDad;
    
    std::vector<int> vBadChildren;
} relinfo;

//----------------------------------------------------------------------------
// checkChildren
//
void checkChildren(AGWindow *pAGW) {


    std::map<int, relinfo> mRelInfo;

    
    AncestorNode *pAnc = pAGW->getFirst();
    while (pAnc != NULL) {
        int iID = pAnc->m_iID;
        int iMomID = pAnc->getMom();
        int iDadID = pAnc->getDad();
        intset sC(pAnc->m_sChildren.begin(), pAnc->m_sChildren.end());
        bool bIsRoot = (pAGW->getRoots().find(iID) != pAGW->getRoots().end());
        AncestorNode *pANMom = pAGW->getNode(iMomID);
        if (pANMom != NULL) {
            intset::const_iterator itc = pANMom->m_sChildren.find(iID);
            if (itc == pANMom->m_sChildren.end()) {
                mRelInfo[iID].bBadMom = true;
            }
        } else {
            if (!bIsRoot) {
                mRelInfo[iID].bNoMom = true;
            }
        }
        AncestorNode *pANDad = pAGW->getNode(iDadID);
        if (pANDad != NULL) {
            intset::const_iterator itc = pANDad->m_sChildren.find(iID);
            if (itc == pANDad->m_sChildren.end()) {
                mRelInfo[iID].bBadDad = true;
            }
        } else {
            if (!bIsRoot) {
                mRelInfo[iID].bNoDad = true;
            }
        }

        for(intset::const_iterator itc1 = sC.begin(); itc1 != sC.end(); ++itc1) {
            AncestorNode *pANC = pAGW->getNode(*itc1);
            if (pANC != NULL) {
                if ((pANC->getMom() != iID) && (pANC->getDad() != iID)) {
                    mRelInfo[iID].vBadChildren.push_back(*itc1);
                }
            } else {
                mRelInfo[iID].vBadChildren.push_back(*itc1);
            }
        }
        pAnc = pAGW->getNext();
    }
    
    if (mRelInfo.size() == 0) {
        printf("parent-child relationships OK\n");
    } else {
        printf("Found %zd problematic ID%s:\n", mRelInfo.size(), (mRelInfo.size()!=1)?"s":"");
        std::map<int, relinfo>::const_iterator itr;
        for (itr = mRelInfo.begin(); itr != mRelInfo.end(); ++itr) {
            printf("  ID %d: ", itr->first);
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
                    printf(" %d", itr->second.vBadChildren[i]);
                }
                bHasOne = true;
            }
            printf("\n");
        }
    }    
}

//----------------------------------------------------------------------------
// getStats
//
void getStats(AGWindow *pAGW) {
    uint iNumNodes     = 0;
    uint iNumMales     = 0;
    uint iNumFemales   = 0;
    uint iNumBadGender = 0;
    uint iNumChildless = 0;
    uint iMaxChildren  = 0;
    
    
    AncestorNode *pAnc = pAGW->getFirst();
    while (pAnc != NULL) {
        iNumNodes++;
        if (pAnc->m_iGender == 0) {
            iNumFemales++;
        } else if (pAnc->m_iGender == 1) {
            iNumMales++;
        } else {
            iNumBadGender++;
        }
        if (pAnc->m_sChildren.size() > 0) {
            if (pAnc->m_sChildren.size() > iMaxChildren) {
                iMaxChildren = pAnc->m_sChildren.size();
            }
        } else {
            iNumChildless++;
        }
        pAnc = pAGW->getNext();
    }

    printf("stats\n");
    printf("Num nodes      %d\n", iNumNodes);
    printf("Num females    %d\n", iNumFemales);
    printf("Num males      %d\n", iNumMales);
    printf("Num unkown sex %d\n", iNumBadGender);
    printf("Num childless  %d\n", iNumChildless);
    printf("Max children   %d\n", iMaxChildren);
}

//----------------------------------------------------------------------------
// action
//
bool action(AGWindow *pAGW,char *pCommand) {
    bool bGoOn = true;
    if (pCommand != NULL) {
        pCommand = trim(pCommand);
        /*
        if ((strstr(pCommand, "progenitors") == pCommand) || isShortCut(pCommand, 'p')){
            showProgenitors(pAG);
        } else if ((strstr(pCommand, "selected") == pCommand) || isShortCut(pCommand, 'S')) {
            showSelected(pAG);
        } else if ((strstr(pCommand, "roots") == pCommand) || isShortCut(pCommand, 'r')) {
            showRoots(pAG);
        } else if ((strstr(pCommand, "leaves") == pCommand) || isShortCut(pCommand, 'l')) {
            showLeaves(pAG);
        } else if ((strstr(pCommand, "num") == pCommand) || isShortCut(pCommand, 'n')) {
            showNumbers(pAG);
        
        } else*/ if ((strstr(pCommand, "list") == pCommand) || isShortCut(pCommand, 'L')) {
            listNodes(pAGW);
            /*
        } else if ((strstr(pCommand, "genders") == pCommand) || isShortCut(pCommand, 'g')) {
            checkGenders(pAG);
            */
        } else if ((strstr(pCommand, "children") == pCommand) || isShortCut(pCommand, 'c')) {
            checkChildren(pAGW);
        } else if ((strstr(pCommand, "stats") == pCommand) || isShortCut(pCommand, 's')) {
            getStats(pAGW);
            /*
        } else if ((strstr(pCommand, "maxchildren") == pCommand) || isShortCut(pCommand, 'M')) {
            getMaxChildren(pAG);
            */
            } else  if ((strstr(pCommand, "display") == pCommand)  || isShortCut(pCommand, 'd')){
            pCommand = strpbrk(pCommand, " \t\n:");
            if (pCommand != NULL) {
                intset sIDs;
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
                    showNodes(pAGW, sIDs);
                }
            } else {
                printf("expected number [%s]\n", pCommand);
            }
            
        } else if ((strstr(pCommand, "matriline") == pCommand) || isShortCut(pCommand, 'm')) {
            pCommand = strpbrk(pCommand, " \t:");
            int iID=-1;
            if (strToNum(trim(pCommand), &iID)) {
                showMatriline(pAGW, iID);
            } else {
                printf("expected number [%s]\n", pCommand);
            }
            
        } else if ((strstr(pCommand, "patriline") == pCommand) || isShortCut(pCommand, 'f')) {
            pCommand = strpbrk(pCommand, " \t:");
            int iID=-1;
            if (strToNum(trim(pCommand), &iID)) {
                showPatriline(pAGW, iID);
            } else {
                printf("expected number [%s]\n", pCommand);
            }
            
        } else if ((strstr(pCommand, "tree") == pCommand) || isShortCut(pCommand, 't')){
            pCommand = strpbrk(pCommand, " \t:");
            int iID=-1;
            int iDepth=DEF_DEPTH;
            bool bWide = false;
            int iResult = -1;
            char *p0 = strtok(pCommand, " .;\t");
            if (p0 != NULL) {
                if (strToNum(trim(p0), &iID)) {
                    iResult = 0;
                    p0 = strtok(NULL, " .;\t");
                    if (p0 != NULL) {
                        if (strToNum(trim(p0), &iDepth)) {
                            iResult = 0;
                            p0 = strtok(NULL, " .;\t");
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
                    showTree(pAGW, iID, iDepth, bWide);
                }
            } else {
                printf("expected number (ID)\n");
            }
        } else if ((strstr(pCommand, "ancestors") == pCommand) || isShortCut(pCommand, 'a')){
            pCommand = strpbrk(pCommand, " \t:");
            int iID=-1;
            int iDepth=DEF_DEPTH;
            bool bWide = false;
            int iResult = -1;
            char *p0 = strtok(pCommand, " .;\t");
            if (p0 != NULL) {
                if (strToNum(trim(p0), &iID)) {
                    iResult = 0;
                    p0 = strtok(NULL, " .;\t");
                    if (p0 != NULL) {
                        if (strToNum(trim(p0), &iDepth)) {
                            iResult = 0;
                            p0 = strtok(NULL, " .;\t");
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
                    showAncestors(pAGW, iID, iDepth, bWide);
                }
            } else {
                printf("expected number (ID)\n");
            }
            /*
        } else if ((strstr(pCommand, "prune") == pCommand) || isShortCut(pCommand, 'x')) {
            prune(pAG);
        } else if ((strstr(pCommand, "refresh") == pCommand) || isShortCut(pCommand, 'R')) {
            pAG->collectRootsLeaves();
            */
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
    int iOracleBlockSize = DEF_OSIZE;

    if (iArgC > 1) {
        strcpy(sAGFile, apArgV[1]);
        if (iArgC > 2) {
            iOracleBlockSize = atoi(apArgV[2]);
        }

        if (iOracleBlockSize > 0) {
            
            AGWindow *pAGW = AGWindow::createInstance(sAGFile, iOracleBlockSize);
            if (pAGW != NULL) {

                bool bGoOn = true;
                printf("Enter a command at the prompt\n");
                while (bGoOn) {
                    printf("> ");
                    char sCommand[256];
                    char *p = fgets(sCommand, 256, stdin);
                    if (p != NULL) {
                        bGoOn = action(pAGW, p);
                    }
                }

                delete pAGW;
            } else {
                printf("Couldn't create AGOracle for [%s]\n", apArgV[1]);
            }

        } else {
            printf("Bad OracleSize [%s]. (shold be a positive number (default %d))\n", apArgV[2], DEF_OSIZE);
        }
    } else {
        iResult = -1;
        printf("%s - inspect large ancestor graph\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <ancestorgraphfile> [<oracleblocksize>]\n", apArgV[0]);
        printf("      starts an interactive loop\n");
        printf("      with various commands\n");
        printf("\n");
        showCommands();
    }
                
    return iResult;
}
