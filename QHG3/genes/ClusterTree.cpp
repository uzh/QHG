#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <set>
#include <vector>
#include <string>

typedef struct settree {
    intset s;
    std::set<settree *> sC;
    std::string ss;
} settree;


//----------------------------------------------------------------------------
// printset
//
void printset(intset &s) {
    printf("( ");
    intset::const_iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        printf("%d ", *it);
    }
    printf(")");
}

//----------------------------------------------------------------------------
// branchshow
//
void branchshow (settree *pS, const char *pIndent) {
    if ((pS != NULL) && (strlen(pIndent) < 51)) {
        printf("%s", pIndent);
        printset(pS->s);
        printf(": %zd branches\n",  pS->sC.size());
        
        std::set<settree*>::const_iterator it2;
        for (it2 = pS->sC.begin(); it2 != pS->sC.end(); ++it2) {
            char sIndent[1024];
            sprintf(sIndent, "  %s", pIndent);
            branchshow(*it2, sIndent);
        }
    }
}


//----------------------------------------------------------------------------
// treeprintR
//
void treeprintR(settree *pst, const char *pIndent, std::vector<std::pair<std::string,int> >&vNames) {
    char sNewIndent[8192];
    if (pst->sC.size() == 2) {
        printf("+--");
        sprintf(sNewIndent, "%s|  ", pIndent);
	treeprintR(*(pst->sC.begin()), sNewIndent, vNames);
	printf("%s\n", sNewIndent);
	printf("%s+--", pIndent);
        sprintf(sNewIndent, "%s   ", pIndent);
	treeprintR(*(pst->sC.rbegin()), sNewIndent, vNames);
    } else if (pst->sC.size() == 1) {
        printf("---");
        sprintf(sNewIndent, "%s   ", pIndent);
	treeprintR(*(pst->sC.begin()), sNewIndent, vNames);
    } else if (pst->sC.size() == 0) {
        //printf("%d\n", *(pst->s.begin()));
        if (pst->ss.empty()) {
            printf("%2d:%s\n", *(pst->s.begin()), vNames[*(pst->s.begin())].first.c_str());
            // set names on the 
            std::string &s1 = vNames[*(pst->s.begin())].first;
            int iPos = s1.find_last_of("_");
            pst->ss = s1.substr(0, iPos);
        } else {
            printf("%s\n", pst->ss.c_str());
        }
            
    }

}


//----------------------------------------------------------------------------
// treemergeR
//
void treemergeR(settree *pst, const char *pIndent) {
    char sIndent[2048];
    sprintf(sIndent, "%s  ", pIndent);
    if (pst->sC.size() == 2) {
        settree *pst1 = *(pst->sC.begin());
        settree *pst2 = *(pst->sC.rbegin());
        if (pst1->ss.empty()) {
            treemergeR(pst1, sIndent);
        }
        if (pst2->ss.empty()) {
            treemergeR(pst2, sIndent);
        }
        if (!(pst1->ss.empty()|| pst2->ss.empty())) {
            if (pst1->ss == pst2->ss) {
                pst->ss = pst1->ss;
                delete pst1;
                delete pst2;
                pst->sC.clear();
            }
        }
    } else if (pst->sC.size() == 1) {
        settree *pst1 = *(pst->sC.begin());
        if (pst1->ss.empty()) {
            treemergeR(pst1, sIndent);
        }
        if(!pst1->ss.empty()) {
            pst->ss = pst1->ss;
            delete pst1;
            pst->sC.clear();
        } else { 
            if (pst1->sC.size() == 1) {
                
                settree *pst2 = *(pst1->sC.begin());
                pst->sC.clear();
                pst->sC.insert(pst2);
                delete pst1;
                pst->ss = pst2->ss;
            }
            
        }
    }
}

