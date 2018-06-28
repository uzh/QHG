#include <stdio.h>
#include <string.h>
#include <vector>

#include "types.h"
#include "GenomeVarChecker.h"


//----------------------------------------------------------------------------
// showStats
//
void showStats(uchar *pCounts, int iGenomeSize, int iNumNucs) {
    int iMin = 999999;
    int iMax = 0;
    int aiFirst[4];
    aiFirst[0] = -1;
    aiFirst[1] = -1;
    aiFirst[2] = -1;
    aiFirst[3] = -1;
    int iF = 4;
    double dAvg = 0.0;
    int freq[5];
    memset(freq, 0, 5*sizeof(int));

    for (int i = 0; i < iGenomeSize; i++) {
        uchar c = *pCounts++;
        if (iF > 0) {
            for (int k = 0; k < iNumNucs; k++) {
                if ((aiFirst[k] < 0) && (c == k+1)) {
                    aiFirst[k] = i;
                    iF--;
                }
            }
        }

        if (c > iMax) {
            iMax = c;
        }
        if (c < iMin) {
            iMin = c;
        }
        dAvg += c;
        if (c < iNumNucs+1) {
            freq[c]++;
        } else {
            printf("i=%d: Found count [%d]}\n", i, c);
        }
    }
    dAvg /= iGenomeSize;

    printf("  Avg: %5.3f\n", dAvg);
    printf("  Min: %d\n", iMin);
    printf("  Max: %d\n", iMax);
    for (int i = 1; i < iNumNucs+1; i++) {
        printf("  # number of sites with %d variant%s  %d\n", i, ((i==1)?": ":"s:"), freq[i]);
    }
    for (int i = 0; i < iNumNucs; i++) {
        printf("First site with %d variant%s: %d\n", i+1, (i > 0)?"s":"", aiFirst[i]);
    }
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    if (iArgC > 1) {
        // check to see if it is a qdf file or a bin file
        int iPos = 1;
        int iShift = 0;
        bool bQDF = true;
        char *pPopName = NULL;
        if (iArgC > 2) {
            if (strcmp(apArgV[1], "-b") == 0) {
                bQDF=false;
                iShift = 1;
            } else  if (strcmp(apArgV[1], "-s") == 0) {
                if (iArgC > 3) {
                    pPopName = apArgV[2];
                    iShift = 2;
                } else {
                    printf("option '-s' needs to be followed by pop name\n");
                    iResult = -1;
                }
            }
        }

        // create GenomeVarCheck-objects for all pop/bin files
        std::vector<GenomeVarChecker*> vpGVC;
        int iGenomeSize = -1;
        iPos += iShift;
        while ((iResult == 0) &&  (iPos < iArgC)) {
            int iBufSizeGenomes = 1000000;
            GenomeVarChecker *pGVC = NULL;
            if (bQDF) {
                pGVC = GenomeVarChecker::createInstance(apArgV[iPos++], pPopName, "Genetics_genome_size", "Genetics_bits_per_nuc", "Genome", iBufSizeGenomes);
            } else {
                pGVC = GenomeVarChecker::createInstance(apArgV[iPos++], iBufSizeGenomes);
            }
            if (pGVC != NULL) {
                int iGenomeSizeA = pGVC->getGenomeSize();
                if (iGenomeSize < 0) {
                    iGenomeSize = iGenomeSizeA;
                } else {
                    if (iGenomeSize != iGenomeSizeA) {
                        iResult = -1;
                        printf("Genome sizes %d for [%s] does not match previous sizes (%d)\n", iGenomeSizeA, apArgV[iPos-1], iGenomeSize);
                    }
                }      
                if (iResult == 0) {
                    vpGVC.push_back(pGVC);
                }
            } else {
                printf("creating instance for [%s] failed\n", apArgV[iPos-1]);
                iResult = -1;
            }
        }
        
        // now evaluate all
        if (iResult == 0) {
            for (uint i = 0; i < vpGVC.size(); i++) {
                int iNumGenomes = vpGVC[i]->getNumGenomes();
                uint iB = vpGVC[i]->getBitsPerNuc();
                printf("Stats for [%s]\n", apArgV[iShift+1+i]);
                uchar *ucCounts2 = vpGVC[i]->getCounts2();
                showStats(ucCounts2, iGenomeSize, 1<<iB);
                float *pOrderedFreqs = vpGVC[i]->getOrderedFreqs();
                printf("# Average relative frequency of\n");
                if (iB == 1) {
                    printf("#   most common nucleotide:        %f (%f)\n", pOrderedFreqs[1]/(2*iNumGenomes), pOrderedFreqs[1]);
                    printf("#   second most common nucleotide: %f (%f)\n", pOrderedFreqs[0]/(2*iNumGenomes), pOrderedFreqs[0]);
                } else {
                    printf("#   most common nucleotide:        %f (%f)\n", pOrderedFreqs[3]/(2*iNumGenomes), pOrderedFreqs[3]);
                    printf("#   second most common nucleotide: %f (%f)\n", pOrderedFreqs[2]/(2*iNumGenomes), pOrderedFreqs[2]);
                    printf("#   third most common nucleotide:  %f (%f)\n", pOrderedFreqs[1]/(2*iNumGenomes), pOrderedFreqs[1]);
                    printf("#   least common nucleotide:       %f (%f)\n", pOrderedFreqs[0]/(2*iNumGenomes), pOrderedFreqs[0]);
                }
                float fNonZero = 0;
                int *pNonZeroCounts = vpGVC[i]->getNonZeroCount();
                for (int j = 0; j < 2 * iGenomeSize; j++) {
                    fNonZero += pNonZeroCounts[j];
                }
                fNonZero /= (2*iNumGenomes);
                printf("# Average number of non-zero nucleotides: %f\n", fNonZero);
                
            }
        }
        for (uint i = 0; i < vpGVC.size(); i++) {
            delete vpGVC[i];
        }
    } else {
        printf("Usage: %s [-s <species_name>] <qdf1> [<qdf2>]*\n", apArgV[0]);
        printf("       %s -b <binfile> [<binfile>]*\n", apArgV[0]); 
        iResult = -1;
    }
    return iResult;
}


