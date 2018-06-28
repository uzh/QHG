
#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "DistMat.h"
#include "BinGeneFile.h"
#include "SequenceDist.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"

#include "QDFArray.cpp"
#include "DistMat.cpp"
#include "SequenceDist.cpp"


//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - genetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <genomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>] [-r <referencefile>]]\n");
    printf("where\n");
    printf("  genomefile       a file created by GeneSamples\n");
    printf("  outputbody       output file name body\n");
    printf("  movestatqdf      qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf          qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                   This file must be specified if movstatqdf contains no grid data\n");
    printf("                   the various outputs will append suffices tp it\n");
    printf("  referencefile    file containing one or more reference genomes, relative to which\n");
    printf("                   the genetic distances are calculated.\n");
    printf("                   If \"auto\", genetic distances are calculated relative to the \n");
    printf("                   sample group closest to the distance origin.\n");
    printf("\n");
    printf("Outputs:\n");
    printf("always:\n");
    char s[512];
    sprintf(s, TEMPLATE_DIST_MAT, "XXX");
    printf("  '%s'  distance matrix\n", s);
    sprintf(s, TEMPLATE_TABLE, "XXX");
    printf("  '%s'  data table\n", s);
    printf("if reference file is given:\n");
    sprintf(s, TEMPLATE_REF_MAT, "XXX");
    printf("  '%s'   distance to reference\n", s);
    sprintf(s, TEMPLATE_GEO_SEQ, "XXX");
    printf("  '%s'    Geo-Genetic distances\n", s);
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
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pGeneFile     = NULL;
    char *pOutput        = NULL;
    char *pMoveStatQDF   = NULL;
    char *pReferenceFile = NULL;

    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(5,
                                   "-g:S!",   &pGeneFile,
                                   "-o:S!",   &pOutput,
                                   "-m:S",    &pMoveStatQDF,
                                   "-G:s",     sGridQDF,
                                   "-r:S",    &pReferenceFile);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                iResult = -1;
                std::map<idtype,int> mIDNodes;
                tnamed_ids mvIDs;
                id_locs mIdLocs;

                id_genomes mIDGen;

                BinGeneFile *pBG = readGenomes2(pGeneFile);
                if (pBG != NULL) {
                    int iNumGenes    = pBG->getNumGenomes();
                    int iGenomeSize = pBG->getGenomeSize();
                    mvIDs    = pBG->getvIDs();
                    mIDGen   = pBG->getIDGen();
                    mIDNodes = pBG->getIDNodes();
                    mIdLocs  = pBG->getIDLocs();

                    tnamed_ids::const_iterator it;
                    int iii = 0;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        iii += it->second.size();
                    }
                    printf("after init: #idgen: %zd, #vIDs: %d\n", mIDGen.size(), iii);
                    SequenceDist<ulong>::calcdist_t fDistCalc = NULL;
                    if  (pBG->getBitsPerNuc()) {
                        fDistCalc = BitGeneUtils::calcDistFloat;
                    } else {
                        fDistCalc = GeneUtils::calcDistFloat;
                    }
                    SequenceDist<ulong> *pSD = new SequenceDist<ulong>(iNumGenes, iGenomeSize, mIDNodes, mIDGen, mvIDs, mIdLocs, fDistCalc);
                    if (pSD != NULL) {
                        // if MoveStat qdf is present, to travel/genetic distance calcs
                        if (pMoveStatQDF != NULL) {
                            // iff no GridFile is specified, assume the MoveStat file 
                            // to contain grid data as well
                            //
                            if (*sGridQDF == '\0') {
                                strcpy(sGridQDF, pMoveStatQDF);
                            }

                            iResult = pSD->prepareNodes(sGridQDF, pMoveStatQDF);
                            if (iResult == 0) {
                                iResult = pSD->distGeoSequences2(pOutput);
                                if (iResult == 0) {
                                    printf("writing dist mat\n");
                                    iResult = pSD->createAndWriteDistMat(pOutput);
                                }
                            } else {
                                fprintf(stderr, "Couldn't properly preparenodes\n");
                            }
                        } else {
                            // no grid/movestats
                            iResult = pSD->createAndWriteDistMat(pOutput);
                            
                        }
                    } else {
                        fprintf(stderr, "Couldn't create SequenceDist\n");
                    }
                    
                } else {
                    // nothing to do
                    fprintf(stderr, "Problem reading the genome file [%s]\n", pGeneFile);
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
