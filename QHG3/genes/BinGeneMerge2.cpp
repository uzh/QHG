#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "types.h"
#include "strutils.h"
#include "BinGeneFile.h"
#include "ParamReader.h"

#define DEF_BITS_PER_NUC 2

typedef std::vector<std::string> stringvec;
typedef std::vector<BinGeneFile *> bgfvec;

typedef struct sHeaderData {
    char sMagic[4];
    int iGenomeSize;
    int iNumGenomes;
    int iNumLocs;
    uint iBitsPerNuc;
    bool bFull;
} sHeaderData;

//----------------------------------------------------------------------------
// usage
//   
void usage(const char *pApp) {
    printf("%s - merge binary gene files\n", pApp);
    printf("Usage;\n");
    printf("  %s <outputname> <bingenefile>*\n", pApp);
    printf("where\n");
    printf("  outputname         name of output file\n");
    printf("  bingenefile        genome file in BIN format\n");
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
            if ((memcmp(sMagic, "GENS", 4) == 0)  ||
                (memcmp(sMagic, "GENX", 4) == 0)  ||
                (memcmp(sMagic, "GENY", 4) == 0)) {
                memcpy(pHD->sMagic, sMagic, 5);

                bool bExtended2 = false;
            
                if (memcmp(sMagic, "GENY", 4) == 0) {
                    bExtended2 = true;
                } else {
                    bExtended2 = false;
                }
                int iHeaderLen = (bExtended2?4:3)*sizeof(int)+sizeof(bool);
                char *pH = new char[iHeaderLen];
                iRead = fread(pH, iHeaderLen, 1, fIn);
             
                if (iRead == 1) {
                    char *p = pH;
                    p = getMem(&pHD->iGenomeSize, p, sizeof(int));
                    p = getMem(&pHD->iNumGenomes, p, sizeof(int));
                    p = getMem(&pHD->iNumLocs,    p, sizeof(int));
                    if (bExtended2) {
                        p = getMem(&pHD->iBitsPerNuc,    p, sizeof(int));
                    } else {
                        pHD->iBitsPerNuc = DEF_BITS_PER_NUC;
                    }
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
sHeaderData *checkHeaders(bgfvec &vBinGeneFiles) {
    int iResult = 0;
    sHeaderData *pHD = NULL;
    std::vector<sHeaderData> vHeaders;
    for (uint i = 0; (iResult == 0) && (i < vBinGeneFiles.size()); i++) {
        if (vBinGeneFiles[i] != NULL) {
            sHeaderData sHD;
            memcpy(sHD.sMagic, vBinGeneFiles[i]->getMagic(), 4);
            sHD.iGenomeSize = vBinGeneFiles[i]->getGenomeSize();
            sHD.iNumGenomes = vBinGeneFiles[i]->getNumGenomes();
            sHD.iNumLocs    = vBinGeneFiles[i]->getNumLocs();
            sHD.iBitsPerNuc = vBinGeneFiles[i]->getBitsPerNuc();
            sHD.bFull       = vBinGeneFiles[i]->getFull();
            vHeaders.push_back(sHD);
        } else {
            // a BinGeneFile couldn't be created
            iResult = -1;
        }
    }
    if (iResult == 0) {
        int iNumGenomesTotal = vHeaders[0].iNumGenomes;
        int iNumLocsTotal    = vHeaders[0].iNumLocs;
        // compare headers
        for (uint i = 1; (iResult == 0) && (i < vHeaders.size()); i++) {
            if ((memcmp(vHeaders[0].sMagic, vHeaders[i].sMagic, 4) != 0) ||
                (vHeaders[0].iGenomeSize != vHeaders[i].iGenomeSize) ||
                (vHeaders[0].iBitsPerNuc != vHeaders[i].iBitsPerNuc) ||
                (vHeaders[0].bFull       != vHeaders[i].bFull)) {
                iResult = -1;
            } else {
                iNumGenomesTotal += vHeaders[i].iNumGenomes;
                iNumLocsTotal    += vHeaders[i].iNumLocs;
            }
        }
        if (iResult == 0) {
            // create output header
            pHD = new sHeaderData;
            memcpy(pHD->sMagic,  vHeaders[0].sMagic, 4);
            pHD->iGenomeSize = vHeaders[0].iGenomeSize;
            pHD->iBitsPerNuc = vHeaders[0].iBitsPerNuc;
            pHD->bFull       = vHeaders[0].bFull;
            pHD->iNumGenomes = iNumGenomesTotal;
            pHD->iNumLocs    = iNumLocsTotal;
            char sMagic[5];
            memcpy(sMagic, pHD->sMagic, 4);
            sMagic[4] = '\0';
            printf("Resultin header:\n");
            printf("  Magic:      %s\n", sMagic);
            printf("  GenomeSize: %d\n", pHD->iGenomeSize);
            printf("  NumGenes:   %d\n", pHD->iNumGenomes);
            printf("  NumLocs:    %d\n", pHD->iNumLocs);
            printf("  BitsPerNuc: %d\n", pHD->iBitsPerNuc);
            printf("  bFull:      %s\n", pHD->bFull?"yes":"no");
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
        bgfvec vBinGeneFiles;

        printf("Have %d files\n", iArgC - 3);

        for (int i = 2; i < iArgC; i++) {
            vBinGeneFiles.push_back(BinGeneFile::createInstance(apArgV[i]));
        }
        iResult = 0;

        sHeaderData *pHD = checkHeaders(vBinGeneFiles);
        
        if (pHD != NULL) {

            memcpy(pHD->sMagic, "GENY", 4);
            FILE *fOut = fopen(pOutputFile, "wb");
            if (fOut != NULL) {
                int iLenH =  4*sizeof(char)+4*sizeof(int)+sizeof(bool);
                char *pLineH = new char[iLenH];
                // write file header;
                char *p0 = pLineH;
                p0 = putMem(p0, pHD->sMagic, 4*sizeof(char));
                p0 = putMem(p0, &pHD->iGenomeSize, sizeof(int));
                p0 = putMem(p0, &pHD->iNumGenomes, sizeof(int));
                p0 = putMem(p0, &pHD->iNumLocs, sizeof(int));
                p0 = putMem(p0, &pHD->iBitsPerNuc, sizeof(int));
                p0 = putMem(p0, &pHD->bFull, sizeof(bool));
                size_t iWritten = fwrite(pLineH, iLenH, 1, fOut);
                delete[] pLineH;
                if (iWritten == 1) {
                    for (uint i = 0; (iResult == 0) && (i < vBinGeneFiles.size()); i++) {
                        printf("Appending [%s]\n",  vBinGeneFiles[i]->getName());
                        iResult = appendData(fOut, vBinGeneFiles[i]->getFileHandle());
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
        
        for (uint i = 0; (iResult == 0) && (i < vBinGeneFiles.size()); i++) {
            delete vBinGeneFiles[i];
        }
    } else {
        usage(apArgV[0]);
    }
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}


