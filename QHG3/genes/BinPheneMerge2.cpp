#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "types.h"
#include "strutils.h"
#include "BinPheneFile.h"
#include "ParamReader.h"


typedef std::vector<std::string>    stringvec;
typedef std::vector<BinPheneFile *> bpfvec;

typedef struct sHeaderData {
    char sMagic[4];
    int iPhenomeSize;
    int iNumPhenomes;
    int iNumLocs;
    bool bFull;
} sHeaderData;

//----------------------------------------------------------------------------
// usage
//   
void usage(const char *pApp) {
    printf("%s - merge binary phenome files\n", pApp);
    printf("Usage;\n");
    printf("  %s <outputname> <binphenefile>*\n", pApp);
    printf("where\n");
    printf("  outputname         name of output file\n");
    printf("  binphenefile       phenome file in BIN format\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// getHeader
//   
int getHeader(const char *pFile, sHeaderData *pHD) {
    int iResult = -1;
    FILE *fIn = fopen(pFile, "rt");
    if (fIn != NULL) {
        char sMagic[5];
        int iRead = fread(sMagic, 4, 1, fIn);
        if (iRead == 1) {
            if (memcmp(sMagic, "PHNY", 4) == 0) {
                memcpy(pHD->sMagic, sMagic, 5);

                int iHeaderLen = 3*sizeof(int)+sizeof(bool);
                char *pH = new char[iHeaderLen];
                iRead = fread(pH, iHeaderLen, 1, fIn);
             
                if (iRead == 1) {
                    char *p = pH;
                    p = getMem(&pHD->iPhenomeSize, p, sizeof(int));
                    p = getMem(&pHD->iNumPhenomes, p, sizeof(int));
                    p = getMem(&pHD->iNumLocs,    p, sizeof(int));
                    p = getMem(&pHD->bFull,       p, sizeof(bool));
                    iResult = 0;
             
                } else {
                    fprintf(stderr, "Couldn't read header\n");
                    iResult = -1;
                }
                delete[] pH;

            } else {
                fprintf(stderr, "Uknown magic number [%s]\n", sMagic);
            }
        } else {
            fprintf(stderr, "Couldn't read magic number\n");
            iResult = -1;
        }
        fclose(fIn);

    } else {
        fprintf(stderr, "Couldn't open [%s] for reading\n", pFile);
        iResult = -1;
    }
    return iResult;
} 

                 
//----------------------------------------------------------------------------
// checkHeaders
//   
sHeaderData *checkHeaders(bpfvec &vBinPheneFiles) {
    int iResult = 0;
    sHeaderData *pHD = NULL;
    std::vector<sHeaderData> vHeaders;
    for (uint i = 0; (iResult == 0) && (i < vBinPheneFiles.size()); i++) {
        if (vBinPheneFiles[i] != NULL) {
            sHeaderData sHD;
            memcpy(sHD.sMagic, vBinPheneFiles[i]->getMagic(), 4);
            sHD.iPhenomeSize = vBinPheneFiles[i]->getPhenomeSize();
            sHD.iNumPhenomes = vBinPheneFiles[i]->getNumPhenomes();
            sHD.iNumLocs     = vBinPheneFiles[i]->getNumLocs();
            sHD.bFull        = vBinPheneFiles[i]->getFull();
            vHeaders.push_back(sHD);
        } else {
            // a BinPheneFile couldn't be created
            iResult = -1;
        }
    }
    if (iResult == 0) {
        int iNumPhenomesTotal = vHeaders[0].iNumPhenomes;
        int iNumLocsTotal     = vHeaders[0].iNumLocs;
        // compare headers
        for (uint i = 1; (iResult == 0) && (i < vHeaders.size()); i++) {
            if ((memcmp(vHeaders[0].sMagic, vHeaders[i].sMagic, 4) != 0) ||
                (vHeaders[0].iPhenomeSize != vHeaders[i].iPhenomeSize) ||
                (vHeaders[0].bFull       != vHeaders[i].bFull)) {
                iResult = -1;
            } else {
                iNumPhenomesTotal += vHeaders[i].iNumPhenomes;
                iNumLocsTotal     += vHeaders[i].iNumLocs;
            }
        }
        if (iResult == 0) {
            // create output header
            pHD = new sHeaderData;
            memcpy(pHD->sMagic,  vHeaders[0].sMagic, 4);
            pHD->iPhenomeSize = vHeaders[0].iPhenomeSize;
            pHD->bFull        = vHeaders[0].bFull;
            pHD->iNumPhenomes = iNumPhenomesTotal;
            pHD->iNumLocs     = iNumLocsTotal;
            char sMagic[5];
            memcpy(sMagic, pHD->sMagic, 4);
            sMagic[4] = '\0';
            printf("Resultin header:\n");
            printf("  Magic:       %s\n", sMagic);
            printf("  PhenomeSize: %d\n", pHD->iPhenomeSize);
            printf("  NumGenes:    %d\n", pHD->iNumPhenomes);
            printf("  NumLocs:     %d\n", pHD->iNumLocs);
            printf("  bFull:       %s\n", pHD->bFull?"yes":"no");
        }
    }
    return pHD;
}


//----------------------------------------------------------------------------
// appendData
//   we assume fIn is at the correct position (i.e. just after thee header)
//
int appendData(FILE *fOut, FILE *fIn) {
    int iResult = 0;
    uchar pBuf[16384];
    int iBufLen = 16384;
    int iRead = 16384;
    while ((iResult == 0) && (iBufLen == iRead)) {
        iRead = fread(pBuf, 1, iBufLen, fIn);
        if (iRead > 0) {
            int iWrite = fwrite(pBuf, 1, iRead, fOut);
            if (iWrite != iRead) {
                iResult = -1;
            }
        }
    }
    return iResult;
}    


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;


    if (iArgC > 2) {
        char *pOutputFile = apArgV[1];
        bpfvec vBinPheneFiles;

        printf("Have %d files\n", iArgC - 3);

        for (int i = 2; i < iArgC; i++) {
            vBinPheneFiles.push_back(BinPheneFile::createInstance(apArgV[i]));
        }
        iResult = 0;

        sHeaderData *pHD = checkHeaders(vBinPheneFiles);
        
        if (pHD != NULL) {

            memcpy(pHD->sMagic, "PHNY", 4);
            FILE *fOut = fopen(pOutputFile, "wb");
            if (fOut != NULL) {
                int iLenH =  4*sizeof(char)+3*sizeof(int)+sizeof(bool);
                char *pLineH = new char[iLenH];
                // write file header;
                char *p0 = pLineH;
                p0 = putMem(p0, pHD->sMagic, 4*sizeof(char));
                p0 = putMem(p0, &pHD->iPhenomeSize, sizeof(int));
                p0 = putMem(p0, &pHD->iNumPhenomes, sizeof(int));
                p0 = putMem(p0, &pHD->iNumLocs, sizeof(int));
                p0 = putMem(p0, &pHD->bFull, sizeof(bool));
                size_t iWritten = fwrite(pLineH, iLenH, 1, fOut);
                delete[] pLineH;
                if (iWritten == 1) {
                    for (uint i = 0; (iResult == 0) && (i < vBinPheneFiles.size()); i++) {
                        printf("Appending [%s]\n",  vBinPheneFiles[i]->getName());
                        iResult = appendData(fOut, vBinPheneFiles[i]->getFileHandle());
                    }
                } else {
                    fprintf(stderr, "Error writing to [%s]: written %zd\n", pOutputFile, iWritten);
                    iResult = -1;
                }
                fclose(fOut);
            } else {
                fprintf(stderr, "Couldn't open [%s] for writing\n", pOutputFile);
                iResult = -1;
            }
            delete pHD;
        } else {
            fprintf(stderr, "Non-compatible headers");
            iResult = -1;
        }
        
        for (uint i = 0; (iResult == 0) && (i < vBinPheneFiles.size()); i++) {
            delete vBinPheneFiles[i];
        }
    } else {
        usage(apArgV[0]);
    }
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}


