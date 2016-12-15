#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "IcoNode.h"
#include "IrregRegion.h"



//-----------------------------------------------------------------------------
// niceDisplay
//
void niceDisplay(nodelist &mNodes, int iW, int iH) {
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            gridtype lID = i*iW+j;
            gridtype lReg = mNodes[lID]->m_iRegionID;
            if (lReg < 0) {
                printf(".");
            } else {
                if (lReg < 10) {
                    printf("%lld", lReg);
                } else if (lReg < 36) {
                    printf("%c", (char)('A'+lReg-10));
                } else if (lReg < 62) {
                    printf("%c", (char)('a'+lReg-36));
                } else {
                    printf("#");
                }
            }
        }
        printf("\n");
    }    
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    /*
    // split of region 2 after 120: nodes 2565 2724 get separated from 2 region
    int iH = 32;
    int iW = 160;
    int N = 50;
    */
    /*
    // split of region 6 after 14: nodes 41 42 get separated from 6 region
    int iH = 32;
    int iW = 64;
    int N = 50;
    */
    /*
    // split of region 1 after 31: node 498 gets separated from 1 region 
    int iH = 32;
    int iW = 32;
    int N = 20;
    */
    /*
    // split of region 3 after 50: nodes 625 655 656 657 ...  get separated from 3 region
    int iH = 32;
    int iW = 32;
    int N = 17;
    */
    /*
    // split of region 7 after 43: nodes  536 560  get separated from 7 region
    int iH = 24;
    int iW = 24;
    int N = 17;
    */
    /*
    // split of region 4 after 9: nodes 222 223 242 243 263  get separated from 4 region
    int iH = 20;
    int iW = 20;
    int N = 16;
    */
    /*
    // split of region 0 after 29: nodes 187 gets separated from 0 region
    int iH = 15;
    int iW = 15;
    int N = 15;
    */
    /*
    // split of region 7 after 18: nodes 117 118 132 get separated from 7 region
    int iH = 14;
    int iW = 14;
    int N = 14;
    */
    /*
    // split of region 6 after 6: nodes 50 62 get separated from 6 region
    int iH = 12;
    int iW = 12;
    int N = 11;
    */
    /*
    // split of region 1 after 1: nodes 54 gets separated from 1 region
    int iH = 12;
    int iW = 12;
    int N = 10;
    */
    /*
    // split of region 0 after 4: nodes 90 102 113 get separated from 0 region
    int iH = 11;
    int iW = 11;
    int N = 9;
    */
    /*
    // split of region 1 after 5: node 33 gets separated from 1 region
    int iH = 9;
    int iW = 9;
    int N = 9;
    */
    /*
    // split of region 0 after 6: node 68 69 78 79 gets separated from 0 region
    int iH = 9;
    int iW = 9;
    int N = 8;
    */
    /*
    // split of region 1 after 0: node 27 gets separated from 1 region
    int iH = 8;
    int iW = 8;
    int N = 7;
    */
    /*
    // split of region 1 after 6: nodes 19 27 get separated from 1 region
    int iH = 8;
    int iW = 8;
    int N = 6;
    */
    
    // split of region 1 after 6: nodes 33 get separated from 1 region
    int iH = 8;
    int iW = 8;
    int N = 5;
    

    printf("LOWEST G/Y BORDER HAS ERROR AFTER 178 steps : nodes 4214 4215 get separated from G region\n");
    printf("LOWEST 2/B BORDER HAS ERROR AFTER 120 steps : nodes 2565 2724 get separated from 2 region\n");
    nodelist lInit;
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            gridtype lID = i*iW+j;
            IcoNode *pIN = new IcoNode(lID, i, j, 0);
            if (i > 0) {
                pIN->addLink(lID - iW, 1);
            }
            if (i < iH-1) {
                pIN->addLink(lID + iW, 1);
            }
            if (j > 0) {
                pIN->addLink(lID - 1, 1);
            }
            if (j < iW-1) {
                pIN->addLink(lID + 1, 1);
            }
            lInit[lID] = pIN;
            /*
            printf("pIN %lld: ", lID);
            for (int i = 0; i < pIN->m_iNumLinks; i++) {
                printf(" %lld", pIN->m_aiLinks[i]);
            }
            printf("\n");
            */
        }
    }


    IrregRegion *pIR = new IrregRegion(lInit);
    /*
    pIR->add(0, iW*2*iH/4   + iW/4);
    pIR->add(1, iW*iH/4   + 3*iW/4);
    pIR->add(2, iW*iH/2   + iW/2);
    pIR->add(3, iW*3*iH/4 + iW/4);
    pIR->add(4, iW*2*iH/4 + 3*iW/4);

    pIR->add(5, iW*3*iH/8 + 3*iW/4);
    pIR->add(6, 0);
    */

    for (int i =0; i < N; i++) {
        pIR->add(i, (int)((1.0*iW*iH*rand())/RAND_MAX));
    }
    pIR->listN();
    int iStep = 1;
    iResult = 1;
    while (iResult > 0) {
        printf("--- step %d ---\n", iStep++);
        iResult = pIR->grow();
        pIR->listN();
        niceDisplay(lInit, iW, iH);
    }

    iResult = 1;
    iStep=0;
    while ((iStep < 7) && (iResult > 0)) {
        printf("--- eq %d ---\n", iStep++);
        iResult = pIR->equalize(false);
        niceDisplay(lInit, iW, iH);
        pIR->listSizes();
    }
    iResult = 0;
    while ((iStep < 200) && (iResult > 0)) {
        printf("--- eq1 %d ---\n", iStep++);
        iResult = pIR->equalize(true);
        niceDisplay(lInit, iW, iH);
        pIR->listSizes();
    }

    pIR->listN();

    //    pIR->doesNotSplitRegion(2, 270, 4);
    //        pIR->doesNotSplitRegion(1, 114, 4);
    //  pIR->doesNotSplitRegion(8, 85, 4);
    delete pIR;

    nodelist::iterator it;
    for (it = lInit.begin(); it != lInit.end();++it) {
        delete it->second;
    }
    printf("LOWEST G/Y BORDER HAS ERROR AFTER 178 steps : nodes 4214 4215 get separated from G region\n");
    printf("LOWEST 2/B BORDER HAS ERROR AFTER 120 steps : nodes 2565 2724 get separated from 2 region\n");

    return iResult;
}
