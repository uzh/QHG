#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <set>

#include "types.h"
#include "Coalescer.h"



int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 5) {
        bool bParallel = false;
        int iOffset = 0;
        if (strcmp(apArgV[1], "-p") == 0) {
            bParallel = true;
            iOffset = 1;
        }
        int iBlockSize = atoi(apArgV[2+iOffset]);
        if (iBlockSize > 0) {
            std::set<idtype> sOriginal;
            for (int i = 3; (iResult == 0) && (i+iOffset < iArgC); ++i) {
                idtype id = atol(apArgV[i+iOffset]);
                if (id > 0) {
                    sOriginal.insert(id);
                } else {
                    iResult = -1;
                    printf("ids must be positive integers\n");
                }
            }
            if ((iResult == 0) && (sOriginal.size() > 0)) {
                Coalescer *pCC = Coalescer::createInstance(apArgV[1+iOffset], iAncSize, iBlockSize);
                if (pCC != NULL) {
                    
                    iResult = pCC->doCoalesce(sOriginal, bParallel);
                    if (iResult == 0) {
                        uint i = 0;
                        printf("\n");
                        printf("    ID1       ID2    \t      ID    \t    Time     CellID\n");
                        printf("-------------------------------------------------------------\n");
                        for (idset_cit it1 = sOriginal.begin(); it1 != sOriginal.end(); ++it1) {
                            uint j = i+1;
                            idset_cit it2 = it1;
                            ++it2;

                            for (; it2 != sOriginal.end(); ++it2) {
                                const timeset& sCij = pCC->getCoalescents(i,j);
                                const timeentry &te = *(sCij.begin());
                                printf("[%9ld:%9ld]\t  %9ld\t[%8.1f; %8d]\n", *it1, *it2, te.first, te.second.fTime, te.second.iCellID);
                                ++j;
                            }

                            ++i;
                        }
                    } else {
                        iResult = -1;
                        printf("Problem in doCoalesce()\n");
                    }
                    delete pCC;
                } else {
                    iResult = -1;
                    printf("Couldn't create Coalescer for (%s, %d, %d)\n", apArgV[1+iOffset], iAncSize, iBlockSize);
                }
                
            } else {
                if (sOriginal.size() < 2) {
                    iResult = -1;
                    printf("There must be at least 2 different ids\n");
                }
            }
        } else {
            iResult = -1;
            printf("<ancsize> [%s] and <blocksize> [%s] must be positive integers\n", apArgV[2], apArgV[3]);
        }
    } else {
        iResult = -1;
        printf("Usage: %s [-p] <ancfile-4> <blocksize> <id> <id> [<id>*]\n", apArgV[0]);
    }
    return iResult;
}
