#include <stdio.h>
#include <string.h>


#include "GeneUtils.h"
#include "PlinkReader.h"


void usage(const char *pApp) {
    printf("%s - convert a ped file to a qhg binary genome file \n", pApp);
    printf("usage:\n");
    printf("  %s <input-ped> <genome-size> <output-bin>\n", pApp);
    printf("where\n");
    printf("  <input-ped>     the plink file to convert\n");
    printf("  <genome-size>   number of sites\n");
    printf("  <output-bin>    name of the binary output file\n");
    printf("\n"); 
}

int main(int iArgC, char*apArgV[]) {
    int iResult = -1;
    if (iArgC > 3) {
        int iGenomeSize = atoi(apArgV[2]);
        printf("STarting PLinkReader\n");
        PlinkReader *pPR = PlinkReader::createInstance(apArgV[1], iGenomeSize);
        if (pPR != NULL) {
            iResult = pPR->readGenomes();
            if (iResult == 0) {
                iResult = pPR->writeBin(apArgV[3]);
                if (iResult == 0) {
                    printf("successfully written bin file\n");
                } else {
                    printf("error writing bin file\n");
                }
            } else {
                printf("error reading ped file\n");
            }
            delete pPR;
        }
        printf("finished\n");
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
