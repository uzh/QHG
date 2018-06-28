#include <stdio.h>
#include <string.h>

#include "AlleleCounter.h"


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - count alleles in genomes\n", pApp);
    printf("Usage:\n");
    printf("  %s <qdfpopfile> [<speciesname>]\n", pApp);
    printf("where\n");
    printf("  qdfpopfile     a QDF file with at least one population group\n");
    printf("  speciesname   nam of species to be analyzed (default: first species found)\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {

    int iResult = -1;

    if (iArgC > 1) {
        char *pSpeciesName = NULL;
        if (iArgC > 2) {
            pSpeciesName = apArgV[2];
        }

        AlleleCounter *pAC = AlleleCounter::createInstance(apArgV[1], pSpeciesName);
        if (pAC != NULL) {
            iResult = 0;
            
            pAC->countAlleles(SEL_GENDER_F);
            uint **ppCounts   = pAC->getCounts();
            int iGenomeSize  = pAC->getGenomeSize();
            int iNumNucs     = pAC->getNumNucs();
            int iNumGenomes  = pAC->getNumGenomes();

            float **fFreq = new float *[iGenomeSize];
            float avg[4];
            memset(avg, 0, 4*sizeof(int));
            printf("NumGenomes: %d\n", iNumGenomes);
            for (int i = 0; i < iGenomeSize; i++) {
                fFreq[i] = new float[iNumNucs];
                for (int j = 0; j < iNumNucs; j++) {
                    fFreq[i][j] = (1.0*ppCounts[i][j])/iNumGenomes;
                }
                if (fFreq[i][0] > 0) {
                    printf("freq[%d]: ", i);
                    for (int j = 0; j < iNumNucs; j++) {
                        printf(" %f (%d)", fFreq[i][j], ppCounts[i][j]);
                    }
                    printf("\n");
                }
                std::sort(fFreq[i], fFreq[i]+iNumNucs);
                for (int j = 0; j < iNumNucs; j++) {
                    avg[j] += fFreq[i][j];
                }
            }

            printf("avgs:\n");
            for (int j = 0; j < iNumNucs; j++) {
                printf("  %f", (1.0*avg[j])/iGenomeSize);
            }
            printf("\n");

            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < iNumNucs; j++) {
                    printf(" %f", fFreq[i][j]);
                }
                printf("\n");
            }

            
            for (int i = 0; i < iGenomeSize; i++) {
                delete[] fFreq[i];
            }
            delete[] fFreq;

            delete pAC;
        } else {
            printf("Couldn't create AlleleCounter for [%s] (%s)\n", apArgV[1], (pSpeciesName == NULL)?"first species":pSpeciesName);
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
