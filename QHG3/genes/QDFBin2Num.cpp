#include <stdio.h>
#include <string.h>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "BinGeneFile.h"


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - Converting Creating bin sample files to numeric\n", pApp);
    printf("Usage:\n");
    printf("%s -i <InBinFile> [-o <OutNumFile>]\n", pApp);
    printf("where\n");
    printf("  InBinFile   binary sample file (as created by QDFSampler\n");
    printf("  OutNumFile  numeric samplefile (default: same name as inoput; with suffix 'dat')«\n");
    printf("              output format: \n");
    printf("                output ::= <line>*\n");
    printf("                line   ::= <AgentID> <nucleotides>*\n");
    printf("                nucleotides ::= <nuc2bit> | <nuc1bit>\n");
    printf("                nuc1bit ::= \"0\" | \"1\"\n");
    printf("                nuc2bit ::= \"0\" | \"1\" | \"2\" | \"3\"\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int   iResult     = 0;
    char *sInBinFile  = NULL;
    char *sOutNumFile = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(2,
                                   "-i:S!",  &sInBinFile,
                                   "-o:S",   &sOutNumFile);

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                char *sActualOut = NULL;
                if (sOutNumFile != NULL) {
                    sActualOut = new char[strlen(sOutNumFile)+1];
                    strcpy(sActualOut, sOutNumFile);
                } else {
                    char *p = strstr(sOutNumFile, ".bin");
                    if (p != NULL) {
                        *p = '\0';
                    }
                    sActualOut = new char[strlen(sOutNumFile)+5+1];
                    strcpy(sActualOut, sOutNumFile);
                    strcat(sActualOut, ".dat");
                }
                BinGeneFile *pBGF = BinGeneFile::createInstance(sInBinFile);
                if (pBGF != NULL) {
                    int iNumGenomes = pBGF->read();
                    
                    if (iNumGenomes > 0) {
                        const id_genomes &mIdGen = pBGF->getIDGen();
                        int iGenomeSize   = pBGF->getGenomeSize();
                        int iNumBlocks    = 0;
                        int iNucsInBlock  = 0;
                        char *(*blockToNumStr)(ulong lBlock, char *pNuc);
                        
                        int iBitsPerNuc = pBGF->getBitsPerNuc();
                        const tnamed_ids mvIDs =  pBGF->getvIDs();
                        if (iBitsPerNuc == 1) {
                            iNumBlocks    =  BitGeneUtils::numNucs2Blocks((int)iGenomeSize);
                            iNucsInBlock  =  BitGeneUtils::NUCSINBLOCK;
                            blockToNumStr = &BitGeneUtils::blockToNumStr;
                        } else {
                            iNumBlocks    =  GeneUtils::numNucs2Blocks((int)iGenomeSize);
                            iNucsInBlock  =  GeneUtils::NUCSINBLOCK;
                            blockToNumStr = &GeneUtils::blockToNumStr;
                        }
                        printf("read %d genomes with genome size %d\n", iNumGenomes, iGenomeSize);
                        // 
                        // open output file
                        FILE *fOut = fopen(sOutNumFile, "wt");
                        if (fOut != NULL) {
                            tnamed_ids::const_iterator it;
                            for (it= mvIDs.begin(); it != mvIDs.end(); ++it) {
                                for (uint i = 0; i < it->second.size(); ++i) {
                                    fprintf(fOut, "%12ld ", it->second[i]);
                                    id_genomes::const_iterator iti = mIdGen.find(it->second[i]);
                                    const ulong *p = iti->second;
                                    int iNucCount = iGenomeSize;
                                    for (int iB = 0; iB < 2*iNumBlocks; iB++) {
                                        char s[2*iNucsInBlock+1];
                                        (*blockToNumStr)(*p, s);
                                        iNucCount -= iNucsInBlock;
                                        if ((iNucCount <  iNucsInBlock) && (iNucCount > 0)) {
                                            s[2*iNucCount] = '\0';
                                        }
                                        if (iB == iNumBlocks) {
                                            fprintf(fOut, " ");
                                            iNucCount = iGenomeSize;
                                        }
                                        fprintf(fOut, "%s", s);
                                        p++;
                                    }
                                    fprintf(fOut, "\n");
                                }
                            }
                            
                            fclose(fOut);
                        } else {
                            printf("Couldn't open [%s] for writing\n", sOutNumFile);
                        }
                    } else {
                        fprintf(stderr,"Couldn't read from [%s]\n", sInBinFile);
                    }
                    delete pBGF;
                } else {
                    fprintf(stderr,"Couldn't create BinGeneFile\n");
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr,"Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    return iResult;
}
