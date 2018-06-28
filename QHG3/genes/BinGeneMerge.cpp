#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"

typedef std::vector<std::string> stringvec;

typedef struct sHeaderData {
    char sMagic[4];
    int iGenomeSize;
    int iNumGenes;
    int iNumLocs;
    bool bFull;
} sHeaderData;

//----------------------------------------------------------------------------
// usage
//   
void usage(const char *pApp) {
    printf("%s - mere binary gene files\n", pApp);
    printf("Usage;\n");
    printf("  %s -i <genomefilelist> -o<outputbody>\n", pApp);
    printf("where\n");
    printf("  genomefilelist     comma-separated list of binary genome files\n");
    printf("  outputname         name of output file\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// splitFiles
//   
int splitFiles(char *pBinFiles, stringvec &vBinFiles) {
    int iResult = 0;
    char *p = strtok(pBinFiles, ",");
    while (p != NULL) {
        vBinFiles.push_back(p);
        p = strtok(NULL, ",");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getHeader
//   
int getHeader(const char *pFile, sHeaderData *pHD) {
    int iResult = -1;
    FILE *fIn = fopen(pFile, "rt");
     if (fIn != NULL) {
         int iHeaderLen = 4*sizeof(char)+3*sizeof(int)+sizeof(bool);
         char *pH = new char[iHeaderLen];
         int iRead = fread(pH, iHeaderLen, 1, fIn);
   
         if (iRead == 1) {
             if ((memcmp(pH, "GENS", 4) == 0)  ||
                 (memcmp(pH, "GENX", 4) == 0)) {
                 memcpy(pHD->sMagic, pH, 4);
               
                 char *p = pH+4;
                 p = getMem(&pHD->iGenomeSize, p, sizeof(int));
                 p = getMem(&pHD->iNumGenes,   p, sizeof(int));
                 p = getMem(&pHD->iNumLocs,    p, sizeof(int));
                 p = getMem(&pHD->bFull,       p, sizeof(bool));
                 iResult = 0;
             } else {
                 char sMagic[5];
                 memcpy(sMagic, pH, 4);
                 sMagic[4] = '\0';
                 printf("Uknown maeic number [%s]\n", sMagic);
             }
             
         } else {
             printf("Couldn't read header\n");
             iResult = -1;
         }
         delete[] pH;
         fclose(fIn);
     } else {
         printf("Couldn't open [%s] for reading\n", pFile);
         iResult = -1;
     }
     return iResult;
}

                 

//----------------------------------------------------------------------------
// checkHeaders
//   
sHeaderData *checkHeaders(stringvec &vBinFiles) {
    int iResult = 0;
    sHeaderData *pHD = NULL;
    std::vector<sHeaderData> vHeaders;
    for (uint i = 0; (iResult == 0) && (i < vBinFiles.size()); i++) {
        sHeaderData sHD;
        iResult = getHeader(vBinFiles[i].c_str(), &sHD);
        if (iResult == 0) {
            vHeaders.push_back(sHD);
        }
    }
    if (iResult == 0) {
        int iNumGenesTotal = vHeaders[0].iNumGenes;
        int iNumLocsTotal  = vHeaders[0].iNumLocs;
        // compare headers
        for (uint i = 1; (iResult == 0) && (i < vHeaders.size()); i++) {
            if ((memcmp(vHeaders[0].sMagic, vHeaders[i].sMagic, 4) != 0) ||
                (vHeaders[0].iGenomeSize != vHeaders[i].iGenomeSize) ||
                (vHeaders[0].bFull       != vHeaders[i].bFull)) {
                iResult = -1;
            } else {
                iNumGenesTotal += vHeaders[i].iNumGenes;
                iNumLocsTotal  += vHeaders[i].iNumLocs;
            }
        }
        if (iResult == 0) {
            // create output header
            pHD = new sHeaderData;
            memcpy(pHD->sMagic,  vHeaders[0].sMagic, 4);
            pHD->iGenomeSize = vHeaders[0].iGenomeSize;
            pHD->bFull = vHeaders[0].bFull;
            pHD->iNumGenes = iNumGenesTotal;
            pHD->iNumLocs = iNumLocsTotal;
            char sMagic[5];
            memcpy(sMagic, pHD->sMagic, 4);
            sMagic[4] = '\0';
            printf("Resultin header:\n");
            printf("  Magic:      %s\n", sMagic);
            printf("  GenomeSize: %d\n", pHD->iGenomeSize);
            printf("  NumGenene:  %d\n", pHD->iNumGenes);
            printf("  NumLocs:    %d\n", pHD->iNumLocs);
            printf("  bFull:      %s\n", pHD->bFull?"yes":"no");
        }
    }
    return pHD;
}


//----------------------------------------------------------------------------
// appendData
//   
int appendData(FILE *fOut, const char *pFile) {
    int iResult = -1;
    FILE *fIn = fopen(pFile, "rb");
     if (fIn != NULL) {
         iResult = 0;
         int iHeaderLen = 4*sizeof(char)+3*sizeof(int)+sizeof(bool);
         fseek(fIn, iHeaderLen, SEEK_SET);
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
         fclose(fIn);
     } else {
         printf("Couldn't open [%s] for reading\n", pFile);
     }
     return iResult;
}    


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char *pBinFiles = NULL;
    char *pOutputFile = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(2,
                                   "-i:S!",   &pBinFiles,
                                   "-o:S!",   &pOutputFile);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                stringvec vBinFiles;
                iResult = splitFiles(pBinFiles, vBinFiles);
                if (iResult == 0) {
                    sHeaderData *pHD = checkHeaders(vBinFiles);
                    
                    if (pHD != NULL) {
                        FILE *fOut = fopen(pOutputFile, "wb");
                        if (fOut != NULL) {
                            int iLenH =  4*sizeof(char)+3*sizeof(int)+sizeof(bool);
                            char *pLineH = new char[iLenH];
                            // write file header;
                            char *p0 = pLineH;
                            p0 = putMem(p0, pHD->sMagic, 4*sizeof(char));
                            p0 = putMem(p0, &pHD->iGenomeSize, sizeof(int));
                            p0 = putMem(p0, &pHD->iNumGenes, sizeof(int));
                            p0 = putMem(p0, &pHD->iNumLocs, sizeof(int));
                            p0 = putMem(p0, &pHD->bFull, sizeof(bool));
                            size_t iWritten = fwrite(pLineH, iLenH, 1, fOut);
                            delete[] pLineH;
                            if (iWritten == 1) {
                                for (uint i = 0; (iResult == 0) && (i < vBinFiles.size()); i++) {
                                    iResult = appendData(fOut, vBinFiles[i].c_str());
                                }
                                
                                
                            } else {
                                printf("Error writing to [%s]\n", pOutputFile);
                                iResult = -1;
                            }
                            fclose(fOut);
                        } else {
                            printf("Couldn't open [%s] for writing\n", pOutputFile);
                            iResult = -1;
                        }
                        delete pHD;
                    } else {
                        printf("Non-compatible headers");
                        iResult = -1;
                    }
                } else {
                    printf("Couldn't split input file list\n");
                    iResult = -1;
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
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}


