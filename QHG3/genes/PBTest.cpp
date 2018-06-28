#include <stdio.h>
#include <string.h>
#include <omp.h>

#include <algorithm>
#include <vector>
#include <set>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "QDFUtils.h"
#include "IDSample.h"
#include "QDFGenomeExtractor.h"
#include "RankTable.h"
#include "ProteinBuilder.h"
#include "ProteomeComparator.h"


typedef std::map<idtype, proteome> protmap;
#define ATTR_GENOME_SIZE  "Genetics_genome_size"
#define ATTR_BITS_PER_NUC "Genetics_bits_per_nuc"


const char *sGenome1 = 
  // LysArgSTRProAlaGlyGlySerGlyValIleLysAspSTPArgSerSTRLeuIleSerProIleArgCys
    "AAACGTATGCCTGCAGGTGGTTCTGGAGTGATAAAGGATTGACGTTCGATGTTAATTTCGCCCATACGATGT"
  // STRHisLeuArgAlaAlaGlySTPSTRValValGlnSTPArgSerLeuGlySTRValThrArgProGlySTP
    "ATGCATTTGCGTGCAGCTGGGTGAATGGTGGTACAGTAGAGAAGTTTAGGTATGGTTACGCGGCCGGGATAA";

const char *sGenome2 = 
  // LysArgSTRProAlaGlyGlySerGlyValIleLysAspSTPArgSerSTRLeuIleSerProIleArgCysGlySTRValThrArgProGlySTP
    "AAACGTATGCCTGCAGGTGGTTCTGGAGTGATAAAGGATTGACGTTCGATGTTAATTTCGCCCATACGATGTGGTATGGTTACGCGGCCGGGATGA"
  // STRHisLeuArgAlaAlaGlySTPSTRValValGlnSTPArgSerLeuGlySTRValThrArgProGlySTPGlySTRValThrArgProGlyLeu
    "ATGCATTTGCGTGCAGCTGGGTGAATGGTGGTACAGTAGAGAAGTTTAGGTATGGTTACGCGGCCGGGATAAGGTATGGTTACGCGGCCGGGATTA";

const char *sGenome3 = 
  // LysArgSTRProAlaGlyGlySerGlyValIleLysAspSTPArgSerSTRLeuIleSerProIleArgCysGlySTRValThrArgProGlySTP
    "AAACGTATGCCTGCAGGTGGTTCTGGAGTGATAAAGGATTGACGTTCGATGTTAATTTCGCCCATACGATGTGGTATGGTTACGCGGCCGGGATAA"
  // LysArgSTRProAlaGlyGlySerGlyValIleLysAspSTPArgSerSTRLeuIleSerProIleArgCysGlySTRValThrArgProGlySTP
    "AAACGTATGCCTGCAGGTGGTTCTGGAGTGATAAAGGATTGACGTTCGATGTTAATTTCGCCCATACGATGTGGTATGGTTACGCGGCCGGGATAA";

const char *sGenome = 
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

#define STATE_SIZE 16
static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};

//----------------------------------------------------------------------------
// SimpleTest
//
int SimpleTest() {
    int iResult = 0;

    int iGenomeSize = strlen(sGenome2)/2;
   
    ulong *pGenome = GeneUtils::translateGenome(iGenomeSize, sGenome2);

    ProteinBuilder *pPB = ProteinBuilder::createInstance(pGenome, iGenomeSize);
    if (pPB != NULL) {
        iResult = pPB->translateGenome();
        if (iResult == 0) {
            const proteome &vProteome = pPB->getProteome();
            printf("Sequence\n");
            // for (int i = 0; i < 2; ++i) {
            GeneUtils::showGenome(pGenome/*+(i*iNumBlocks)*/, iGenomeSize, SHOW_GENES_NUC);
                //}
            printf("creates %zd proteins:\n", vProteome.size());
            for (uint i = 0; i < vProteome.size(); ++i) {
                const protein &vProt = vProteome[i];
                printf("  [ ");
                for (uint j = 0; j < vProt.size(); ++j) {
                    printf("%s ", ProteinBuilder::getAAName(vProt[j]));
                }
                printf("]\n");
            }

            printf("All aminoacids:\n");
            pPB->displaySequence();

        } else {
            printf("Couldn't translate Genome\n");
        }
        delete pPB;
    } else {
        printf("COUldn't create ProteinBuilder\n");
        iResult = -1;
    }
    delete[] pGenome;
    return iResult;
}


