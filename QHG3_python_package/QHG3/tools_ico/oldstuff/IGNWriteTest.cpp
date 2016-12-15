#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "IcoGridNodes.h"
#include "IcoGridCreator.h"
#include "TrivialSplitter.h"



int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 2) {
        char sInputIco[1024];
        char sOutputIgn[1024];
        int iHalo = 0;
        bool bPreSel = false;
        bool bNodeOrder = false;
        int iMaxLinks = 0;
        bool bTileInfo = false;
        *sOutputIgn = '\0';
        strcpy(sInputIco, apArgV[1]);
        int iC = 2;
        while (iC < iArgC) {
            if (strcmp(apArgV[iC], "-t")== 0) {
                bTileInfo = true;
            } else if (strcmp(apArgV[iC], "-m")== 0) {
                iC++;
                if (iC < iArgC) {
                    char *pEnd;
                    iMaxLinks = strtol(apArgV[iC], &pEnd, 10);
                    if (*pEnd != '\0') {
                        iMaxLinks = -1;
                    }
                }
            } else if (strcmp(apArgV[iC], "-o")== 0) {
                bNodeOrder = true;
            } else {
                strcpy(sOutputIgn, apArgV[iC]);
            }
            iC++;
        }

        if (iMaxLinks >= 0) {
            if (*sOutputIgn != '\0') {
                TrivialSplitter *pTS = new TrivialSplitter();
                IcoGridCreator *pIGC = IcoGridCreator::createInstance(apArgV[1], bPreSel, iHalo, pTS, bNodeOrder);
                if (pIGC != NULL) {
                    IcoGridNodes *pIGN = pIGC->getGrid(0);
                    
                    iResult = pIGN->write(apArgV[2], iMaxLinks, bTileInfo);
                    if (iResult == 0) {
                        printf("+++ success +++\n");
                        IcoGridNodes *pIGN2 = new IcoGridNodes();
                        clock_t c1 = clock();
                        pIGN2->read(apArgV[2], false);
                        clock_t c2 = clock();
                        printf("read took %f secs\n", (1.0*(c2-c1))/CLOCKS_PER_SEC);
                        //                        pIGN->read(apArgV[2], true);
                        delete pIGN2;
                    } else {
                        printf("Error writing output to [%s]\n", sOutputIgn);
                    }
                    
                    delete pIGN;
                    delete pIGC;
                } else {
                    printf("Couldn't open icofile [%s]\n", sInputIco);
                }
                delete pTS;
            } else {
                printf("No output name provided\n");
            }
        } else {
            printf("Invalid '-m' argunent [%s]\n", apArgV[iC]);
        }
    } else {
        printf("Usage: %s <icofile_in> [-t] [-o] [-m <maxlinks>] <ignfile_out>\n", apArgV[0]);
    }
    return iResult;
}


