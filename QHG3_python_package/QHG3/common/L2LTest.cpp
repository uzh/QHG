#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "L2List.h"



int testLowLevel() {
    int iResult = 0;

    L2List *pL2LA = new L2List(4);
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);

  
    pL2LA->addElement();
    pL2LA->addElement();
    pL2LA->addElement();
    pL2LA->addElement();
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    
    pL2LA->removeElement(0);
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    pL2LA->removeElement(1);
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    /*
    pL2LA->removeElement(2);
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    pL2LA->removeElement(3);
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    printf("adding again\n");
    pL2LA->addElement();

    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    */
    uint iASize = 30;
    uint aiHoles[iASize];
    uint aiActive[iASize];

    printf("collecting compact info\n");
   
    uint iRes = pL2LA->collectFragInfo(iASize, aiHoles, aiActive);
    printf("first call to collectFragInfo returned %d\n",iRes);
    while (iRes > 0) {
        printf("collectFragInfo returned %d\n",iRes);
        uint iNum = iRes;
        if (iNum > iASize) {
            iNum = iASize;
        }
 
        printf("Holes:   ");
        for (uint i = 0;i < iNum; i++) {
            printf("%02d ", aiHoles[i]);
        }
        printf("\n");
        printf("Actives: ");
        for (uint i = 0;i < iNum; i++) {
            printf("%02d ", aiActive[i]);
        }
        printf("\n");
        pL2LA->defragment(iNum, aiActive);
        if (iRes > iASize) {
            iRes = pL2LA->collectFragInfo(iASize, aiHoles, aiActive);
        } else {
            iRes = 0;
        }
    }
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
 

    delete pL2LA;
    return iResult;
}


void fulltest(int iSize, int iNum) {
    L2List *pL2LA = new L2List(iSize);
    for (int i = 0; i < iSize; i++) {
        pL2LA->addElement();
    }
    unsigned char puRec[iSize];
    memset(puRec, 1, iSize * sizeof(unsigned char));
    int iNumUsed = 0;
    int iMaxUsed = 0;
    for (int i = 0; i < iNum; i++) {
        /*
          for (int i = 0; i < iSize; i++) {
          printf("%d ", puRec[i]);
          }
        */
        int x = (1.0*iSize*rand())/RAND_MAX;
        if (puRec[x] == 0) {
            /*
              int y = pL2LA->addElement();
              if (y >= 0) {
              //                printf("added %d    \r", y);
              puRec[y] = 1;
              iNumUsed++;
              if (iNumUsed > iMaxUsed) {
              iMaxUsed = iNumUsed;
              printf("%d: %d\n", i, iMaxUsed);
              }
              } else {
              printf("errrrrrrrrrrr\n");
              }
            */
        } else { if (puRec[x] > 0) {
                pL2LA->removeElement(x);
                //            printf("removed %d  \r", x);
                puRec[x] = 0;
                iNumUsed--;
            }
        }
    }

    printf("\n");
    for (int i = 0; i < iSize; i++) {
        printf("%d ", puRec[i]);
    }
    printf("\n");
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
    printf("Maxused %d\n", iMaxUsed);
    
    uint iASize = 9;
    uint aiHoles[iASize];
    uint aiActive[iASize];

    printf("collecting compact info\n");
    uint iRes = pL2LA->collectFragInfo(iASize, aiHoles, aiActive);
    printf("first call to collectFragInfo returned %d\n",iRes);
    while (iRes > 0) {
        printf("collectFragInfo returned %d\n",iRes);
        uint iNum = iRes;
        if (iNum > iASize) {
            iNum = iASize;
        }
 
        printf("Holes:   ");
        for (uint i = 0;i < iNum; i++) {
            printf("%02d ", aiHoles[i]);
        }
        printf("\n");
        printf("Actives: ");
        for (uint i = 0;i < iNum; i++) {
            printf("%02d ", aiActive[i]);
        }
        printf("\n");
        pL2LA->defragment(iNum, aiActive);
        if (iRes > iASize) {
            iRes = pL2LA->collectFragInfo(iASize, aiHoles, aiActive);
        } else {
            iRes = 0;
        }
    }
    pL2LA->ddisplay(ACTIVE);
    pL2LA->ddisplay(PASSIVE);
 
    delete pL2LA;
    
}

void killtest(int iSize, int iNum) {
    L2List *pL2LA = new L2List(8);
    for (int i = 0; i < 7; i++) {
        pL2LA->addElement();
    }
    pL2LA->displayArray(0, 8);
    for (int i = 0; i < 6; i++) {
        pL2LA->removeElement(i);
    }
    pL2LA->displayArray(0, 8);
    pL2LA->removeElement(6);
    pL2LA->displayArray(0, 8);
    pL2LA->removeElement(6);
    pL2LA->displayArray(0, 8);
    pL2LA->checkList();
    printf("--------\n");
    pL2LA->clear();
    for (int i = 0; i < 8; i++) {
        pL2LA->addElement();
    }
    pL2LA->removeElement(6);
    pL2LA->displayArray(0, 8);
    pL2LA->addElement();
    pL2LA->displayArray(0, 8);
    pL2LA->addElement();
    pL2LA->displayArray(0, 8);
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    //   testLowLevel();
    //    fulltest(20, 900000);
        //    fulltest(40, 20);
        killtest(0,0);
    return iResult;
}
