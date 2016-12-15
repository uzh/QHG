#include <stdio.h>

#include "types.h"
#include "DistMat.h"

const char *g1[] = {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAATTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAATTTTTTTTTTTTTTTTTTTTGTTTTTTTTTTTT",
                    "TTTTTTTTTTTTTTTTTTTTGTTTTTTTTTTTTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
                    "TTTTTTTTTTTTTTTTTTTTGTTTTTTTTTTTTAAAAAAAAAAAAAAAAAAAAAAAACCAAAAAA",
                    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGTGATTTACGTATTTATATGCGTGATTTTTTTTT",
            
};

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    int iN = sizeof(g1)/sizeof(char*);
    int iG =  sizeof(g1[0]);
    DistMat *pDM = DistMat::createDistMat(iG, g1, iN);
    printf("DM is %p\n", pDM);
    if (pDM != NULL) {
        int **pM = pDM->createMatrix();
        for (int i = 0; i < iN; i++) {
            for (int j = 0; j < iN; j++) {
                printf("  % 3d", pM[i][j]);
            }
            printf("\n");
        }

        delete pDM;
    }
    return iResult;
}