//----------------------------------------------------------------------------
// SimpleTestHash
//
int SimpleTestHash() {
    int iResult = 0;

    int iGenomeSize = strlen(sGenome2)/2;
   
    ulong *pGenome = GeneUtils::translateGenome(iGenomeSize, sGenome2);

    ProteinBuilder *pPB = ProteinBuilder::createInstance(pGenome, iGenomeSize);
    if (pPB != NULL) {
        iResult = pPB->translateGenomeHash();
        if (iResult == 0) {
            const prothash &vProtHash = pPB->getProtHash();
            printf("Sequence\n");
            // for (int i = 0; i < 2; ++i) {
            GeneUtils::showGenome(pGenome/*+(i*iNumBlocks)*/, iGenomeSize, SHOW_GENES_NUC);
                //}
            printf("creates %zd prothash:\n", vProtHash.size());
            for (uint i = 0; i < vProtHash.size(); ++i) {
                printf("  [ ");
                for (uint j = 0; j < MD5_SIZE/sizeof(ulong); j++) {
                    printf("%016lx ", vProtHash[i][j]);
                }
                printf("]\n");
            }


        } else {
            printf("Couldn't translate Genome\n");
        }
        delete pPB;
    } else {
        printf("COUldn't create ProteinBuilder\n");
        iResult = -1;
    }
    delete[] pGenome;
    return iResult;
}


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - building proteins for a selection of genomes\n", pApp);
    printf("Usage:\n");
    printf("%s -i <QDFPopFile> [-s <SpeciesName>] -o <OutputName> [-f <format>(\":\"<format>)*]\n", pApp);
    printf("      --location-file=<LocationFile>\n");
    printf("      [-g <QDFGeoFile>]\n");
    printf("      [-c <cutoff>] [-p <seed>]\n");
    printf("      [--dense]\n");
    printf("where\n");
    printf("  QDFPopFile   QDF Population file with genome info\n");
    printf("  SpeciesName  Species name ( if omitted, first species will be used)\n");
    printf("  OutputName   Name for outpud '.ped' and '.map' files\n");
    printf("  format               outpur ofrmat; one of \"ped\", \"bin\", \"asc\", and/or \"num\"\n");
    printf("  Locationfile         name of location file (format: see below)\n");
    printf("  QDFGeoFile           QDF grid file \n");
    printf("  cutoff               cutoff value for match table (default 0)\n");
    printf("  -p                  permute array of females before pairing using random seed (default 0: no permutation) \n");
    printf("  --dense              use if selected genomes dense in all genomes\n");
    printf("\n");
    printf("Location File Format\n");
    printf("  LocationFile ::= <LocationLine>*\n");     
    printf("  LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
    printf("  identifier   : string (name of location)\n");
    printf("  longitude    : double (longitude in degrees)\n");
    printf("  latitude     : double (latitude in degrees)\n");
    printf("  dist         : double (sample radius in km)\n");
    printf("  num          : int    (number of elements to sample)\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// placeStartStops
//
void placeStartStops(ulong *pGenome, int iGenomeSize, int iNum, int iMaxLen) {
    uint iPos0 = 0;
    int iNumBlocks  = GeneUtils::numNucs2Blocks(iGenomeSize);
    for (int i = 0; i < iNum; i++) {
        uint iOffset  = 3*((1.0*iMaxLen*rand())/(3.0*RAND_MAX));
        uint iLen  = 3*((1.0*iMaxLen*rand())/(3.0*RAND_MAX));
        uint iPos1 = iPos0+iOffset;
        uint iPos2 = iPos1+ iLen;
        uint iAll =  (2.0*rand())/(RAND_MAX+1.0);
        if (iPos2 < (uint)iGenomeSize-3) {
            GeneUtils::insertSequence(pGenome+(iAll*iNumBlocks), iGenomeSize, iPos1, "ATG"); 
            GeneUtils::insertSequence(pGenome+(iAll*iNumBlocks), iGenomeSize, iPos2, "TGA");
        }
        iPos0 = iPos2;
    }
}


//----------------------------------------------------------------------------
// showProteins
//
void showProteins(const proteome &vProteome, long id) {
    printf("Genome %ld: %zd proteins\n", id, vProteome.size());
    for (uint i = 0; i < vProteome.size(); ++i) {
        const protein &pr = vProteome[i];
        printf("  (%03d:%zd)[", i, pr.size());
        for (uint j = 0; j < pr.size(); ++j) {
            printf("%s", ProteinBuilder::getAAName(pr[j]));
            
        }
        printf("]\n");
        
    }
}


//----------------------------------------------------------------------------
// showAllProteomes
//
void showAllProteomes(QDFGenomeExtractor *pQGE) {
    int iResult = 0;
    printf("And now the proteins\n");
    protmap mProts;
    // now we have the selection 
    const IDSample *pSample =  pQGE->getSample();
    idset sSelected;
    pSample->getFullIDSet(sSelected);
    printf("We have %zd ids\n", sSelected.size());
    int iGenomeSize = pQGE->getGenomeSize();
    
    
    int iNumBlocks  = GeneUtils::numNucs2Blocks(iGenomeSize);
    idset::const_iterator it;
    ulong *pGenome = new ulong[2*iNumBlocks];
    for (it = sSelected.begin(); (iResult == 0) && (it != sSelected.end()); ++it) {
        memcpy(pGenome, pQGE->getGenome(*it), 2*iNumBlocks*sizeof(ulong));
        // insert fake coding regions
        // placeStartStops(pGenome, iGenomeSize, 0, 30);                                
        ProteinBuilder *pPB = ProteinBuilder::createInstance(pGenome, iGenomeSize);
        if (pPB != NULL) {
            iResult = pPB->translateGenome();
            if (iResult == 0) {
                const proteome &vProteome = pPB->getProteome();
                if (vProteome.size() > 0) {
                    showProteins(vProteome, *it);
                } else {
                    //printf(" %zd proteins\n", vProteome.size());
                }
                mProts[*it] = vProteome;
            } else {
                printf("translate genome failed\n");
            }
            delete pPB;
        } else {
            printf("couldn't create ProteinBuilder\n");
        }
    }
    delete[] pGenome;
}

//----------------------------------------------------------------------------
// agdatacomp
//   compare function for sorting a ontainer of agdata*
//
bool agdatacomp(agdata *p1, agdata *p2) {
    return p1->iID < p2->iID;
}


//----------------------------------------------------------------------------
// genderSplit
//
int genderSplit(QDFGenomeExtractor *pQGE, std::vector<idtype> &vF, std::vector<idtype> &vM) {
    int iResult = 0;
    printf("gender splitting\n");

    // now we have the selection 
    const IDSample *pSample =  pQGE->getSample();
    idset sSelected;
    pSample->getFullIDSet(sSelected);
    

    locagd msLocAD;
    pSample->getLocationADSet(msLocAD);
    sampleinfo::const_iterator it_ltd;
    const sampleinfo &mmvAgentData = pSample->getSampleInfo();
    for (it_ltd = mmvAgentData.begin(); it_ltd != mmvAgentData.end(); ++it_ltd) {
        timeagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            
            // sort the IDs
            std::vector<agdata*> v2 = it_td->second;
            std::sort(v2.begin(),v2.end(), agdatacomp);
            
            for (uint i = 0; i < v2.size(); ++i) {
                agdata *pAD = v2[i];
                
                if (pAD->iGender == 0) {
                    vF.push_back(pAD->iID);
                } else {
                    vM.push_back(pAD->iID);
                }
            }
        }
    }        

    printf("We have %zd females and %zd males\n", vF.size(), vM.size());
    return iResult;
}


//----------------------------------------------------------------------------
// compareAllProteomes
//
float **compareAllProteomes(QDFGenomeExtractor *pQGE, std::vector<idtype> &vF, std::vector<idtype> &vM) {
    int iResult = 0;

    float **ppMatch = new float*[vF.size()];
    for (uint iF = 0; (iResult == 0) && (iF < vF.size()); ++iF) {
        ppMatch[iF] = new float[vM.size()];
        memset(ppMatch[iF], 0, vM.size()*sizeof(float));
    }


    int iGenomeSize = pQGE->getGenomeSize();
    for (uint iF = 0; (iResult == 0) && (iF < vF.size()); ++iF) {
        ProteinBuilder *pPB1 = ProteinBuilder::createInstance(pQGE->getGenome(vF[iF]), iGenomeSize);
        if (pPB1 != NULL) {
            iResult = pPB1->translateGenome();
            if (iResult == 0) {
                const proteome &vProteome1 = pPB1->getProteome();
            
                for (uint iM = 0; (iResult == 0) && (iM < vM.size()); ++iM) {
                    ProteinBuilder *pPB2 = ProteinBuilder::createInstance(pQGE->getGenome(vM[iM]), iGenomeSize);
                    if (pPB2 != NULL) {
                        iResult = pPB2->translateGenome();
                        if (iResult == 0) {
                            const proteome &vProteome2 = pPB2->getProteome();
                            
                            int iMatch =  ProteomeComparator::countProteinMatches(vProteome1, vProteome2);
                            //printf("%u, %u:%d\n", iF, iM, iMatch);
                            ppMatch[iF][iM] = iMatch;
                        } else {
                            //couldn't translate
                        }
                        delete pPB2;
                    } else {
                        //couldn't create
                    }
                }
            } else {
                //couldn't translate
            }
            delete pPB1;
        } else {
            //couldn't create
        }
    }

    if (iResult == 0) { 
    } else {
        for (uint i = 0; i < vF.size(); ++i) {
            delete[] ppMatch[i];
        }
        delete[] ppMatch;
        ppMatch = NULL;
    }

    return ppMatch;
}


//----------------------------------------------------------------------------
// showHash
//
void showHash(ulong *pHash) {
    printf("[");
    for (uint i = 0; i < MD5_SIZE/sizeof(ulong); i++) {
        printf(" %016lx", pHash[i]);
    }
    printf(" ]\n");
}


//----------------------------------------------------------------------------
// compareAllProtHash
//
float **compareAllProtHash(QDFGenomeExtractor *pQGE, std::vector<idtype> &vF, std::vector<idtype> &vM) {
    int iResult = 0;

    int iGenomeSize = pQGE->getGenomeSize();

    float **ppMatch = new float*[vF.size()];
    for (uint iF = 0; (iResult == 0) && (iF < vF.size()); ++iF) {
        ppMatch[iF] = new float[vM.size()];
        memset(ppMatch[iF], 0, vM.size()*sizeof(float));
    }

    // remember the male proteomes for later use
    std::vector<std::pair<ProteinBuilder *, const prothash*> > vInner;
    for (uint iM = 0; (iResult == 0) && (iM < vM.size()); ++iM) {
        ProteinBuilder *pPB2 = ProteinBuilder::createInstance(pQGE->getGenome(vM[iM]), iGenomeSize);
        if (pPB2 != NULL) {
            iResult = pPB2->translateGenomeHash();
            if (iResult == 0) {
                vInner.push_back(std::pair<ProteinBuilder *, const prothash*>(pPB2, &(pPB2->getProtHash())));
            }
        }
    }

    for (uint iF = 0; (iResult == 0) && (iF < vF.size()); ++iF) {
        ProteinBuilder *pPB1 = ProteinBuilder::createInstance(pQGE->getGenome(vF[iF]), iGenomeSize);
        if (pPB1 != NULL) {
            iResult = pPB1->translateGenomeHash();
            if (iResult == 0) {
                const prothash &vProtHash1 = pPB1->getProtHash();
            
                int iMatch = 0;
                for (uint iM = 0; (iResult == 0) && (iM < vM.size()); ++iM) {
                    // get the saved proteome
                    const prothash &vProtHash2 = *(vInner[iM].second);
                    /*
                    iMatch = 0;
                    for (uint i1 = 0; i1 < vProtHash1.size(); ++i1) {
                        for (uint i2 = 0; i2 < vProtHash2.size(); ++i2) {
                            bool bSame = true;
                            //printf("F%03d:%03d ", iF, i1); showHash(vProtHash1[i1]);
                            //printf("M%03d:%03d ", iM, i2); showHash(vProtHash2[i2]);
                            for (uint k = 0; bSame && (k < MD5_SIZE/sizeof(long)); k++) {
                                if (vProtHash1[i1][k] != vProtHash2[i2][k]) {
                                    bSame = false;
                                }
                            }
                            //                                    printf("--> %s\n", bSame?"same":"different");
                            iMatch += (bSame?1:0);
                        }
                    }
                    */
                    iMatch = ProteomeComparator::countProtHashMatches(vProtHash1, vProtHash2);
                    //printf("%u, %u:%d\n", iF, iM, iMatch);
                    ppMatch[iF][iM] = iMatch;
                }

            } else {
                //couldn't translate
            }
            delete pPB1;
        } else {
            //couldn't create
        }
    }
    for (uint iM = 0; (iResult == 0) && (iM < vM.size()); ++iM) {
        delete vInner[iM].first;
    }


    if (iResult == 0) { 
    } else {
        for (uint i = 0; i < vF.size(); ++i) {
            delete[] ppMatch[i];
        }
        delete[] ppMatch;
        ppMatch = NULL;
    }

    return ppMatch;
}



//----------------------------------------------------------------------------
// createWELL
//
WELL512 **createWELL() {
    int iMaxThreads = omp_get_max_threads();
    WELL512 **apWELL =new WELL512*[iMaxThreads];
    for (int iT = 0; iT < iMaxThreads; iT++) {
        unsigned int temp[STATE_SIZE]; 
        for (unsigned int j = 0; j < STATE_SIZE; j++) {
            temp[j] = s_aulDefaultState[(iT+13*j)%16];
        }
        apWELL[iT]     = new WELL512(temp);
    }
    return apWELL;
}
//----------------------------------------------------------------------------
// createWELL
//
void deleteWELL(WELL512 **apWELL)  {
    int iMaxThreads = omp_get_max_threads();
    for (int iT = 0; iT < iMaxThreads; iT++) {
        delete apWELL[iT] ;
    }
    delete apWELL;
}


//----------------------------------------------------------------------------
// main1
//
int main1(int iArgC, char *apArgV[], bool bHash) {
    int iResult = 0;
    char *pPopFile      = NULL;
    char *pGeoFile      = NULL;
    char *pSpecies      = NULL;
    char *pOutputBody   = NULL;
    char *pFormatList   = NULL;
    char *pLocationFile = NULL;
    float fCutOff       = 0;
    int iSeed           = 0;
    bool bPermute       = false;
    bool bQuiet         = true;
    bool bDense         = false;
  


    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(9,
                                   "-i:S!",  &pPopFile,
                                   "-s:S",   &pSpecies,
                                   "-g:S",   &pGeoFile,
                                   "-f:S",   &pFormatList,
                                   "-o:S!",  &pOutputBody,
                                   "-c:f",   &fCutOff,
                                   "-p:i",   &iSeed,
                                   "--location-file:S!",   &pLocationFile,
                                   "--dense:0", &bDense);


        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (!bQuiet) pPR->display();
                
                iResult = 0;
                

                QDFGenomeExtractor *pQGE = NULL;

                if (pGeoFile != NULL) {
                    pQGE = QDFGenomeExtractor::createInstance(pGeoFile, pPopFile, pSpecies, ATTR_GENOME_SIZE, ATTR_BITS_PER_NUC, GENOME_DATASET_NAME, false);
                } else {
                    pQGE = QDFGenomeExtractor::createInstance(pPopFile, pSpecies, ATTR_GENOME_SIZE, ATTR_BITS_PER_NUC, GENOME_DATASET_NAME, false);
                }
                if (pQGE != NULL) {
                    pQGE->setVerbose(!bQuiet);
                    if (pQGE->getBitsPerNuc()  != 2) {
                        printf("Protein-building only works for 2-bit genomes\n");
                        iResult = -1;
                    }
                    if (iResult == 0) {
                        iResult = pQGE->createSelection(pLocationFile, NULL, bDense, -1);
                       
                        if (iResult == 0) {
                            if (!bQuiet) {
                                printf("selected %d ids:", pQGE->getNumSelected());
                                const idset &sSelected = pQGE->getSelectedIDs();
                                if (false) {
                                    for (idset::const_iterator it = sSelected.begin(); it != sSelected.end(); ++it) {
                                        printf(" %ld", *it);
                                    }
                                }
                                printf("\n");
                            }
                        } else {
                            fprintf(stderr, "error creating selection\n");
                        }
                        if (iResult == 0) {
                            WELL512 **apWELL = createWELL();
                            //showAllProteomes(pQGE);
                            std::vector<idtype> vF;
                            std::vector<idtype> vM;
                            genderSplit(pQGE, vF, vM);
                            
                            float **ppMatch = NULL;
                            if (bHash) {
                                ppMatch = compareAllProtHash(pQGE, vF, vM);
                            } else {
                                ppMatch = compareAllProteomes(pQGE, vF, vM);
                            }
                            if (ppMatch != NULL) {
                                /*
                                for (uint i = 0; i < vF.size(); ++i) {
                                    printf("{");
                                    for (uint j = 0; j < vM.size(); ++j) {
                                        printf("%s %7.3f", (j != 0)?",":"", ppMatch[i][j]);
                                    }
                                    printf("},\n");
                                }
                                */
                               
                                bool bVerbose = !bQuiet;
                                
                                RankTable *pRT = RankTable::createInstance(vF.size(), vM.size(), apWELL, ppMatch);
                                if (pRT != NULL) {
                                    pRT->setVerbosity(bVerbose);

                                    if (iSeed > 0) {
                                        bPermute = true;
                                        srandom(iSeed);
                                    }

                                    //            pRT->display();
                                    iResult = pRT->makeAllPairings(fCutOff, bPermute);
                                    const couples &vc = pRT->getPairs();
                                    if (true) {
                                        printf("Got %zd pairs (%s)\n", vc.size(), bHash?"hash":"full");
                                        for (uint i = 0; i < vc.size(); ++i) {
                                            printf("  (%ld,%ld):%8.3f\n", vF[vc[i].first], vM[vc[i].second], ppMatch[vc[i].first][vc[i].second]);
                                        }
                                    }
                                    delete pRT;    
                                } else {
                                    printf("Xouldn't create RankTable\n");
                                }

                                for (uint i = 0; i < vF.size(); ++i) {
                                    delete[] ppMatch[i];
                                }
                                delete[] ppMatch;
                            }
                            deleteWELL(apWELL);   
                        }
                    
     
                    }
                    
                    delete pQGE;
                } else {
                    printf("Couldn't create QDFGenomeExtractor\n");
                    iResult = -1;
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


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    srand(1);
    float f1 = omp_get_wtime();
    iResult = main1(iArgC, apArgV, true) ;
    float f2 = omp_get_wtime();
    srand(1);
    float f3 = omp_get_wtime();
    iResult = main1(iArgC, apArgV, false) ;
    float f4 = omp_get_wtime();

    printf("Normal: %f\n", f4 - f3);
    printf("Hash:   %f\n", f2 - f1);
    
    //iResult = SimpleTest();

    //iResult = SimpleTestHash();

    return iResult;
}

