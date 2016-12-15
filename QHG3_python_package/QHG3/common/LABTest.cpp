#include <stdio.h>

#include "LayerArrBuf.h"
#include "LayerArrBuf.cpp"
#include "LBController.h"

#define ARR_SIZE 5

long a0[] = {0, 1, 2, 3, 4};
long a1[] = {5, 6, 7, 8, 9};
long a2[] = {10, 11, 12, 13, 14};
long a3[] = {15, 16, 17, 18, 19};
long a4[] = {20, 21, 22, 23, 24};
long a5[] = {25, 26, 27, 28, 29};
long a6[] = {30, 31, 32, 33, 34};
long a7[] = {35, 36, 37, 38, 39};
long a8[] = {40, 41, 42, 43, 44};
long a9[] = {45, 46, 47, 48, 49};

long b[] =  {60, 61, 62, 63, 64,
             65, 66, 67, 68, 69,
             70, 71, 72, 73, 74,
             75, 76, 77, 78, 79};

void show(LayerArrBuf<long> &LAB, LBController *pLBC, int iArrSize) {
    
    printf("Have %u elements\n", pLBC->getNumUsed());
   
    int i = pLBC->getFirstIndex(ACTIVE);
    while (i != NIL) {
        long *pCur = &(LAB[i]);
        printf("[%d]: ", i);
        for (int j = 0; j < iArrSize; j++) {
            printf("%ld ", pCur[j]);
        }
        printf("\n");
        i = pLBC->getNextIndex(ACTIVE, i);
    }

}


int main(int iArgC, char * apArgV[]) {
    int iResult=0;

    int iLayerSize = 4;
    int iArrSize = ARR_SIZE;

    LayerArrBuf<long> LAB(iLayerSize, iArrSize);
    printf("After creation: LAB has %zd layers (%zd used) of size %zd\n", LAB.getNumLayers(), LAB.getNumUsedLayers(), LAB.getLayerSize());
    LBController *pLBC = new LBController();
    pLBC->init(iLayerSize);
    printf("Adding buffer\n");
    pLBC->addBuffer(&LAB);
    pLBC->addLayer();
    printf("After init: %u layers: %d elements\n", pLBC->getNumLayers(), pLBC->getNumUsed());
    printf("After addLayer: LAB has %zd layers (%zd used) of size %zd\n", LAB.getNumLayers(), LAB.getNumUsedLayers(), LAB.getLayerSize());

    
    long j = pLBC->getFreeIndex();
    printf("inserting a0 at %ld\n", j);
    LAB.copyBlock(j, a0, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a1 at %ld\n", j);
    LAB.copyBlock(j, a1, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a2 at %ld\n", j);
    LAB.copyBlock(j, a2, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a3 at %ld\n", j);
    LAB.copyBlock(j, a3, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a4 at %ld\n", j);
    LAB.copyBlock(j, a4, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a5 at %ld\n", j);
    LAB.copyBlock(j, a5, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a6 at %ld\n", j);
    LAB.copyBlock(j, a6, 1);

    int iCap = LAB.size();
    printf("Have capacity %d\n", iCap);

    show(LAB, pLBC, iArrSize);

    printf("deleting element 2\n");
    pLBC->deleteElement(2);
    printf("deleting element 3\n");
    pLBC->deleteElement(3);
    printf("deleting element 5\n");
    pLBC->deleteElement(5);

    show(LAB, pLBC, iArrSize);
    
    j = pLBC->getFreeIndex();
    printf("inserting a7 at %ld\n", j);
    LAB.copyBlock(j, a7, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a8 at %ld\n", j);
    LAB.copyBlock(j, a8, 1);
    j = pLBC->getFreeIndex();
    printf("inserting a9 at %ld\n", j);
    LAB.copyBlock(j, a9, 1);

    show(LAB, pLBC, iArrSize);

    printf("compacting\n", j);
    pLBC->compactData();
    
    show(LAB, pLBC, iArrSize);
    
    printf("deleting element 2\n");
    pLBC->deleteElement(2);
    printf("deleting element 2\n");
    pLBC->deleteElement(4);
    printf("deleting element 2\n");
    pLBC->deleteElement(6);

    show(LAB, pLBC, iArrSize);

    int iNumNew = sizeof(b)/(sizeof(long)*iArrSize);
    printf("Reserving for %d\n", iNumNew);
    int iStart = pLBC->reserveSpace(iNumNew);
    show(LAB, pLBC, iArrSize);
    LAB.copyBlock(iStart, b, iNumNew);

    show(LAB, pLBC, iArrSize);

    return iResult;
}

