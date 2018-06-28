#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define LINE_SIZE 1024
#define ELEMENT_SIZE 3


void merge_sort(long int* pA, long int* pB, int nA, int nB, long int* pOut) {
    

    if (nA > 1) {
        int nX = nA/2;
        int nY = nA - nX;
        long int* pX = pA;
        long int* pY = pA + nX * ELEMENT_SIZE;
        long int* pZ = new long int[nA * ELEMENT_SIZE];
        
        //        fprintf(stderr,"merge_sort(%p, %p, %d, %d, %p);\n",pX,pY,nX,nY,pZ);

        merge_sort(pX, pY, nX, nY, pZ);
        
        delete pZ;
    }
    
    if (nB > 1) {
        int nX = nB/2;
        int nY = nB - nX;
        long int* pX = pB;
        long int* pY = pB + nX * ELEMENT_SIZE;
        long int* pZ = new long int[nB * ELEMENT_SIZE];
        
        //        fprintf(stderr,"merge_sort(%p, %p, %d, %d, %p);\n",pX,pY,nX,nY,pZ);

        merge_sort(pX, pY, nX, nY, pZ);
        
        delete pZ;
    }
    
    int i = 0;
    int j = 0;
    int k = 0;
    
    while (i < nA && j < nB) {
        
        if (pA[i * ELEMENT_SIZE] <= pB[j * ELEMENT_SIZE]) {
            
            memcpy(pOut + k * ELEMENT_SIZE, pA + i * ELEMENT_SIZE, sizeof(long int) * ELEMENT_SIZE);
            i++;
            k++;
            
        } else {

            memcpy(pOut + k * ELEMENT_SIZE, pB + j * ELEMENT_SIZE, sizeof(long int) * ELEMENT_SIZE);
            j++;
            k++;
        }
    }
    
    if (i == nA) {
        memcpy(pOut + k * ELEMENT_SIZE, pB + j * ELEMENT_SIZE, sizeof(long int) * ELEMENT_SIZE * (nB - j));
    } else {
        memcpy(pOut + k * ELEMENT_SIZE, pA + i * ELEMENT_SIZE, sizeof(long int) * ELEMENT_SIZE * (nA - i));
    }
    
    // since pA and pB are always contiguous, I can safely copy the results back to pA

    memcpy(pA, pOut, sizeof(long int) * ELEMENT_SIZE * (nA + nB));
    
}


int main(int argc, char *argv[]) {
    
    char fileName[256];
    struct stat fileStat;
    
    if (argc != 2) {
        fprintf(stderr,"please provide input file name\n");
        return -1;
    }

    strcpy(fileName, argv[1]);

    if ( stat(fileName, &fileStat) == 0)  {

        long int fileSize = fileStat.st_size;

        FILE *IN = fopen(fileName,"rb");

        fprintf(stderr, "sorting %s of size %ld\n", fileName, fileSize);
            
        int nElements = fileSize / ( ELEMENT_SIZE * sizeof(long int));
        fprintf(stderr, "file contains %d entries\n", nElements);

        // read elements
            
        long int* pIn = new long int[nElements * ELEMENT_SIZE];
        long int* pOut = new long int[nElements * ELEMENT_SIZE];
        long int* pA = pIn;

        while( fread(pA, sizeof(long int), ELEMENT_SIZE, IN) == ELEMENT_SIZE ) {
            pA += ELEMENT_SIZE;
        }

        fclose(IN);

        // check that everything's OK

        if ( (pA - pIn) != ELEMENT_SIZE * nElements ) {

            fprintf(stderr, "expected %d integers, got %ld\n", ELEMENT_SIZE * nElements, pA - pIn);

        } else {

            int nA = nElements/2; // this may have 1 extra element
            int nB = nElements - nA; 
                
            merge_sort(pIn, pIn + nA * ELEMENT_SIZE, nA, nB, pOut);
                    
        }
            
        char outName[256];
        strcpy(outName, fileName);
        strcat(outName, "_sort");
        FILE *OUT = fopen(outName,"wb");

        // sorted array is stored in pIn, not pOut
        fwrite(pIn, sizeof(long int), ELEMENT_SIZE * nElements, OUT);
        
        fclose(OUT);

        delete pIn;
        delete pOut;
            
    }
    
    return 0;
}
