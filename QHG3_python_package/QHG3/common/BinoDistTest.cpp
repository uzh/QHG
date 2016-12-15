#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BinomialDist.h"



int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 3) {
        double dP = atof(apArgV[1]);
        int iN = atoi(apArgV[2]);
        double dEps = atof(apArgV[3]);
        if ((dP > 0) && (iN > 0) && (dEps > 0)) {
            BinomialDist *pCB = BinomialDist::create(dP, iN, dEps);
            if (pCB != NULL) {
                //                pCB->showTable();
                int iLoop = 10000;
                int iMax = 0;
                int iMaxPos = -1;
                int *aCounts = new int[iN+1];
                memset(aCounts, 0, (iN+1)*sizeof(int));
                for (int i = 0; i < iLoop; ++i) {
                    double p = (1.0*rand())/RAND_MAX;
                    int k = pCB->getN(p);
                    aCounts[k]++;
                    if (aCounts[k] > iMax) {
                        iMax = aCounts[k];
                        iMaxPos = k;
                    }
                }

                printf("Max %d at pos %d\n", iMax, iMaxPos);
                for (int i = 0; i <= iN; ++i) {
                    printf("%08d:", aCounts[i]); 
                    for (int k = 0; k < (50.0*aCounts[i])/iMax; k++) {
                        printf("*");
                    }
                    printf("\n");
                }
                delete[] aCounts;
                delete pCB;
            } else {
                printf("Couldn't create CumuBinoProb\n");
            }
        } else {
            printf("The params must be positive numbers\n");
        }
    } else {
        printf("%s <prob0> <N> <epsilon>\n", apArgV[0]);
    }
    return iResult;
}
