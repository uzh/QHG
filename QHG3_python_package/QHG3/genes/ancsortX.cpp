#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/resource.h>

#include <omp.h>

#include <map>

#include "ParamReader.h"
#include "BufWriter.h"
#include "AncFileBuffers.h"
#include "AncMergeSorter.h"

#define DEF_CHUNK_SIZE (100000)
#define DEF_TEMP_NAME  "__temp_ancsortX"


double dTotSaW = 0;
double dTotGsR = 0;
double dInit   = 0;

//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - sort an binary Anc file\n", pApp);
    printf("Anc files are binary files consisting of long tuples\n");
    printf("  <curID> <momID> <dadID> ...\n");
    printf("Sorting of the files:\n");
    printf("- split file into chunks\n");
    printf("- load and sort the chunks\n");
    printf("- save chunk in temporary files\n");
    printf("- merge the temporary files to output file\n");
    printf("\n");
    printf("Usage:\n");
    printf("  %s -i <inputAnc> -o <outputAnc> -a <ancSize> \n", pApp);
    printf("     [-s <chunkSize>] [-t <tempFileName>] [-k]\n");
    printf("where\n");
    printf("  inputAnc      input anc file\n");
    printf("  outputAnc     output anc file\n");
    printf("  ancSize       size of anc record (3 or 4)\n");
    printf("  chunkSize     size of chunks (default:%d)\n", DEF_CHUNK_SIZE);
    printf("  tempFileName  name template for temporary files (default: %s)\n", DEF_TEMP_NAME);
    printf("  -k            don't delete temporary files\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// sortAndWrite
//
int sortAndWrite(long *pBuf, size_t iRead, int iCount, char *pTempFile, int iAncSize) {
    int iResult = -1;

    // read data into map to have it sorted
    std::map<long, long*> mAncData;
    for (uint i = 0; i < iRead; i++) {
        mAncData[pBuf[iAncSize*i]] = pBuf+iAncSize*i;
    }

    if (iRead == mAncData.size()) {
        char sFileName[256];
        sprintf(sFileName, "%s_%06d", pTempFile, iCount);
        FILE *fOut = fopen(sFileName, "wb");
        if (fOut != NULL) {
    
            long *pOut = new long[iRead*iAncSize];
            int i = 0;
            
            std::map<long,  long*>::const_iterator it;
            for (it = mAncData.begin(); it != mAncData.end(); ++it) {
                memcpy(pOut+i, it->second, iAncSize*sizeof(long));
		i += iAncSize;
            }
            size_t iWritten = fwrite(pOut, iAncSize*sizeof(long), iRead, fOut);
    
            if (iWritten == iRead) {
                //                printf("Wrote %zd records to %s\n", iWritten, sFileName);
                iResult = 0;
            } else {
                printf("Write size (%zd) does not correspond to read size (%zd)\n", iWritten, iRead);
                pOut = NULL;
            }
            fclose(fOut);
            delete[] pOut;
        } else {
            printf("Couldn't open [%s] for writing\n", sFileName);
        }
    } else {
        printf("map has wrong number of entries (%zd instead of %zd) - duplicate ids?\n", mAncData.size(), iRead);
        printf("the reason for this could be that the anc files and population files were created in different runs\n");
    }
    
    return iResult;
}
//----------------------------------------------------------------------------
// sortAndWriteAMS
//
int sortAndWriteAMS(long *pBuf, size_t iRead, int iCount, char *pTempFile, int iAncSize) {
    int iResult = -1;

    AncMergeSorter *pAMS = AncMergeSorter::createInstance(pBuf, iRead, iAncSize);
    if (pAMS != NULL) {
        pAMS->start();
        delete pAMS;
    }
    // pBuf should now be sorted
    // do duplicate check
    bool bOK = true;
    for (uint i = 1; (i < iRead) &&  (pBuf[i-1] != pBuf[i]); i++) {}
    if (bOK) {
        char sFileName[256];
        sprintf(sFileName, "%s_%06d", pTempFile, iCount);
        FILE *fOut = fopen(sFileName, "wb");
        if (fOut != NULL) {
            size_t iWritten = fwrite(pBuf, iAncSize*sizeof(long), iRead, fOut);

            if (iWritten == iRead) {
                //                printf("Wrote %zd records to %s\n", iWritten, sFileName);
                iResult = 0;
            } else {
                printf("Write size (%zd) does not correspond to read size (%zd)\n", iWritten, iRead);
            }
            fclose(fOut);
        } else {
            printf("Couldn't open [%s] for writing\n", sFileName);
        }
        
    } else {
        printf("duplicate ids detected!\n");
        printf("the reason for this could be that the anc files and population files were created in different runs\n");
    }
        
    return iResult;
}


//----------------------------------------------------------------------------
// createSortedChunks
//
int createSortedChunks(const char*pInput, long *piChunkSize, char *pTempFile, int iAncSize, int iMaxFile) {
    int iResult = -1;
    long iNumFiles = -1;
    FILE *fIn = fopen(pInput, "rb");
    if (fIn != NULL) {
        fseek(fIn, 0, SEEK_END);
        long lSize = ftell(fIn);
        fseek(fIn, 0, SEEK_SET);
	ldiv_t q = ldiv(lSize, iAncSize*sizeof(long)*(*piChunkSize));
        printf("div(%ld,%zd)= %ld r%ld\n", lSize,  iAncSize*sizeof(long)*(*piChunkSize), q.quot, q.rem);
        if (q.quot < iMaxFile) {
            iNumFiles = q.quot;
            if (q.rem > 0) {
                iNumFiles++;
            }
        } else {
            iNumFiles = iMaxFile-1;
            long iCS = 1+lSize/(iAncSize*sizeof(long)*(iMaxFile-1));
            ldiv_t q1 = ldiv(lSize, iAncSize*sizeof(long)*iCS);
            printf("div(%ld,%zd)= %ld r%ld\n", lSize,  iAncSize*sizeof(long)*iCS, q1.quot, q1.rem);
            *piChunkSize = iCS;
            iNumFiles = q1.quot;
            if (q1.rem > 0) {
                iNumFiles++;
            }
        }
        long *pBuf = new long[iAncSize*(*piChunkSize)];
        printf("Using %ld files; chunksize %ld\n", iNumFiles, *piChunkSize);
        int iC = 0;
        size_t iTotBytes = 0;
        iResult = 0;
        while ((iResult == 0) && !feof(fIn)) {
            printf("\r                                                       \rreading chunk %d/%ld", iC, iNumFiles);fflush(stdout);
            size_t iRead = fread(pBuf, iAncSize*sizeof(long), *piChunkSize, fIn);
            if (iRead > 0) {
                double dT = omp_get_wtime();
                iResult = sortAndWrite(pBuf, iRead, iC, pTempFile, iAncSize);
                dTotSaW +=  omp_get_wtime()-dT;
                iC++;
                iTotBytes += iAncSize*sizeof(long)*iRead;
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

    printf("sortAndWrite: %f\n", dTotSaW);
    return iResult;
}


//----------------------------------------------------------------------------
// createSortedChunks2
//
int createSortedChunks2(const char*pInput, long *piChunkSize, char *pTempFile, int iAncSize, int iMaxFiles) {
    int iResult = -1;
    long iNumFiles = -1;
    long lSize = -1;
    FILE *fIn = fopen(pInput, "rb");
    if (fIn != NULL) {
        iResult = 0;
        fseek(fIn, 0, SEEK_END);
        lSize = ftell(fIn);
        fseek(fIn, 0, SEEK_SET);
	ldiv_t q = ldiv(lSize, iAncSize*sizeof(long)*(*piChunkSize));
        printf("div(%ld,%zd)= %ld r%ld\n", lSize,  iAncSize*sizeof(long)*(*piChunkSize), q.quot, q.rem);
        if (q.quot < iMaxFiles) {
            iNumFiles = q.quot;
            if (q.rem > 0) {
                iNumFiles++;
            }
        } else {
            iNumFiles = iMaxFiles-1;
            long iCS = 1+lSize/(iAncSize*sizeof(long)*(iMaxFiles-1));
            printf("file limit exceeded, setting chink size to %ld\n", iCS);
            ldiv_t q1 = ldiv(lSize, iAncSize*sizeof(long)*iCS);
            printf("div(%ld,%zd)= %ld r%ld\n", lSize,  iAncSize*sizeof(long)*iCS, q1.quot, q1.rem);
            *piChunkSize = iCS;
            iNumFiles = q1.quot;
            if (q1.rem > 0) {
                iNumFiles++;
            }
        }
        fclose(fIn);
    }

    if (iResult == 0) {
        long **pBuf = new long*[omp_get_max_threads()];
        FILE **pfIn = new FILE*[omp_get_max_threads()];
        int   *pC   = new int[omp_get_max_threads()];

#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            pBuf[iT] = new long[iAncSize*(*piChunkSize)];
            memset(pBuf[iT], 0, iAncSize*(*piChunkSize)*sizeof(long));
            pfIn[iT] = fopen(pInput, "rb");
            pC[iT] = 0;
        }

        size_t iTotBytes = 0;
#pragma omp parallel for schedule(dynamic) reduction(+:iTotBytes)
        for (long iPos = 0; iPos < lSize; iPos += iAncSize*(*piChunkSize)*sizeof(long)) {
            int iT = omp_get_thread_num();
            int iIndex = iPos/(iAncSize*(*piChunkSize)*sizeof(long));
            fseek(pfIn[iT], iPos, SEEK_SET);
            size_t iRead = fread(pBuf[iT], iAncSize*sizeof(long), *piChunkSize, pfIn[iT]);
            if (iRead > 0) {
                iResult = sortAndWriteAMS(pBuf[iT], iRead, iIndex, pTempFile, iAncSize);
                iTotBytes += iAncSize*sizeof(long)*iRead;
                pC[iT]++;
            }
        }
        
        int iC = 0;
            for (int i = 0; i < omp_get_max_threads(); i++) {
            iC += pC[i];
        }

        printf("Wrote %zd bytes to %d files\n", iTotBytes, iC);

#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            delete[] pBuf[iT];
            fclose(pfIn[iT]);
                
        }

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

int mergeChunkFiles(char *pTempFile, int iCount, long iBufSize, const char *pOutput, int iAncSize) {
    int iResult = -1;
    double ddd = 0;
    // create BufWriter for output
    BufWriter *pBW = BufWriter::createInstance(pOutput);
    if (pBW != NULL) {
        ddd = omp_get_wtime();
        AncFileBuffers *pAFB = AncFileBuffers::createInstance(pTempFile, iCount, (int)iBufSize, iAncSize);
        dInit += omp_get_wtime()-ddd;
        if (pAFB != NULL) {
            printf("AncFileBuffers initialized\n");
	    int iC = 0;
            iResult = 0;
            ddd = omp_get_wtime();
            const idtype *p = pAFB->getSmallestRecordOld();
            dTotGsR += omp_get_wtime()-ddd;
            while (p != NULL) {
	    //printf("(%d %d %d)", p[0], p[1], p[2]);
                pBW->addChars((char *)p, iAncSize*sizeof(long));
                ddd = omp_get_wtime();
                p = pAFB->getSmallestRecordOld();
                dTotGsR += omp_get_wtime()-ddd;
                iC += iAncSize;
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
    int iResult = -1;
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char *sInput  = NULL;
        char *sOutput = NULL;
        int iAncSize  = 0;
        long iChunkSize = DEF_CHUNK_SIZE;
        char sTempFileName[256];
        strcpy(sTempFileName, DEF_TEMP_NAME);
        bool bKeepTemp = false;

        bool bOK = pPR->setOptions(6,
                                   "-i:S!",   &sInput,
                                   "-o:S!",   &sOutput,
                                   "-a:i!",   &iAncSize,
                                   "-s:l",    &iChunkSize,
                                   "-t:s",     sTempFileName,
                                   "-k:0",    &bKeepTemp);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (iAncSize > 0) {
                    struct rlimit rl;
                    iResult = getrlimit(RLIMIT_NOFILE, &rl);
                    

                    double d0 = omp_get_wtime();

                    iChunkSize *= iAncSize;
                    // open file; read chunks; sort chunks; save them
                    double T0 = omp_get_wtime();
                    int iNumChunks = createSortedChunks2(sInput, &iChunkSize, sTempFileName, iAncSize, rl.rlim_cur-10); // 10: stdin,stdout,stderr,... (to be sure)
                    double T1 = omp_get_wtime()-T0;
                    double T2 = 0;
                    double T3 = 0;
                
                    if (iNumChunks > 0) {
                        long iBufSize = iAncSize*(iChunkSize/iAncSize);
                        // open all chunk files, load buffer
                        T0 = omp_get_wtime();
                        iResult = mergeChunkFiles(sTempFileName, iNumChunks, iBufSize,  sOutput, iAncSize);
                        T2 = omp_get_wtime()-T0;
                    }

                    if (!bKeepTemp) {
                        T0 = omp_get_wtime();
                        cleanup(sTempFileName, iNumChunks);
                        T3 = omp_get_wtime()-T0;
                    }

                    printf("createSortedChunks:  %f\n", T1);
                    printf("  sortAndWrite:      %f\n", dTotSaW);
                    printf("mergeChunkFiles:     %f\n", T2);
                    printf("  init:              %f\n", dInit);
                    printf("  getSmallestRecord: %f\n", dTotGsR);
                    printf("cleanup:             %f\n", T3);
                    printf("Total:              %f\n", omp_get_wtime()-d0);
                } else {
                    iResult = -1;
                    printf("AncSize must be greater than 0\n");
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
