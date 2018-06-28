#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icoutil.h"
#include "Region.h"
#include "RectRegion.h"
#include "RegionSplitter.h"
#include "RectSplitter.h"

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 3) {
        bool bStrict = false;
        bool bGrid = false;
        int iNX = atoi(apArgV[1]);
        if (iNX > 0) {
            iResult = 0;
            int iNY = 0;
            if (iArgC > 2) {
                iNY = atoi(apArgV[2]);
                if (iNY > 0) {
                    iResult = 0;
                } else {
                    if (strcmp(apArgV[2], "grid") == 0) {
                        bGrid = true;
                    } else if (strcmp(apArgV[2], "free") == 0) {
                        bStrict = false;
                    } else {
                        printf("Unknown argument: [%s]\n", apArgV[3]);
                        iResult = -1;
                    }
                }
                if (iResult == 0) {
                    if (strcmp(apArgV[3], "strict") == 0) {
                        bStrict = true;
                    } else if (strcmp(apArgV[3], "free") == 0) {
                        bStrict = false;
                    } else {
                        printf("Unknown argument: [%s]\n", apArgV[3]);
                        iResult = -1;
                    }
                }

                if (iResult == 0) {
                    RectSplitter *pRS = NULL;
                    if (iNY > 0) {
                        pRS = new RectSplitter(NULL, iNX, iNY, bStrict);
                    } else {
                        pRS = new RectSplitter(NULL, iNX, bGrid, bStrict);
                    }
                    //                    tbox box(0, 120, 0, 180);
                    tbox box(0, 4, 0, 4);
                    pRS->setBox(&box);
                    int iActual = 0;
                    Region **ppR = pRS->createRegions(&iActual);
                    if (ppR != NULL) {
                        printf("created %d tiles\n", iActual);
                        for (int i = 0; i < iActual; i++) {
                            ppR[i]->display();
                        }
                    } else {
                        printf("--- error ---\n");
                    }
                    delete pRS;
                }

            }
        } else {
            printf("First arg must be positive int\n");
        }
    } else {
        printf("Usage:\n");
        printf("  %s <NX> <NY> (\"strict\"|\"free\")\n", apArgV[0]);
        printf("or\n");
        printf("  %s <Num> (\"grid\"|\"free\") (\"strict\"|\"free\")\n", apArgV[0]);
    }
    return iResult;
}
