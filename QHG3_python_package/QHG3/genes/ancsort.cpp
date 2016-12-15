#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <map>

#include "ParamReader.h"
#include "BufWriter.h"
#include "AncFileBuffers.h"

#define DEF_CHUNK_SIZE (3*100000)
#define DEF_TEMP_NAME  "temp_ancsort"


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - sort an binary Anc file\n", pApp);
    printf("Anc files are binary files consisting of int triples\n");
    printf("  <curID> <momID> <dadID>\n");
    printf("Sorting of the files:\n");
    printf("- split file into chunks\n");
    printf("- load and sort the chunks\n");
    printf("- save chunk in temporary files\n");
    printf("- merge the temporary files to output file\n");
    printf("\n");
    printf("Usage:\n");
    printf("  %s -i <inputAnc> -o <outputAnc> \n", pApp);
    printf("     [-s <chunkSize>] [-t <tempFileName>] [-k]\n");
    printf("where\n");
    printf("  inputAnc      input anc file\n");
    printf("  outputAnc     output anc file\n");
    printf("  chunkSize     size of chunks (default:%d)\n", DEF_CHUNK_SIZE);
    printf("  tempFileName  name template for temporary files (default: %s)\n", DEF_TEMP_NAME);
    printf("  -k            don't delete temporary files\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// sortAndWrite
//
int sortAndWrite(long *pBuf, size_t iRead, int iCount, char *pTempFile) {
    int iResult = -1;
    std::map<long, std::pair<long, long> > mAncData;
    for (uint i = 0; i < iRead; i++) {
        mAncData[pBuf[3*i]] = std::pair<long,long>(pBuf[3*i+1], pBuf[3*i+2]);
    }
    if (iRead == mAncData.size()) {
        char sFileName[256];
        sprintf(sFileName, "%s_%06d", pTempFile, iCount);
        FILE *fOut = fopen(sFileName, "wb");
        if (fOut != NULL) {
            int i = 0;
            std::map<long, std::pair<long, long> >::const_iterator it;
            for (it = mAncData.begin(); it != mAncData.end(); ++it) {
                pBuf[i]   = it->first;
                pBuf[i+1] = it->second.first;
                pBuf[i+2] = it->second.second;
            //    printf("  %d", pBuf[i]);
		i+=3;
            }
            size_t iWritten = fwrite(pBuf, 3*sizeof(long), iRead, fOut);
            if (iWritten == iRead) {
                iResult = 0;
            }
            fclose(fOut);
        } else {
            printf("Couldn't open [%s] for writing\n", sFileName);
        }
    } else {
        printf("map has wrong number of entries (%zd instead of %zd) - duplicate ids?\n", mAncData.size(), iRead);
    }
            
    return iResult;
}

//----------------------------------------------------------------------------
// createSortedChunks
//
int createSortedChunks(const char*pInput, long *piChunkSize, char *pTempFile) {
    int iResult = -1;
    long iNumFiles = -1;
    FILE *fIn = fopen(pInput, "rb");
    if (fIn != NULL) {
        fseek(fIn, 0, SEEK_END);
        long lSize = ftell(fIn);
        fseek(fIn, 0, SEEK_SET);
	ldiv_t q = ldiv(lSize, 3*sizeof(long)*(*piChunkSize));
        printf("div(%ld,%zd)= %ld r%ld\n", lSize,  3*sizeof(long)*(*piChunkSize), q.quot, q.rem);
        if (q.quot < 511) {
            iNumFiles = q.quot;
            if (q.rem > 0) {
                iNumFiles++;
            }
        } else {
            iNumFiles = 512;
            long iCS = 1+lSize/(3*sizeof(long)*511);
            ldiv_t q1 = ldiv(lSize, 3*sizeof(long)*iCS);
            printf("div(%ld,%zd)= %ld r%ld\n", lSize,  3*sizeof(long)*iCS, q1.quot, q1.rem);
            *piChunkSize = iCS;
            iNumFiles = q1.quot;
            if (q1.rem > 0) {
                iNumFiles++;
            }
        }
        long *pBuf = new long[3*(*piChunkSize)];

        printf("Using %ld files; chunksize %ld\n", iNumFiles, *piChunkSize);
        int iC = 0;
        size_t iTotBytes = 0;
        iResult = 0;
        while ((iResult == 0) && !feof(fIn)) {
            printf("\r                                                       \rreading chunk %d/%ld", iC, iNumFiles);fflush(stdout);
            size_t iRead = fread(pBuf, 3*sizeof(long), *piChunkSize, fIn);
            if (iRead > 0) {
                iResult = sortAndWrite(pBuf, iRead, iC, pTempFile);
                iC++;
                iTotBytes += 3*sizeof(long)*iRead;
            }
        }
        printf("\n");
        printf("Wrote %zd bytes to %d files\n", iTotBytes, iC);
        fclose(fIn);
        delete[] pBuf;
    } else {
        printf("Couldn't open anc file [%s]\n", pInput);
    }

    if (iResult == 0) {
        iResult = (int)iNumFiles;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// mergeChunkFiles
//
int mergeChunkFiles(char *pTempFile, int iCount, long iBufSize, const char *pOutput) {
    int iResult = -1;
    
    // create BufWriter for output
    BufWriter *pBW = BufWriter::createInstance(pOutput);
    if (pBW != NULL) {

        AncFileBuffers *pAFB = AncFileBuffers::createInstance(pTempFile, iCount, (int)iBufSize);
        if (pAFB != NULL) {
	    int iC = 0;
            iResult = 0;
            idtype *p = pAFB->getSmallestRecord();
            while (p != NULL) {
	    //printf("(%d %d %d)", p[0], p[1], p[2]);
                pBW->addChars((char *)p, 3*sizeof(long));
                p = pAFB->getSmallestRecord();
		iC += 3;
            }
            delete  pAFB;
        } else {
            printf("Couldn't create AncFileBuffers\n");
        }
        // delete BufWriter
        delete pBW;
    } else {
        printf("Couldn't open [%s] for writing\n", pOutput);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// cleanup
//
int cleanup(char *pTempFile, int iCount) {
    int iResult = 0;
    char sFileName[256];
    for (int i =0; i < iCount; ++i) {
        sprintf(sFileName, "%s_%06d", pTempFile, i);
        int iResult1 = remove(sFileName);
        if (iResult1 != 0) {
            printf("Error removing [%s]\n", sFileName);
            iResult =-1;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main (int iArgC, char *apArgV[]) {
    int iResult = 0;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char *sInput  = NULL;
        char *sOutput = NULL;
        long iChunkSize = DEF_CHUNK_SIZE;
        char sTempFileName[256];
        *sTempFileName = '\0';
        bool bKeepTemp = false;

        bool bOK = pPR->setOptions(5,
                                   "-i:S!",   &sInput,
                                   "-o:S!",   &sOutput,
                                   "-s:l",    &iChunkSize,
                                   "-t:s",     sTempFileName,
                                   "-k:0",    &bKeepTemp);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                // open file; read chunks; sort chunks; save them
                int iNumChunks = createSortedChunks(sInput, &iChunkSize, sTempFileName);
                
                if (iNumChunks > 0) {
                   
                    long iBufSize = 3*(iChunkSize/3);
                    // open all chunk files, load buffer
                    iResult = mergeChunkFiles(sTempFileName, iNumChunks, iBufSize,  sOutput);
                }

                if (!bKeepTemp) {
                    cleanup(sTempFileName, iNumChunks);
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        printf("Couldn't create ParamReader\n");
    }
    return iResult;
}