//----------------------------------------------------------------------------
// treeprint
//
void treeprint(settree *pst, std::vector<std::pair<std::string,int> >&vNames) {
  
    char sNewIndent[8192];
    sprintf(sNewIndent, "   ");
    printf("---");
    treeprintR(pst, sNewIndent, vNames);
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::vector<std::pair<std::string, int> > vNames;
    std::vector<std::vector<int> > vValues;
    
    FILE *fIn = fopen(apArgV[1], "rt");
    
    char sLine[2000];
    int i = 0;
    while ((iResult == 0) && !feof(fIn)) {
        
        std::vector<int> vv;
        vValues.push_back(vv);
        char *p = fgets(sLine, 2000, fIn);
        if (p != NULL) {
            if (*p != '\n') {
                //@@                printf("Doing line [%s]\n", p);
                char *p1 = strtok(p, " \t\n");
                if (p1 != NULL) {
		    vNames.push_back(std::pair<std::string,int>(p1, i));
                    char *p2 = strtok(NULL, " \t\n");
                    if (p2 != NULL) {
                        std::pair<std::string, int> si(p1, atoi(p2));
                        p1 = strtok(NULL, " \t\n");
                        while (p1 != NULL) {
                            vValues[i].push_back(atoi(p1));
                            p1 = strtok(NULL, " \t\n");
                        }    
                    }
                }
                i++;
            }
        }
    }       
    printf("Values read\n");

    // put values in integer array
    int iN = i; //vValues.size();
    printf("Processing blocks size %d\n", iN);
    int **ppValues = new int*[iN];
    for (int i = 0; i < iN; i++) {
         ppValues[i] = new int[iN];
         for (int j = 0; j < iN; j++) {
             ppValues[i][j] = vValues[i][j];
         }
    }

    // array of array of sets
    intset **ppBlocks = new intset*[iN];
    for (int i = 0; i < iN; i++) {
        ppBlocks[i] = new intset[iN];
    }

    // fill the sets
    std::vector<std::vector<intset > > vBlocks(vValues.size());
    for ( int i = 0; i < iN; i++) {
        std::vector<intset > vSlice(vValues.size());
        for ( int j = 0; j < iN; j++) {
            ppBlocks[j][ppValues[i][j]].insert(i);
            //@@            printf("step %d: item %d is part of set %d\n", j, i, ppValues[i][j]);
        }
    }
/*
    printf("Result:\n");

    for ( int i = 0; i < iN; i++) {
        printf("|");
        
        for ( int j = 0; j < iN; j++) {
            intset::const_iterator it;
            for (it = ppBlocks[i][j].begin(); it != ppBlocks[i][j].end(); ++it) {
                printf(" %d", *it);
            }
            //            if  ((j < iN-1) && (ppBlocks[i][j+1].size() > 0)) {
            if  ((i == iN-1-j) || ((j < iN-1) && (ppBlocks[i][j+1].size() > 0))) {
                printf(" |");
            }
        }
        printf("\n");    
    }
*/
    std::vector<std::set<settree *> > vsst;
   
    for ( int i = iN-1; i >= 0; --i) {
        std::set<settree *> sst;

        for ( int j = 0; j < iN; j++) {
            settree *pCur = new settree;
            intset::const_iterator it;
            for (it = ppBlocks[i][j].begin(); it != ppBlocks[i][j].end(); ++it) {
                pCur->s.insert(*it);
            }
            if  ((i == iN-1-j) || ((j < iN-1) && (ppBlocks[i][j+1].size() > 0))) {
                if (!pCur->s.empty()) {
                    sst.insert(pCur);
                    pCur = new settree;

                }
            }
        }
        vsst.push_back(sst);
        //@@        printf("Level %zd: %zd sets\n", vsst.size()-1, vsst.back().size());
    }

    printf("---------\n");
    for (unsigned int i = 0; i < vsst.size(); ++i) {
        printf("i=%d:\n", i);
        std::set<settree *>::const_iterator it1;
        for (it1 = vsst[i].begin(); it1 != vsst[i].end(); ++it1) {
            printset((*it1)->s);
        }
        printf("\n");
    }
    printf("---------\n");
    
    for (unsigned int i = 1; i < vsst.size(); ++i) {
        //        printf("Level %d:\n", i);
        std::set<settree *>::const_iterator it1;
        int i0 = 0;
        for (it1 = vsst[i].begin(); it1 != vsst[i].end(); ++it1) {
            std::set<settree *>::const_iterator it2;
            int i1 = 0;
            /*
            printf("Doing ");
            printset((*it1)->s);
            printf("\n");
            */
            bool bSearching = true;            
            for (it2 = vsst[i-1].begin(); bSearching && (it2 != vsst[i-1].end()); ++it2) {
                /*
                printf("  comp ");
                printset((*it2)->s);
                printf("\n");
                */
                intset &s = (*it1)->s;
                intset::const_iterator it3;
                for (it3 = s.begin(); bSearching && (it3 != s.end()); ++it3) {
                    intset::const_iterator it4 = (*it2)->s.find(*it3);
                    if (it4 != (*it2)->s.end()) {
                        (*it2)->sC.insert(*it1);
                        bSearching = false;
                        /*
                        printf("Adding set {");
                        printset((*it1)->s);
                        printf("} of level %d to set {",i+1);
                        printset((*it2)->s);
                        printf("} of level %d\n", i);
                        */
                    }
                }
                i1++;
            }
            if ((*it1)->sC.size() > 0) {
                printf("%zd branches\n", (*it1)->sC.size());
            }
            i0++;
        }
    }
    settree *pRoot = *(vsst[0].begin());
    settree *pCur = pRoot;
    
    //branchshow(pCur, "");
    printf("---------\n");
    treeprint(pRoot, vNames);
    printf("---------\n");
    treemergeR(pRoot, "");
    treeprint(pRoot, vNames);
    return iResult;
}
