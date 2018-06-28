
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <map>
#include <vector>
#include <algorithm>

#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArray.cpp"
#include "CrossDistMat.h"
#include "GeneUtils.h"
#include "BinGeneFile.h"


//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - crosswise genetic distances between two sets of genomes\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <genomefile0>  -g <genomefile0> -o <outputfile>\n", pApp);
    printf("where\n");
    printf("  genomefile0      a bin-file (as created by QDFSampler)\n");
    printf("  genomefile1      a bin-file (as created by QDFSampler)\n");
    printf("  outputfile       output file name\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// readGenomes2
//   try to read given file a s binary
//
BinGeneFile *readGenomes2(const char *pGeneFile) {
    int iNumGenomes = -1;
    BinGeneFile *pBG = BinGeneFile::createInstance(pGeneFile);
    if (pBG != NULL) {
        iNumGenomes = pBG->read();
        if (iNumGenomes <= 0) {
            delete pBG;
            pBG = NULL;
        }
    }
    return pBG;
}   


//----------------------------------------------------------------------------
// reorderGenes
//  
ulong **reorderGenes(id_genomes &mIDGen, tnamed_ids &mvIDs) {
    ulong **pAllGenomes = new ulong*[mIDGen.size()];
    int iC = 0;
    tnamed_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        for (uint k = 0; k < it->second.size(); k++) {
            pAllGenomes[iC++] = mIDGen[it->second[k]];
        }
    }
    return pAllGenomes;
}


//----------------------------------------------------------------------------
// writeFullMatrix
//   
int writeFullMatrix(const char *pOutput, int **pM, int iNumGenes0, tnamed_ids &mvIDs0, int iNumGenes1, tnamed_ids &mvIDs1) {
    int iResult = -1;
    
    FILE *fOut = NULL;

    fOut = fopen(pOutput, "wt");

    if (fOut != NULL) {
        iResult = 0;
        int iMin = 100000;
        int iMax = -100000;
        double dAvg = 0;

        tnamed_ids::const_iterator it0;
        tnamed_ids::const_iterator it1;
        int iW = 0;
        for (it1 = mvIDs1.begin(); it1 != mvIDs1.end(); ++it1) {
	    iW += it1->second.size();
	}
        int iH = 0;
        for (it0 = mvIDs0.begin(); it0 != mvIDs0.end(); ++it0) {
	    iH += it0->second.size();
	}
        // write distances with prepended location and agent ID
//        int i = 0;
//        for (it0 = mvIDs0.begin(); it0 != mvIDs0.end(); ++it0) {
//            for (uint k0 = 0; k0 < it0->second.size(); k0++) {
            for (int i = 0; i < iH; i++) {
//                int j = 0;
//                for (it1 = mvIDs1.begin(); it1 != mvIDs1.end(); ++it1) {
//                    for (uint k1 = 0; k1 < it1->second.size(); k1++) {
                for (int j = 0; j < iW; j++) {
                        fprintf(fOut, "%d\t", pM[i][j]);
                        dAvg += pM[i][j];
                        if (pM[i][j] < iMin) {
                            iMin = pM[i][j];
                        }
                        if (pM[i][j] > iMax) {
                            iMax = pM[i][j];
                        }
//                    }
//                    j++;
                }
//                i++;
                fprintf(fOut, "\n");
//            }
        }

        fclose(fOut);
        printf("Writing full distance matrix\n  [%s]\n", pOutput);
        dAvg /= (iW*iH);
        printf("cc Num Distances: %d x %d = %d\n", iW, iH,  iW*iH);
        printf("cc Avg Distance:  %f\n", dAvg);
        printf("cc Min Distance:  %d\n", iMin);
        printf("cc Max Distance:  %d\n", iMax);
        
    } else {
        fprintf(stderr, "Couldn't open output file [%s]\n", pOutput);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createAndWriteDistMat
//   
int createAndWriteDistMat(int iGenomeSize, id_genomes &mIDGen0, tnamed_ids &mvIDs0, id_genomes &mIDGen1, tnamed_ids &mvIDs1, const char *pOutput) {
    int iResult = -1;
    ulong **pGenomes0 = NULL;
    pGenomes0 = reorderGenes(mIDGen0, mvIDs0);
    ulong **pGenomes1 = NULL;
    pGenomes1 = reorderGenes(mIDGen1, mvIDs1);
    CrossDistMat *pCDM = CrossDistMat::createCrossDistMat(iGenomeSize, pGenomes0, mIDGen0.size(), pGenomes1, mIDGen1.size());
    if (pCDM != NULL) {
        int **pM = pCDM->createMatrix();
        char sName1[512];
        sprintf(sName1, "%s.full.mat", pOutput);
        iResult = writeFullMatrix(sName1, pM, mIDGen0.size(), mvIDs0, mIDGen1.size(), mvIDs1);
        pCDM->showStats();
        delete pCDM;
    }
    
    if ((pGenomes0 != NULL) && (pGenomes1 != NULL))  {
        delete[] pGenomes0;
        pGenomes0 = NULL;
        delete[] pGenomes1;
        pGenomes1 = NULL;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pGeneFile0     = NULL;
    char *pGeneFile1     = NULL;
    char *pOutput        = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(3,
                                   "-G:S!",   &pGeneFile0,
                                   "-g:S!",   &pGeneFile1,
                                   "-o:S!",   &pOutput);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                int iGenomeSize0 = -1;
                int iGenomeSize1 = -1;
                id_genomes mIDGen0;
                id_genomes mIDGen1;

                tnamed_ids mvIDs0;
                tnamed_ids mvIDs1;

                iResult = -1;
                std::map<int,int> mIDNodes;
                tnamed_ids mvIDs;
                id_locs mIdLocs;

                id_genomes mIDGen;

                BinGeneFile *pBG0 = readGenomes2(pGeneFile0);
                if (pBG0 != NULL) {
                    iGenomeSize0 = pBG0->getGenomeSize();
                    mvIDs0       = pBG0->getvIDs();
                    mIDGen0      = pBG0->getIDGen();

                    BinGeneFile *pBG1 = readGenomes2(pGeneFile1);
                    if (pBG1 != NULL) {
                        iGenomeSize1 = pBG1->getGenomeSize();
                        mvIDs1       = pBG1->getvIDs();
                        mIDGen1      = pBG1->getIDGen();

                        if (iGenomeSize0 == iGenomeSize1) {
                            iResult = createAndWriteDistMat(iGenomeSize0, mIDGen0, mvIDs0, mIDGen1, mvIDs1, pOutput);

                            
                        } else {
                            printf("Genome sizes don't match: %d != %d\n", iGenomeSize0, iGenomeSize1);
                            iResult = -1;
                        }
                        delete pBG1;
                    } else {
                        printf("Couldn't read [%s]\n", pGeneFile1);
                        iResult = -1;
                    }
                    delete pBG0;
                } else {
                    printf("Couldn't read [%s]\n", pGeneFile0);
                    iResult = -1;
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}
