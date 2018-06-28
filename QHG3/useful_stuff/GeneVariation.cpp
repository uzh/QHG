#include <stdio.h>
#include <string.h>
#include <hdf5.h>
#include <omp.h>

#include "types.h"
#include "ParamReader.h"
#include "QDFAllGenomes.h"

ulong aGenos[8][2] = {
    { 0xab34c1006ef28854, 0x0123456789abcdee},
    { 0x445f331cade67900, 0x35faaddef2849ca9},
    { 0x7ffad533babb3762, 0xa003f7e2265194a1},
    { 0x71fa1935bce56a6a, 0xcfeedede712394d5},
    { 0xbe3452f8dd9a5e12, 0x2718281828abcded},
    { 0x31415926574abcd1, 0x06182fe67accc329},
    { 0x141421356575feaa, 0xcafebadfeedbeef1},
    { 0xaaaaaaaaaaaaaaaa, 0x5555555555555555}
};

//----------------------------------------------------------------------------
// calcVariation
//
int calcVariation(ulong **pGenomes, int iNumLong, int iNumGenomes, int *piMaxVar) {
    int iVariation = 0;
    *piMaxVar = 0;
    uchar aVar[4];

    for (int iSegment = 0; iSegment < iNumLong; iSegment++) {
        ulong iMask = 0x3;
        int iSubVar = 0;
        for (int iShift = 0; iShift < 32; iShift++) {
            memset(aVar, 0, 4*sizeof(uchar));
            for (int j = 0; j < iNumGenomes; j++) {
                int iVariant = (pGenomes[j][iSegment] & iMask) >> (2*iShift);
                //                printf("  %lx -> m:%lx ->iVar:%x\n", pGenomes[j][iSegment], (pGenomes[j][iSegment] & iMask), iVariant);
                aVar[iVariant] = 1;
            }
            iMask <<= 2;

            int iCurVar =  aVar[0]+aVar[1]+aVar[2]+aVar[3];
            if (iCurVar > *piMaxVar) {
                *piMaxVar = iCurVar;
            }
            //            printf("Seg[%d], shift[%d]:Var = %d\n", iSegment, iShift, iCurVar);
            iSubVar += iCurVar;
        }
        iVariation += iSubVar;
    }
    return iVariation;
}



//----------------------------------------------------------------------------
// simpleTest
//
int simpleTest() {
    
    ulong **pGenos = new ulong*[8];
    for (int i = 0; i < 8; i++) {
        pGenos[i] = new ulong[2];
        pGenos[i][0] = aGenos[i][0];
        pGenos[i][1] = aGenos[i][1];
    }
    int x = 0;
    int iTotal1 = calcVariation(pGenos, 2, 8, &x);
    printf("Total var1: %d\n", iTotal1);

 
    for (int i = 0; i < 8; i++) {
        delete[] pGenos[i];
    }
    delete[] pGenos;
    return iTotal1;
}

#define DEF_GS_ATTR    "Genetics_genome_size"
#define DEF_G_DATASET  "Genome"

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    
    char *sQDFFile = NULL;
    char *sSpeciesName = NULL;
    char sGenomeSizeAttribute[256];
    char sGenomeDataSet[256];
    *sGenomeSizeAttribute = '\0';
    *sGenomeDataSet = '\0';

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(4,
                                 "-q:S!", &sQDFFile,
                                 "-s:S",  &sSpeciesName,
                                 "-g:s",  sGenomeSizeAttribute,
                                 "-G:s",  sGenomeDataSet);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (*sGenomeSizeAttribute == '\0') {
                strcpy(sGenomeSizeAttribute, DEF_GS_ATTR);
            }
            if (*sGenomeDataSet == '\0') {
                strcpy(sGenomeDataSet, DEF_G_DATASET);
            }

            double d1 = 0;
            double d2 = 0;
            double d3 = 0;
            d1 = omp_get_wtime();    
        
            QDFAllGenomes *pQAG = QDFAllGenomes::createInstance(sQDFFile, sSpeciesName, sGenomeSizeAttribute, sGenomeDataSet);
            //QDFAllGenomes *pQAG = QDFAllGenomes::createInstance("/home/jody/Simulations/gentest07/gen_pop-Sapiens__040000.qdf", "Sapiens", "Genetics_genome_size", "Genome");
            if (pQAG != NULL) {
                iResult = pQAG->extractGenomes();
                d2 = omp_get_wtime();    
                if (iResult == 0) {
                    printf("Extracted %d genomes of size %d\n", pQAG->getNumGenomes(), pQAG->getGenomeSize());
                    int iMaxVar = 0;
                    int iTotal1 = calcVariation(pQAG->getGenomes(), pQAG->getNumBlocks(), pQAG->getNumGenomes(), &iMaxVar);
                    printf("Total var: %d %d %d\n", iTotal1, iMaxVar, pQAG->getNumGenomes());
                    d3 = omp_get_wtime();    
                
                } else {
                    printf("Couldn't extract\n");
                }
                delete pQAG;
                printf("Extract: %f\n", d2-d1);
                printf("CalcVar: %f\n", d3-d2);
            } else {
                iResult = -1;
                printf("Failed to create QDFAllGenomes\n");
            }

        } else {
            printf("[GeneVariation::readOptions] Error reading options\n");
        }

    } else {
        printf("[GeneVariation::readOptions] Error setting options\n");
    }
    delete pPR;
    return iResult;
}

