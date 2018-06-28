
#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "DistMat.h"
#include "BinPheneFile.h"
#include "SequenceDist.h"

#include "QDFArray.cpp"
#include "DistMat.cpp"
#include "SequenceDist.cpp"

//----------------------------------------------------------------------------
// calcEuclideanDist
//   
static float calcEuclideanDist(float *p1, float *p2, int iN) {
    float dS = 0;
    for (int i = 0; i < iN; i++) {
        dS += (p1[i]-p2[i])*(p1[i]-p2[i]);
    }
    return sqrt(dS);
}



//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - phenetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <phenomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>] [-r <referencefile>]]\n");
    printf("where\n");
    printf("  phenomefile      a file created by QDFPhenSampler\n");
    printf("  outputbody       output file name body\n");
    printf("  movestatqdf      qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf          qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                   This file must be specified if movstatqdf contains no grid data\n");
    printf("                   the various outputs will append suffices tp it\n");
    printf("  referencefile    file containing one or more reference phenomes, relative to which\n");
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
    printf("  '%s'    Geo-Phenetic distances\n", s);
    printf("\n");
}

//----------------------------------------------------------------------------
// readPhenomes2
//   try to read given file a s binary
//
BinPheneFile *readPhenomes2(const char *pPheneFile) {
    int iNumPhenomes = -1;
    BinPheneFile *pBP = BinPheneFile::createInstance(pPheneFile);
    if (pBP != NULL) {
        iNumPhenomes = pBP->read();
        if (iNumPhenomes <= 0) {
            delete pBP;
            pBP = NULL;
        }
    }
    return pBP;
}   


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pPheneFile     = NULL;
    char *pOutput        = NULL;
    char *pMoveStatQDF   = NULL;
    char *pReferenceFile = NULL;

    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(5,
                                   "-g:S!",   &pPheneFile,
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

                id_phenomes mIDPhen;

                BinPheneFile *pBP = readPhenomes2(pPheneFile);
                if (pBP != NULL) {
                    int iNumPhenes = pBP->getNumPhenomes();
                    int iPhenomeSize = pBP->getPhenomeSize();
                    mvIDs    = pBP->getvIDs();
                    mIDPhen  = pBP->getIDPhen();
                    mIDNodes = pBP->getIDNodes();
                    mIdLocs  = pBP->getIDLocs();

                    tnamed_ids::const_iterator it;
                    int iii = 0;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        iii += it->second.size();
                    }
                    printf("after init: #idgen: %zd, #vIDs: %d\n", mIDPhen.size(), iii);


                    SequenceDist<float> *pSD = new SequenceDist<float>(iNumPhenes, iPhenomeSize, mIDNodes, mIDPhen, mvIDs, mIdLocs, calcEuclideanDist);
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
                    fprintf(stderr, "Problem reading the phenome file [%s]\n", pPheneFile);
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
