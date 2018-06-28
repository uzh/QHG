#include <stdio.h>
#include <string.h>

#include "types.h"
#include "GeneUtils.h"
#include "GenomeSegments.h"

//----------------------------------------------------------------------------
// display
//
void display(ulong *pGenome, int iNumBlocks, GenomeSegments *pGS) {
    //        pGS->showMasks();
    int iNumSegments = pGS->getNumSegments();
    int iSegSize = pGS->getSegmentSize(0);
    printf("%03d segments of length %03d\n", iNumSegments, iSegSize);
    char pNuc[33];
    pNuc[32] = '\0';
    
    for (int i = 0; i < iNumSegments; i++) {
        ulong lDummy = 0;
        ulong *p = pGS->getMaskedSegment(pGenome, i);
        int iFirstBlock = pGS->getFirstBlock(i);
        printf("    ");
        for (int j = 0; j < iFirstBlock; j++) {
            printf("%s ", GeneUtils::blockToNucStr(lDummy, pNuc));
        }
        for (int j = 0; j < pGS->getMaskLength(i); j++) {
            printf("%s ", GeneUtils::blockToNucStr(p[j], pNuc));
        }
        for (int j = iFirstBlock+pGS->getMaskLength(i); j < iNumBlocks; j++) {
            printf("%s ", GeneUtils::blockToNucStr(lDummy, pNuc));
        }
        
        printf("\n---\n");
    }
    
}

//----------------------------------------------------------------------------
// compare
//  create 2 pseudo genomes of size 2*masklen
//  by applying a msak to each of the original genomes' two strands
//  then use GeneUtils::calcDist()
// 
int compare(GenomeSegments *pGS, ulong *pGenome1, ulong *pGenome2, int iNumBlocks) {
    ulong *p1 = NULL;
    ulong *p2 = NULL;
    int iPrevLen = 0;
    int iNumSegments = pGS->getNumSegments();
    printf("Comparing\n");
    for (int i = 0; i < iNumSegments; i++) {
        printf("-- segment %d; size %d\n", i, pGS->getSegmentSize(i));
        int iMaskLen =  pGS->getMaskLength(i);
        if (iPrevLen < iMaskLen) {
            iPrevLen = iMaskLen;
            if (p1 != NULL) {
                delete[] p1;
            }
            p1 = new ulong[2*iMaskLen];
            if (p2 != NULL) {
                delete[] p2;
            }
            p2 = new ulong[2*iMaskLen];
        }
        ulong *p;
        // create masked parts for first strand
        p = pGS->getMaskedSegment(pGenome1, i);
        memcpy(p1, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2, i);
        memcpy(p2, p, iMaskLen*sizeof(ulong));

        // create masked parts for second strand
        p = pGS->getMaskedSegment(pGenome1+iNumBlocks, i);
        memcpy(p1+iMaskLen, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2+iNumBlocks, i);
        memcpy(p2+iMaskLen, p, iMaskLen*sizeof(ulong));

        int iDist = GeneUtils::calcDist(p1, p2,iMaskLen*GeneUtils::NUCSINBLOCK); 
        printf("Dist#%d : %d\n", i, iDist);
    }
    return 0;
}



//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    uchar cGeneBlock = 0x66; // 0xe4
    int iGenomeSize = 100;
    int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
    int iLastBit = 2*(GeneUtils::NUCSINBLOCK - iGenomeSize);

    ulong *pGenome1 = new ulong[2*iNumBlocks];
    memset(pGenome1, cGeneBlock, iNumBlocks*sizeof(ulong));
    memset(pGenome1+iNumBlocks, cGeneBlock, iNumBlocks*sizeof(ulong));
    pGenome1[iNumBlocks-1] >>= iLastBit;
    pGenome1[2*iNumBlocks-1] >>= iLastBit;
    GeneUtils::showGenome(pGenome1, iGenomeSize, SHOW_GENES_NUC);

    cGeneBlock = 0xe4;
    ulong *pGenome2 = new ulong[2*iNumBlocks];
    memset(pGenome2, cGeneBlock, iNumBlocks*sizeof(ulong));
    memset(pGenome2+iNumBlocks, cGeneBlock, iNumBlocks*sizeof(ulong));
    pGenome2[iNumBlocks-1] >>= iLastBit;
    pGenome2[2*iNumBlocks-1] >>= iLastBit;
    GeneUtils::showGenome(pGenome2, iGenomeSize, SHOW_GENES_NUC);


    GenomeSegments *pGS = GenomeSegments::createInstance(iGenomeSize, 4);
    if (pGS != NULL) {
        display(pGenome1, iNumBlocks,  pGS);
        delete pGS;
    } else {
        printf("Couldn't create GenomeSegments1\n");
    } 
    iResult = 0;
    pGS = GenomeSegments::createInstance(iGenomeSize);
    if (pGS != NULL) {
        if (iResult == 0) {
            iResult =  pGS->addSegment(12, 33);
        }
        if (iResult == 0) {
            iResult =  pGS->addSegment(46,10);
        }
        if (iResult == 0) {
            iResult =  pGS->addSegment(56, 22);
        }
        if (iResult == 0) {
            iResult =  pGS->addSegment(88, 44);
        }
        if (iResult == 0) {
            display(pGenome1, iNumBlocks,  pGS);
        } else {
            printf("problem\n");
        }

        compare(pGS, pGenome1, pGenome2, iNumBlocks);

        delete pGS;
    } else {
        printf("Couldn't create GenomeSegments2\n");
    } 


    delete[] pGenome1;
    delete[] pGenome2;
    return iResult;
}


