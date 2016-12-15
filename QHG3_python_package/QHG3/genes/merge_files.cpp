#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX_FILES 100000
#define LINE_SIZE 1024
#define ELEMENT_SIZE 3
#define BUFFER_ELEMENTS 32


int main(int argc, char *argv[]) {
    
    fprintf(stderr,"REMINDER: please sort each file individually before running %s\n",argv[0]);

    if (argc != 2) {
        fprintf(stderr,"please provide input file with file list\n");
        return -1;
    }

    char fileName[256];
    char* fileNames[MAX_FILES];
    FILE* FILES[MAX_FILES]; 
    int nElements[MAX_FILES];
    long int readElement[MAX_FILES * ELEMENT_SIZE];
    long int totElements = 0;

    FILE* FILELIST = fopen(argv[1],"r");

    int iFile = 0;
    int numFiles = 0;
	int iResult = 1;

    while ( fscanf(FILELIST, "%s", fileName) == 1 && numFiles < MAX_FILES && iResult > 0) {
        
        struct stat fileStat;
        
        if ( stat(fileName, &fileStat) == 0)  {
            
            long int fileSize = fileStat.st_size;
            nElements[iFile] = fileSize / ( ELEMENT_SIZE * sizeof(long int));
            totElements += nElements[iFile];
            
            fileNames[iFile] = new char[LINE_SIZE];
            
            strcpy(fileNames[iFile], fileName);
            
            FILES[iFile] = fopen(fileNames[iFile], "rb");
            
            //            fprintf(stderr,"opened %s at %p\n",fileNames[iFile],FILES[iFile]);

			iResult = fread(&readElement[iFile * ELEMENT_SIZE], sizeof(long int), ELEMENT_SIZE, FILES[iFile]);

            /*
            fprintf(stderr,"read %d %d %d from %s\n", 
                    readElement[iFile * ELEMENT_SIZE],
                    readElement[iFile * ELEMENT_SIZE + 1],
                    readElement[iFile * ELEMENT_SIZE + 2],
                    fileNames[iFile]);
            */

            iFile++;
            numFiles++;
        } else {
            fprintf(stderr,"some error occurred with %s\n", fileName);
        }
    }
    
    char outName[256];
    strcpy(outName, "ancestors_mergesort");
    FILE *OUT = fopen(outName,"wb");
    
    int count = 0;

    while (count < totElements) {
        
        long int iMin = readElement[0];
        iFile = 0;

        for (int i = 1; i < numFiles; i++) {
            if (readElement[i * ELEMENT_SIZE] < iMin) {
                iMin = readElement[i * ELEMENT_SIZE];
                iFile = i;
            }
        }
        
        fwrite(&readElement[iFile * ELEMENT_SIZE], sizeof(long int), ELEMENT_SIZE, OUT);

        nElements[iFile]--;

        if (nElements[iFile] > 0) {
            iResult = fread(&readElement[iFile * ELEMENT_SIZE], sizeof(long int), ELEMENT_SIZE, FILES[iFile]);            
        } else {
            readElement[iFile * ELEMENT_SIZE] = INT_MAX;
            readElement[iFile * ELEMENT_SIZE + 1] = INT_MAX;
            readElement[iFile * ELEMENT_SIZE + 2] = INT_MAX;
        }
        
        count++;
    }
      
    fclose(OUT);

    
    return 0;
}
