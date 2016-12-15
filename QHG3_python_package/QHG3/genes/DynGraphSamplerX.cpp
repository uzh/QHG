#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>  // omp_get_wtime()
#include <time.h>

#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "types.h"
#include "utils.h"
#include "strutils.h"
#include "colors.h"
#include "geomutils.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "BufReader.h"
#include "crypto.h"
#include "SystemInfo.h"
#include "QDFUtils.h" 
#include "QDFArray.h" 
#include "AncestorNode.h"
#include "AncGraphBase.h"

#include "DynAncGraphX.h"

#include "AGOracle.h"
#include "AGFileMerger.h"
#include "GraphEvolverBase.h"
#include "GraphEvolverF.h"
#include "GeneUtils.h"
#include "GeneWriter.h"
#include "IDSample.h"
#include "IDSampler2.h"

#define DEF_NUM_CROSS 1
#define DEF_MUT_RATE  0
#define ANC_ORACLE_BLOCK_SIZE 10000
#define ANC_SIZE 4

#define NUM_LOAD_NODES 1000
#define MERGE_BUF_SIZE 10000
#define AG_ORACLE_BLOCK_SIZE 10000
#define GENOME_SAVE_BUFFER_SIZE 1000


typedef std::vector<std::string>  stringvec; 

#define RADIUS 6371.3

                    

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - create or read ancestor graph an evolve a genome along it\n", pApp);
    printf("Usage:\n");
    printf("  (creating ancestor graph)\n");
    printf("  %s --anc-file=<AncFile> --grid-file=<QDFGridFile>\n", pApp);
    printf("        --pop-file=<QDFPopFile>[:<PopName>] (\",\"<QDFPopFile>[:<PopName>])*\n");
    printf("        --location-file=<LocationFile>[\":\"<dist>[\":\"<num>]] --anc-size=<AncSize>\n");
    printf("        --output-file=<OutputFile>  [--output-format=<OutputFormat>]\n");
    printf("        [--use-file-mode]\n");
    printf("        [--parallel-build=(\"yes\"|\"no\")]\n");
    printf("       ([--genome-file=<GenomeFile>] | --initial-genes=<Randomness>)\n");
    printf("        [--crossing-overs=<NumCrossOver>] [--mutation-rate=<MutationRate>]\n");
    printf("        [--anc-block-size=<AncBlockSize>] \n");
    printf("        [--dump-buffer-size=<DumpBufferSize>] \n");
    printf("       ([--anc-oracle-read=<AncOracleRead>] | [--anc-oracle-write=<AncOracleWrite>])\n");
    printf("        [--ag-output=<AGOutputFile>]\n");
    printf("        [--ag-block-size=<AGBlockSize>]\n");
    printf("        [--random-seed=<RandomSeed>]\n");
    printf("or\n");
    printf("  (reading ancestor graph)\n");
    printf("  %s --ag-input=<AGInputFile> --grid-file=<QDFGridFile>\n", pApp);
    printf("        --pop-file=<QDFPopFile>[:<PopName>] --location-file=<LocationFile>[\":\"<dist>[\":\"<num>]] --anc-size=<AncSize>\n");
    printf("        --output-file=<OutputFile>  [--output-format=<OutputFormat>]\n");
    printf("        [--use-file-mode]\n");
    printf("        [--parallel-build=(\"yes\"|\"no\")]\n");
    printf("       ([--genome-file=<GenomeFile>] | --initial-genes=<Randomness>)\n");
    printf("        [--crossing-overs=<NumCrossOver>] [--mutation-rate=<MutationRate>]\n");
    printf("        [--dump-buffer-size=<DumpBufferSize>] \n");
    printf("        [--ag-block-size=<AGBlockSize>]\n");
    printf("        [--random-seed=<RandomSeed>]\n");
    printf("or\n");
    printf("  %s --show-formats=which\n", pApp);
    printf("where\n");
    printf("  AncFile         binary file containing ancestor data\n");
    printf("                  format: (<id><momid><dadid> [<timeloc>])* \n");
    printf("  AncSize         size of binary ancestor record (3 or 4)\n");
    printf("  QDFGridFile     QDF file containing grid and geography\n");
    printf("  QDFPopFile      QDF file containing population data\n");
    printf("                  This file should be created by the same simulation that created <AncFile>\n");
    printf("  PopName         Name of population (only needed if QDFPopFile contains several populations)\n");
    printf("  GenomeFile      file containing initial genomes (cf. \"--show-formats=in\"\n");
    printf("  LocationFile    file containing locations and sampling areas (cf. \"--show-formats=in\")\n");
    printf("   dist           overrides distances in LocationFile\n");
    printf("   num            overrides selection numbers in LocationFile\n");
    printf("  Randomness      parameters for creating random genes:\n");
    printf("    Randomness  ::= <GenomeSize>\":\"<type>\n");
    printf("    GenomeSize  :  number of nucleotides\n");
    printf("    type        ::= <fullType> | <flatType>\n");
    printf("    fullType    :: = \"full\"\n");
    printf("    flatType    :: = \"flat:\"<GenomeSize> \":\" <NumMutations>\n");
    printf("\n");
    printf("  OutputFile      output file name\n");
    printf("  OutputFormat    (\"bin\"|\"asc\"|\"plink\")\n");
    printf("                  bin:   binary\n");
    printf("                  asc:   ascii\n");
    printf("                  plink: plink format\n");
    printf("                  (cf. \"--show-formats=out\")\n");
    printf("\n");
    printf("  NumCrossOver    Number of cross overs in a new Genome (default:%d)\n", DEF_NUM_CROSS);
    printf("  MutationRate    MutationRate: #mutations per bp per year (default: %d)\n", DEF_MUT_RATE);
    printf("\n");
    printf("  AncBlockSize    Number of records from the anc file to be read (default %d)\n", ANC_ORACLE_BLOCK_SIZE);
    printf("  DumpBufferSize  if this option is used, the genome is built bottom up,\n");
    printf("                  while removing unused genomes as soon as their number\n");
    printf("                  exceeds DumpBufferSize.\n");
    printf("                  if this option is omitted the genome is built recursively top down,\n");
    printf("                  keeping everything in memory (may use a lot of memory!).\n");
    printf("  AncOracleRead   File containing a previously created oracle for AncFile\n");
    printf("  AncOracleWrite  Name of file to save oracle for AncFile\n");
    printf("  AGOutputFile    Name of file to save AncestorGraph\n");
    printf("  AGInputFile     Name of file to read AncestorGraph from\n");
    printf("  AGBlockSize     Oracle block size for AncestorGraph\n");
    printf("  RandomSeed      Seed for random number generator (rand)\n");
    printf("                  -1  creates a seed from current time (default)\n");
    printf("                  (1  is seed used if srand is not called)\n"); 
    printf("  --use-file-mode  set filemode (should be used for really big Anc files\n");
    printf("  --parallel-build parallelized genome building (OMP)\n");
    printf("\n");
    printf("  which           (\"in\" | \"out\") show input or output file formats\n");
    printf("\n");
    printf("\n");
    printf("Examples:\n");
    printf("\n");
    printf("Create from scratch\n");
    printf("%s  --anc-file=all_010000.anc4s --anc-size=4  --grid-file=../resources/grids/GridSG_ieq_128.qdf\n", pApp);
    printf("              --pop-file=/home/jody/Simulations/anctest19h/ppp_pop-Sapiens__010000.qdf --location-file=LocList_10000_30.txt\n");
    printf("              --output-file=test_10000.ped --output-format=plink --initial-genes=flat:8192:4\n");
    printf("              --crossing-overs=1 --mutation-rate=0.0001 --anc-block-size=400000 --dump-buffer-size=10000\n");
    printf("              --use-file-mode=yes --random-seed=7123  --ag-output=test_10000.ag\n");
    printf("\n");

    printf("Use existing AG file\n");
    printf("%s --ag-input=H501_bb.ag --grid-file=../resources/grids/GridSG_ieq_128.qdf\n", pApp);
    printf("            --pop-file=../app/foancHPop/ppp__000501.qdf --location-file=LocList_10000_30.txt\n");
    printf("            --output-file=H501_genesF_bd.plink --output-format=plink --initial-genes=flat:8192:4\n");
    printf("            --crossing-overs=1 --mutation-rate=0.0001 --anc-block-size=400000 --dump-buffer-size=10000\n");
    printf("            --use-file-mode\n");
    printf("Note: in this case the location file *must* be the same as the one used for the creation of the AG file\n");

    printf("\n");
}

 
//----------------------------------------------------------------------------
// showFormats
//
void showFormats(bool bOut) {
    if (bOut) {
        printf("OUTPUT FORMATS\n");
        printf("---------------------------------------------\n");
        printf("Binary output format\n");
        printf("---------------------------------------------\n");
        printf("  File          ::= <Header><LocationBlock>*\n");
        printf("  Header        ::= \"GENS\"<GenomeSize><NumGenes><NumLocs>\n");
        printf("  LocationBlock ::= <BlockHeader><GenomeData>*\n");
        printf("  BlockHeader   ::= <NameLen><Name><LocLon><LocLat><LocDist><NumLocGenes>\n");
        printf("  GenomeData    ::= <GenomeHeader><Genome>\n");
        printf("  GenomeHeader  ::= <ID><MomID><DadID><Gender><CellID><Lon><Lat>\n");
        printf("  GenomeSize  : int (size of a single DNA strand\n");
        printf("  NumGenes    : int (total number of genes)\n");
        printf("  NumLocs     : int (number of sampling locations)\n");
        printf("  NameLen     : int (length of location name (including terminating 0))\n");
        printf("  Name        : char sequence (name of sampling location)\n");
        printf("  LocLon      : double (longitude of sampling location)\n");
        printf("  LocLat      : double (latitude of sampling location)\n");
        printf("  LocDist     : double (search distance of sampling location)\n");
        printf("  NumLocGenes : int (number of genes found for location)\n");
        printf("  ID          : int (id of agent)\n");     
        printf("  MomID       : int (id of agent's mother)\n");
        printf("  DadID       : int (id of agent's father)\n");
        printf("  Gender      : int (gender of agent)\n"); 
        printf("  CellID      : int (id of cell holding agent)\n");
        printf("  Lon         : double (longitude of cell)\n");
        printf("  Lat         : double (latitude of cell)\n");
        printf("  Genome      : long sequence (gneome)\n");
        printf("\n");
        printf("---------------------------------------------\n");
        printf("ASCII output format\n");
        printf("---------------------------------------------\n");
        printf("  File          ::= <LocationBlock>*\n");
        printf("  LocationBlock ::= <HeaderLine><GenomeLine>*\n");
        printf("  HeaderLine    ::= \"# GROUP\" Name \"(\"LocLon\",\"LocLat\") d \" <LocDist> NL\n");
        printf("  GenomeLine    ::= <ID> <MomID> <DadID> <Gender> <CellID> <Lon> <Lat> <GenomeAsc> NL\n");
        printf("  GenomeAsc     ::= (\"A\" | \"C\" | \"G\" | \"T\" | \" \")\n");
        printf("\n");
    } else {
        printf("INPUT FORMATS");
        printf("---------------------------------------------\n");
        printf("GenomeFile\n");
        printf("---------------------------------------------\n");
        printf("  GenomeFile ::= <header> <dataline>*\n");                                    
        printf("  header     ::= <GenomeSize> <form> <NL>\n");                                
        printf("  form       ::= \"Num\" | \"ACGT\"\n");                                      
        printf("  dataline   ::= <genome> <NL>\n");                                           
        printf("  genome     ::= <long>*                  // if form=\"Num\"\n");             
        printf("  genome     ::= (\"A\"| \"C\" | \"G\" | \"T\")*  // if form=\"ACGT\"\n");    
        printf("  GenomeSize : number of longs if form=\"Num\", else numer of nucleotides\n");
        printf("\n");
        printf("---------------------------------------------\n");
        printf("LocationFile\n");
        printf("---------------------------------------------\n");
        printf("  LocationFile ::= <LocationLine>*\n");     
        printf("  LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
        printf("  identifier   : string (name of location)\n");
        printf("  longitude    : double (longitude in degrees)\n");
        printf("  latitude     : double (latitude in degrees)\n");
        printf("  dist         : double (sample radius in km)\n");
        printf("  num          : int    (number of elements to sample)\n");
        printf("\n");
    }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- POPULATION STUFF ---------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/*
//----------------------------------------------------------------------------
// popInfo
//  callback function for iteration in getFirstPopulation()
//
herr_t popInfo(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        strcpy((char *)opdata, name);
        status = 1;
    }
    return status;
}


//----------------------------------------------------------------------------
// getFirstPopulation
//  create and return name for first population found in QDF file
//
char *getFirstPopulation(const char *pQDF) {
    char *pPop = NULL;

    hid_t hFile = qdf_openFile(pQDF);
    if (hFile > 0) {
        if (H5Lexists(hFile, POPGROUP_NAME, H5P_DEFAULT)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup > 0) {
                char s[64];
                *s = '\0';
                H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfo, s);
                
                if (*s != '\0') {
                    pPop = new char[strlen(s)+1];
                    strcpy(pPop, s);
                } else {
                    printf("No Population subgroup in file [%s]\n", pQDF);
                }
                qdf_closeGroup(hPopGroup);
            }
        } else {
            printf("No Population Group in file [%s]\n", pQDF);
        }
        qdf_closeFile(hFile);
    } else {
        printf("Not a QDF file [%s]\n", pQDF);
    }
    return pPop;
}


//----------------------------------------------------------------------------
// checkForPop
//  create and return name of population if it appears in QDF file
//
char *checkFoPop(const char *pQDF, const char *pName) {
    char *pPop = NULL;

    hid_t hFile = qdf_openFile(pQDF);
    if (hFile >= 0) {
        if (H5Lexists(hFile, POPGROUP_NAME, H5P_DEFAULT)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup >= 0) {
                if (H5Lexists(hPopGroup, pName, H5P_DEFAULT)) {
                    pPop = new char[strlen(pName)+1];
                    strcpy(pPop, pName);
                }
                qdf_closeGroup(hPopGroup);
            }
            qdf_closeFile(hFile);
        } else {
            printf("No Population Group in file [%s]\n", pQDF);
        }
    } else {
        printf("Not a QDF file [%s]\n", pQDF);
    }
    return pPop;
}
*/

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- INITIAL GENOMES ----------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// readGenes
//   read genes from genome file and set them for the progenitors in 
//   GraphEvolver
//
int readGenes(GraphEvolverBase *pGE, idset &sProgenitors, const char *pGenomeFile) {
    int iResult = -1;
    int iGenomeSize = -1;
    LineReader *pLR = LineReader_std::createInstance(pGenomeFile, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        char sForm[16];

        int iRead = sscanf(pLine, "%d %s", &iGenomeSize, sForm);
        iResult = pGE->setGenomeSize(iGenomeSize);
        if (iResult == 0) {
            bool bNuc = false;
            if (iRead == 2) {
                if (strcmp(sForm, "Num") == 0) {
                    bNuc = false;
                    iResult = 0;
                } else if (strcmp(sForm, "ACGT") == 0) {
                    bNuc = true;
                    iResult = 0;
                } else {
                    printf("%sUnknown GeneFile format [%s]\n", RED, sForm);
                    iResult = -1;
                }

                if (iResult == 0) {
                    idset_it it = sProgenitors.begin();
                    pLine = pLR->getNextLine();
                    while ((it != sProgenitors.end()) && (pLine != NULL)) {
                        ulong *pGenome = NULL;
                        if (bNuc) {
                            pGenome = GeneUtils::translateGenome(iGenomeSize, pLine);
                        } else {
                            pGenome = GeneUtils::readGenome(iGenomeSize, pLine);
                        }
                        if (pGenome != NULL) {
                            printf("Setting genome [%p] to agent %ld\n", pGenome, *it);
                            pGE->setGenome(*it, pGenome);
                        }
                        it++;
                        pLine = pLR->getNextLine();
                    }
                }
            } else {
                printf("%sbad header line [%s]\n", RED, pLine);
                iResult = -1;
            }
        } else {
            printf("%sCouldn't set GenomeSize (bad mutation rate or non-positive GenomeSize)\n", RED);
            iResult = -1;
        }
    } else {
        printf("%sCouldn't open [%s] for reading\n", RED, pGenomeFile);
        iResult = -1;
    }
    if (iResult < 0) {
        iGenomeSize = 0;
    }
    return iGenomeSize;
}


//----------------------------------------------------------------------------
// createFullRandomGenes
//   each genome is completely random
//
int createFullRandomGenes(GraphEvolverBase *pGE, const idset &sProgenitors, int iGenomeSize) {
    int iResult = 0;

    printf("creating fully randome genomes of size %d\n", iGenomeSize);
    idset::const_iterator it;
    for (it = sProgenitors.begin(); it != sProgenitors.end(); ++it) {
        const ulong *pGenome = GeneUtils::createFullRandomGenes(iGenomeSize, NULL);
        pGE->setGenome(*it, pGenome);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createFlatRandomGenes
//   all genomes are mutated copies of a random genome
//
int createFlatRandomGenes(GraphEvolverBase *pGE, const idset &sProgenitors, int iGenomeSize, int iNumMutations) {
    int iResult = 0;
    printf("  creating genomes of size %d with %d mutations\n", iGenomeSize, iNumMutations);
    ulong *pG0 = GeneUtils::createRandomAlleles(iGenomeSize, iNumMutations, NULL);
    idset::const_iterator it;
    for (it = sProgenitors.begin(); it != sProgenitors.end(); ++it) {
        ulong *pGenome = GeneUtils::createFlatRandomGenes(iGenomeSize, iNumMutations, pG0);
        pGE->setGenome(*it, pGenome);
    }
    delete[] pG0;
    return iResult;
}


//----------------------------------------------------------------------------
// createRandomGenes
//   
int createRandomGenes(GraphEvolverBase *pGE, const idset &sProgenitors, char *pRandomness) {
    int iResult = -1;
    int iGenomeSize = 0;
 
    char *pType = strtok(pRandomness, ":");
    if (pType != NULL) {
        printf("  Have type [%s]\n", pType);
        char *pParam = strtok(NULL, ":");
        if (pParam != NULL) {
            printf("  Param: [%s]\n", pParam);
            if (strToNum(pParam, &iGenomeSize)) {
                pGE->setGenomeSize(iGenomeSize);
                if (strcmp(pType, "full") == 0) {
                    iResult = createFullRandomGenes(pGE, sProgenitors, iGenomeSize);
                    if (iResult == 0) {
                        iResult = iGenomeSize;
                    }
                } else  if (strcmp(pType, "flat") == 0) {
                    pParam = strtok(NULL, ":");
                    if (pParam != NULL) {
                        int iNumber=0;
                        if (strToNum(pParam, &iNumber)) {
                            iResult = createFlatRandomGenes(pGE, sProgenitors, iGenomeSize, iNumber);
                            if (iResult == 0) {
                                iResult = iGenomeSize;
                            }
                        } else {
                            printf("%sInvalid Number for number of mutations [%s]\n", RED, pParam);
                        }
                    } else {
                        printf("%sExpected numerical parameter (num mutations)\n", RED);
                    }
                } else {
                    printf("%sUnknown type: [%s]\n", RED, pType);
                }
            } else {
                printf("%sInvalid Number for genome size in Randomness [%p]\n", RED, pParam);
            }
        } else {
            printf("%sExpected numerical parameter (genome size)\n", RED);
        }
    } else {
        printf("%sExpected randomness type\n", RED);
    }

    return iResult;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- OUTPUT -------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- TEMPORARY AG FILES -------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// mergeTempFiles
//
int mergeTempFiles(const char *pFileName, const char *pAGOutput, intset &sSavePoints, idset &sSelected, idset &sRoots) {
    int iResult = 0;
    std::vector<std::string> vAGFileNames;
    intset::const_iterator it;
    for (it = sSavePoints.begin(); it != sSavePoints.end(); ++it) {
        char sName[64];
        sprintf(sName, "%s_%04d.ag", pFileName, *it);

        vAGFileNames.push_back(sName);
    }

    AGFileMerger *pAGM = AGFileMerger::createInstance(vAGFileNames, NUM_LOAD_NODES);
    if (pAGM != NULL) {
        
        iResult = pAGM->merge(pAGOutput, MERGE_BUF_SIZE, sSelected, sRoots);

        delete pAGM;
    } else {
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// deleteTempFiles
//
int deleteTempFiles(const char *pFileName, intset &sSavePoints) {
    int iResult = 0;
    std::vector<std::string> vAGFileNames;
    intset::const_iterator it;
    for (it = sSavePoints.begin(); (iResult == 0) && (it != sSavePoints.end()); ++it) {
        char sName[64];
        sprintf(sName, "%s_%04d.ag", pFileName, *it);
        iResult = remove(sName);
        if (iResult != 0) {
            printf("%sCouldn't remove [%s]\n", RED, sName);
            iResult = 0;
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- INPUT UTILITIES ----------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// splitPopFiles
//
char *splitPopFiles(char *pFileName,  stringvec &vPopFileNames) {
    char *pReturn = NULL;
    int iResult = 0;

    printf("splitting pop files\n");
    stringvec vPopNames;
    // split along commas, then each along ":"
    // -> vector of PopNames or NULL
    char *pCtx1;
    char *p = strtok_r(pFileName, ",", &pCtx1);
    while (p != NULL) {
        char *p1 = strchr(p, ':');
        if (p1 != NULL) {
            *p1 = '\0';
            p1++;
        }
        vPopFileNames.push_back(p);
        if (p1 != NULL) {
            vPopNames.push_back(p1);
        } else {
            vPopNames.push_back("");
        }
        p = strtok_r(NULL, ",", &pCtx1);
    }
    if (vPopFileNames.size() > 0) {
        printf("have %zd pop files\n", vPopFileNames.size());
        for (uint i = 0; (iResult == 0) && (i < vPopFileNames.size()); ++i) {
            char *pName = NULL;
            if (vPopNames[i].empty()) {
                printf("Searching popname for [%s]\n", vPopFileNames[i].c_str());
                pName = qdf_getFirstPopulation(vPopFileNames[i].c_str());
            } else {
                printf("Checking popname for [%s]\n", vPopFileNames[i].c_str());
                pName = qdf_checkForPop(vPopFileNames[i].c_str(), vPopNames[i].c_str());
            }
            if (pName != NULL) {
                vPopNames[i] = pName;
                delete[] pName;
            } else {
                iResult = -1;
            }
        }
        if (iResult == 0) {
            std::string &s = vPopNames[0];
            bool bEqual = true;
            for (uint j = 1; j < vPopNames.size(); ++j) {
                if (s.compare(vPopNames[j]) != 0) {
                    bEqual = false;
                }
            }
            if (bEqual) {
                pReturn = new char[vPopNames[0].length()+1];
                strcpy(pReturn, vPopNames[0].c_str());
            }
        }
    } else {
        iResult = -1;
    }
    if (iResult == 0) {
        printf("Determined pop name [%s]\n", pReturn);
    }
    return pReturn;
}

//----------------------------------------------------------------------------
// splitLocSpec
//  the returned locspec is the resopnsability of the caller
//
locspec *splitLocSpec(char *pLocSpec) {
    locspec *pls = NULL;
    char *pFile = strtok(pLocSpec, ":");
    if (pFile != NULL) {
        char *pDist = NULL;
        char *pNum  = NULL;
        pDist = strtok(NULL, ":");
        double dDist = 0;
        uint iNum = 0;
        if (pDist != NULL) {
            dDist = atof(pDist);
            pNum = strtok(NULL, ":");
            if (pNum != NULL) {
                iNum = atoi(pNum);
            }
        }
        pls = new locspec(pFile, dDist, iNum);
    }
    return pls;
}

//----------------------------------------------------------------------------
// getSamplesMulti
//
IDSample *getSamplesMulti(const char *sQDFGrid, stringvec &vQDFPops, const locspec *pLocSpec, const char *pPopName,
                    locdata &mLocData, idset &sSelected, bool bParallelSample) {
    IDSample *pSample = NULL;

    printf("--- creating IDSampler from [%s]\n", sQDFGrid);
    IDSampler2 *pIS = IDSampler2::createInstance(/*momdad NULL, */sQDFGrid);
    if (pIS != NULL) {
        printf("--- getting samples from [");
        for (uint i = 0; i < vQDFPops.size(); ++i) {
            if (i > 0) {
                printf(", ");
            }
            printf("%s", vQDFPops[i].c_str());
        }
        printf("]\n");
        pSample =  pIS->getSamples(vQDFPops, pPopName, pLocSpec, mLocData);
        if (pSample != 0) {
            pSample->getFullIDSet(sSelected);
            printf("Sampled: Total %zd ids\n", sSelected.size());fflush(stdout);
        } else {
            printf("%sCouldn't get samples\n", RED);
        }
        delete pIS;
    } else {
        printf("%sCouldn't create IDSampler for Grid [%s]\n", RED, sQDFGrid);
    }
    return pSample;
}


//----------------------------------------------------------------------------
// getAttributesMulti
//
IDSample *getAttributesMulti(const char *sQDFGrid, stringvec &vQDFPops, const locspec *pLocSpec, const char *pPopName,
                             locdata &mLocData, idset &sSelected) {
    IDSample *pSample = NULL;

    printf("--- creating IDSampler from [%s]\n", sQDFGrid);
    IDSampler2 *pIS = IDSampler2::createInstance(/*momdad NULL, */sQDFGrid);
    if (pIS != NULL) {
        printf("--- getting samples from [");
        for (uint i = 0; i < vQDFPops.size(); ++i) {
            if (i > 0) {
                printf(", ");
            }
            printf("%s", vQDFPops[i].c_str());
        }
        printf("]\n");
        pSample =  pIS->getAttributes(vQDFPops, pPopName, pLocSpec, mLocData, sSelected);
        if (pSample != NULL) {
            pSample->getFullIDSet(sSelected);
            printf("Got attributes for %zd IDs\n", sSelected.size());
        } else {
            printf("%sCouldn't get samples\n", RED);
        }
        delete pIS;
    } else {
        printf("%sCouldn't create IDSampler for Grid [%s]\n", RED, sQDFGrid);
    }
    return pSample;
}
             

//----------------------------------------------------------------------------
// getRelevantLists
//
int getRelevantLists(const char *sAGFile, idset &sSelected, idset &sRoots) {
    int iResult = -1;

    long lListOffset = AncGraphBase::getListOffset(sAGFile);
    FILE *fIn = fopen(sAGFile, "rb");
    if (fIn != NULL) {
        iResult = fseek(fIn, lListOffset, SEEK_SET);
        if (iResult == 0) {
            // BufReader will  to start reading at current position of file
            BufReader *pBR = BufReader::createInstance(fIn, 16384);
            if (pBR != NULL) {
                idset sProgs;
                iResult = AncGraphBase::readIDSetBin(pBR, sProgs);
                if (iResult == 0) {
                    iResult = AncGraphBase::readIDSetBin(pBR, sSelected);
                }
                if (iResult == 0) {
                    iResult = AncGraphBase::readIDSetBin(pBR, sRoots);
                }
                if (iResult == 0) {
                    printf("Retrieved %zd roots and %zd selected from [%s]\n", sRoots.size(), sSelected.size(), sAGFile);
                } else {
                    iResult = -1;
                    printf("%sProblem while reading lists in [%s] at %ld\n", RED, sAGFile, lListOffset);
                }
                delete pBR;
            } else {
                iResult = -1;
                printf("%sCouldn't open BufReader for [%s] at %ld\n", RED, sAGFile, lListOffset);
            }
        } else {
            iResult = 0;
            printf("%sCouldn't move to ListOffset\n", RED);
        }
            
        fclose(fIn);
    } else {
        printf("%sCouldn't open AGFile [%s]\n", RED, sAGFile);
    }
   
    
    return iResult;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//------- MAIN ---------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    time_t tTime = time(NULL);
    printf("Start at %s\n", asctime(localtime(&tTime)));

    ulong ulStartPeak            = 0;
    ulong ulAfterSamplePeak      = 0;
    ulong ulAfterGraphBuildPeak  = 0;
    ulong ulAfterGenomeBuildPeak = 0;
    ulong ulStartUsage            = 0;
    ulong ulAfterSampleUsage      = 0;
    ulong ulAfterGraphBuildUsage  = 0;
    ulong ulAfterGenomeBuildUsage = 0;
    SystemInfo *pSys = SystemInfo::createInstance();
    pSys->showCurrent();
    ulStartPeak  = pSys->getPeakVM();
    ulStartUsage = pSys->getProcUsedVirtualMemory();

    char *sAncFile        = NULL;
    char *sGenomeFile     = NULL;
    char *sQDFPop         = NULL;
    char *sLocFile        = NULL;
    char *sQDFGrid        = NULL;
    char *sRandomness     = NULL;
    char *sOutputFile     = NULL;
    char *sOutputFormat   = NULL;
    char *sAGOutput       = NULL;
    char *sAGInput        = NULL;
    char *sAncOracleRead  = NULL;
    char *sAncOracleWrite = NULL;
    char *sWhich          = NULL;

    int iNumCrossingOver   = DEF_NUM_CROSS;
    double dMutationRate   = DEF_MUT_RATE;
    int iDumpBufferSize    = 0;
    int iAncBlockSize      = ANC_ORACLE_BLOCK_SIZE;
    int iAGOracleBlockSize = AG_ORACLE_BLOCK_SIZE;
    int iRandomSeed = -1;
    bool bKeepFragments = false;
    bool bVerbose = false;
    bool bFileMode = false;
    bool bNiceCleanUp = false;
    bool bCreateAG = true;
    bool bParallelBuild = false;
    bool bParallelSample = false;
    int  iBackReachType = 3;
    int  iGenLatency = 5;
    int  iAncSize = -1;

    double dTime0;
    double dTimeSampling;
    double dTimeKillPoints;
    double dTimeGraphBuild;
    double dTimeGraphEvolve;
    double dTimeWrite;
    double dTimeClean;
    bool bLoadPar = false;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(29,
                                   "--anc-file:S",          &sAncFile,
                                   "--anc-size:S",          &iAncSize,
                                   "--pop-file:S!",         &sQDFPop,
                                   "--grid-file:S!",        &sQDFGrid,
                                   "--location-file:S!",    &sLocFile,
                                   "--genome-file:S",       &sGenomeFile,
                                   "--initial-genes:S",     &sRandomness,
                                   "--crossing-overs:i",    &iNumCrossingOver,
                                   "--mutation-rate:d",     &dMutationRate,
                                   "--output-file:S!",      &sOutputFile,
                                   "--use-file-mode:0",     &bFileMode,
                                   "--output-format:S",     &sOutputFormat,
                                   "--anc-block-size:i",    &iAncBlockSize,
                                   "--anc-size:i",          &iAncSize,
                                   "--dump-buffer-size:i",  &iDumpBufferSize,
                                   "--ag-block-size:i",     &iAGOracleBlockSize,
                                   "--anc-oracle-read:S",   &sAncOracleRead,
                                   "--anc-oracle-write:S",  &sAncOracleWrite,
                                   "--ag-input:S",          &sAGInput,
                                   "--ag-output:S",         &sAGOutput,
                                   "--random-seed:i",       &iRandomSeed,
                                   "-k:0",                  &bKeepFragments,
                                   "--nice-cleanup:0",      &bNiceCleanUp,
                                   "--show-formats:S",      &sWhich, 
                                   "--parallel-build:b",    &bParallelBuild,
                                   "--parallel-samp:b",     &bParallelSample,
                                   "--back-reach-type:i",   &iBackReachType,
                                   "--gen-latency:i",       &iGenLatency,
                                   "--load-par:b",          &bLoadPar);

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                printf("parallel build: %s\n", bParallelBuild?"yes":"no");
                printf("parallel samp:  %s\n", bParallelSample?"yes":"no");
                // sanity checks?
                if ((sGenomeFile == NULL) && (sRandomness == NULL)) {
                    printf("%sEither randomness (-r) or genome file (-g) must be provided\n", RED);
                    iResult = -1;
                }
                if ((sAncFile != NULL) && (iAncSize != 3) && (iAncSize != 4)) {
                    printf("%sAncSize must be 3 or 4\n", RED);
                    iResult = -1;
                }
                
                if (sOutputFormat != NULL) {
                    if (!GeneWriter::formatAccepted(sOutputFormat)) {
                        printf("%sUnknown file format [%s]\n", RED, sOutputFormat);
                        iResult = -1;
                    }
                } else {
                    printf("%sNo output format given\n", RED);
                    iResult = -1;
                }
                
                if ((sAncFile == NULL) == (sAGInput == NULL)) {
                    printf("%sEither AncFile or AGInputFile must be provided\n", RED);
                    iResult = -1;
                }

                if (sAGInput != NULL) {
                    if (bFileMode) {
                        bCreateAG = false;
                    } else {
                        printf("%sUsing sAGInput requires filemode (--use-file-mode)\n", RED);
                        iResult = -1;
                    }
                }
                
                if (bFileMode) {
                    printf("--- Using filemode\n");
                }

                printf("Seed: %d\n", iRandomSeed);
                if (iRandomSeed < 0) {
                    iRandomSeed = (int) time(NULL);
                }
                srand(iRandomSeed);
                    
                // show params
                printf("--- %s called with\n", apArgV[0]);
                pPR->display();

                //--- set AG output file name and temp file name 
                char sAGFragBody[128];
                strcpy(sAGFragBody, "tempfrag");
                char sAGFileName[128];
                if (sAGOutput != NULL) {
                    strcpy(sAGFileName, sAGOutput);
                } else {
                    sprintf(sAGFileName, "%s_full.ag", sAGFragBody);
                }
                
                // fill vPopFileName with the provided population files  
                char *pPopName = NULL;
                std::vector<std::string> vPopFileNames;
                if ((iResult == 0) && (sQDFPop != NULL)) {
                    pPopName = splitPopFiles(sQDFPop, vPopFileNames);
                    if (pPopName == NULL) {
                        printf("%sCouldn't determine population name\n", RED);
                        iResult = -1;
                    }
                }

                
                unsigned char aChecksumRIP[RIP_SIZE];
                //---
                //--- we need the checksum if Oracle read or write is set
                //---
                if (bCreateAG && (iResult == 0) && ((sAncOracleRead != NULL) || (sAncOracleWrite != NULL))) {
                    iResult = ripsum(sAncFile, aChecksumRIP);
                    if (iResult != 0) {
                        printf("%scouldn't get checksum\n", RED);
                    }
                }
                
                idset sSelected;
                idset sRoots;
                
                dTime0 = omp_get_wtime();

                tTime = time(NULL);
                printf("Getting samples at %s (res %d)\n", asctime(localtime(&tTime)), iResult);
               
  
                IDSample *pSample = NULL;
                locdata        mLocData;    // location name -> (lon, lat, dist, num)
                //---
                //--- get sampled IDs with all informations about them
                //---
                if (iResult == 0) {
                    locspec *pls = splitLocSpec(sLocFile);
                    if (pls != NULL) {
                        if (bCreateAG) {
                            pSample = getSamplesMulti(sQDFGrid, vPopFileNames, pls, pPopName,
                                                      mLocData, sSelected, bParallelSample);
                        } else {
                            iResult = getRelevantLists(sAGInput, sSelected, sRoots);
                            
                            pSample = getAttributesMulti(sQDFGrid, vPopFileNames, pls, pPopName, 
                                                         mLocData, sSelected);
                            if (pSample == NULL) {
                                iResult = -1;
                            }
                        }
                        delete pls;
                    }
                }
                

                // at this point mAgentData has ID, gender, cellID and lon and lat, but no parent IDs
                
                //---
                //--- poName is not needed anymore
                //---
                if (pPopName != NULL) {
                    delete[] pPopName;
                }

                ulAfterSamplePeak  = pSys->getPeakVM();
                ulAfterSampleUsage = pSys->getProcUsedVirtualMemory();

                dTimeSampling = omp_get_wtime();
                dTimeKillPoints = omp_get_wtime();
                //---
                //--- extracting set of selected from sample data
                //---
                if (bCreateAG && (iResult == 0)) {
                    // here sSelected should contain all selected IDs
                    
                    if (sSelected.size() > 0) {
                        printf("Selected %zd", sSelected.size());
                        if (true || bVerbose) {
                            printf(": ");
                            idset_cit it2;
                            for (it2 = sSelected.begin(); it2 != sSelected.end(); ++it2) {	
                                printf(" %ld", *it2);
                            }
                        }
                        printf("\n");fflush(stdout);



                    } else {
                        printf("%s*** No agents found in sample regions!\n", RED);
                        iResult = -1;
                    }
                }

                //--- create DynAncGraph from sampled IDs and anc file
                    
                DynAncGraphX *pDAG = NULL;

                if (iResult == 0) {
                    if (bCreateAG) {
                        
                        idset sSelectedCopy(sSelected.begin(), sSelected.end());
                        printf("--- creating DynAncGraph for selected\n");fflush(stdout);
                        if (sAncOracleRead != NULL) {

                            pDAG = DynAncGraphX::createInstance(sAncFile, iAncBlockSize, iAncSize, sAncOracleRead, aChecksumRIP, RIP_SIZE);

                        } else {
                            pDAG = DynAncGraphX::createInstance(sAncFile, iAncBlockSize, iAncSize);
                        }
                        if (pDAG != NULL) {
                            printf("DynAncGraph instantiated\n");fflush(stdout);
                            if (bFileMode) {
                                tTime = time(NULL);
                                printf("Getting killpoints at %s\n", asctime(localtime(&tTime)));fflush(stdout);

                                printf("--- calculating kill points\n");fflush(stdout);
                                iResult = pDAG->calcKillPoints(sSelectedCopy, iBackReachType, iGenLatency);
                                if (iResult !=  0) {
                                    printf("%skill point calculation failed\n", RED);fflush(stdout);
                                }
                                dTimeKillPoints = omp_get_wtime();
                            }
                        
                            if (iResult == 0) {
                                if (sAncOracleWrite != NULL) {
                                    iResult = pDAG->writeOracle(sAncOracleWrite, aChecksumRIP, RIP_SIZE);
                                }
                            }
                        
                            if (iResult == 0) {
                                intset sSavePoints;
                                intset sAllPoints;
                                idset sRootsBackReach;
                                tTime = time(NULL);
                                printf("building ancestor graph at %s\n", asctime(localtime(&tTime)));fflush(stdout);
                                
                                printf("--- building ancestor graph\n");
                                if (iBackReachType > 0) {
                                    switch (iBackReachType) {
                                    case 3:
                                    case 5:
                                        iResult = pDAG->createGraph3(sSelectedCopy, sAGFragBody, sSavePoints);
                                        break;
                                    case 4:
                                    case 6:
                                        iResult = pDAG->createGraph4(sSelectedCopy, sAGFragBody, sSavePoints);
                                        break;
                                    case 7:
                                        iResult = pDAG->createGraph7(sSelectedCopy, sAGFragBody, sSavePoints);
                                        break;
                                    default:
                                        iResult = pDAG->createGraph2(sSelectedCopy, sAGFragBody, sSavePoints);
                                    }
                                    printf("Have %zd roots\n", pDAG->getProgenitors().size());
                                    sRootsBackReach = pDAG->getProgenitors();
                                } else {
                                    iResult = pDAG->createGraph(sSelectedCopy, sAGFragBody, sSavePoints);
                                    printf("Saving DAG [%s]\n", sAGFileName);
                                    printf("  num elements: %zd\n", pDAG->getMap().size());
                                    printf("  num roots: %zd\n",    pDAG->getRoots().size());
                                    printf("  num selected: %zd\n", pDAG->getSelected().size());
                                    fflush(stdout);
                                    iResult = pDAG->saveBin(sAGFileName);
                                }
                            
                                if (iResult == 0) {
                                    if (bFileMode && (iBackReachType > 0)) {

                                        tTime = time(NULL);
                                        printf("merging ancestor graph at %s\n", asctime(localtime(&tTime)));fflush(stdout);
                                        //---
                                        //--- merge temporary AG files
                                        //--
                                        iResult = mergeTempFiles(sAGFragBody, sAGFileName, sSavePoints, sSelected, sRootsBackReach);
                                        if (iResult == 0) {
                                            if (!bKeepFragments) {
                                                int  iRes2 = deleteTempFiles(sAGFragBody, sSavePoints);
                                                if (iRes2 != 0) {
                                                    printf("%sCouldn't remove fragments [%s_NNNN.ag]\n", RED, sAGFragBody);
                                                }
                                            }
                                            //                                            iResult = pDAG->createNodesForIds(sSelected);
                                        }
                                        tTime = time(NULL);
                                        printf("ancestor graph merged at %s\n", asctime(localtime(&tTime)));fflush(stdout);

                                    } else {
                                        // not really necessary - useful for comparison normal/filemode
                                        printf("Saving DAG (no filemode)[%s]\n", sAGFileName);
                                        printf("  num elements: %zd\n", pDAG->getMap().size());
                                        printf("  num roots: %zd\n",    pDAG->getRoots().size());
                                        printf("  num selected: %zd\n", pDAG->getSelected().size());
                                        fflush(stdout);
                                        iResult = pDAG->saveBin(sAGFileName);
                                    }
                                
                                    if (iResult == 0) {
                                        // get the roots 
                                        printf("Collecting %zd roots\n",  pDAG->getRoots().size());fflush(stdout);
                                        sRoots.insert(pDAG->getRoots().begin(), pDAG->getRoots().end());
                    

                                    }
                                
                                } else {
                                    printf("%sCouldn't create graph\n", RED);
                                }
                            }
                       
                        } else {
                            iResult = -1;
                            printf("%sCouldn't create DynAncGraph for [%s]\n", RED, sAncFile);
                        }
                    } else {
                        //--- get parent ids for selected's AgentData
                        strcpy(sAGFileName, sAGInput);
                        pDAG = DynAncGraphX::createInstance();
                    }
                    
                    if (iResult == 0) {
                        AGOracle *pAGO = AGOracle::createInstance(sAGFileName, iAGOracleBlockSize);
                        if (pAGO != NULL) {
                            iResult = pAGO->loadNodes(pDAG, sSelected);
                            if (iResult == 0) {
                                idagd mIDAD;
                                pSample->getIDADMap(mIDAD);
                                printf("--- adding parent IDs to AgentData of selected (%zd)\n", pDAG->getMap().size());fflush(stdout);
                                idset_cit it2;
                                for (it2 = sSelected.begin(); (iResult == 0) && (it2 != sSelected.end()); ++it2) {	
                                    AncestorNode *pAN = pDAG->findAncestorNode(*it2);
                                    if (pAN != NULL) {
				        agdata *pAD = mIDAD[*it2];
					if (pAD != NULL) {
                                            pAD->iMomID = pAN->getMom();
                                            pAD->iDadID = pAN->getDad();
                                            pAN->m_bSelected = true;
					} else {
                                            printf("%sCouldn't find AG data for node %ld\n", RED, *it2);
					  printf("probable cause%s:\n", bCreateAG?"":"s");
					  printf("  ancdata was not created for specified pop file\n");
					  if (!bCreateAG) {
					      printf("  current location file different from the one used to create the agent graph\n");
					  }
					  iResult = -1;
					}
                                    } else {
                                        printf("%sNo ancestornode for ID %ld\n", RED, *it2);
                                        iResult = -1;
                                    }
                                }
                            } else {
                                printf("%sCouldn't load ancestor nodes from [%s]\n", RED, sAGFileName);
                                iResult = -1;
                            }
                            delete pAGO;
                        } else {
                            printf("%sCouldn't create AGOracle for [%s]\n", RED, sAGFileName);
                            iResult = -1;
                        }
                    }
                }
                    
                fflush(stdout);
                ulAfterGraphBuildPeak  = pSys->getPeakVM();
                ulAfterGraphBuildUsage = pSys->getProcUsedVirtualMemory();

                dTimeGraphBuild = omp_get_wtime();
                printf("wtime after graphbuild: %f\n", dTimeGraphBuild);                     

                GraphEvolverF *pGEF = NULL;
                if (iResult == 0) {
                    int iGenomeSize = 0; 
                    if (bFileMode) {

                        printf("--- reading GraphEvolver from [%s]\n", sAGFileName);fflush(stdout);
                        pGEF = GraphEvolverF::create(pDAG, sAGFileName, iAGOracleBlockSize, -1, iNumCrossingOver, dMutationRate);
                    } else {
                        printf("--- creating GraphEvolver for DynAncGraph\n");fflush(stdout);
                        pGEF = GraphEvolverF::create(pDAG, -1, iNumCrossingOver, dMutationRate);
                    }
                    
                    if (pGEF != NULL) {
                        pGEF->setParLoad(bLoadPar);
                        // initial genes for progenitors
                        if (sGenomeFile != NULL) {
                            printf("---  reading initial genes...\n");fflush(stdout);
                            iGenomeSize = readGenes(pGEF, sRoots, sGenomeFile);
                        } else {
                        //random genes
                            printf("---  creating random genes for progenitors...\n");fflush(stdout);
                            iGenomeSize = createRandomGenes(pGEF, sRoots, sRandomness);
                        }
                        if (iGenomeSize == 0) {
                            printf("%sError while setting progenitor genes\n", RED);
                            iResult = -1;
                        }
                    
                        if (iResult == 0) {

                            tTime = time(NULL);
                            printf("calculating genomes at %s\n", asctime(localtime(&tTime)));fflush(stdout);

                            printf("--- calculating genomes...\n");fflush(stdout);
                            if (bFileMode) {
                                if (iDumpBufferSize == 0) {
                                    iDumpBufferSize = GENOME_SAVE_BUFFER_SIZE;
                                }

                                if (bParallelBuild) {
                                    iResult = pGEF->buildGenomePar(iDumpBufferSize, sRoots, sSelected);
                                } else {
                                    iResult = pGEF->buildGenomeSeq(iDumpBufferSize, sRoots, sSelected);
                                }
                            } else {
                                if (iDumpBufferSize > 0) {
                                    printf("  Calculating buffered (%d)\n", iDumpBufferSize);fflush(stdout);
                                    iResult = pGEF->buildGenome(iDumpBufferSize, sRoots, sSelected);
                                } else {
                                    printf("  Calculating top down (recursive)\n");fflush(stdout);
                                    iResult = pGEF->calcGenomes(sSelected);
                                }
                            }
                            if (iResult != 0) {
                                printf("%sGenome building failed\n", RED);
                            }
                        }
                    } else {
                        printf("%sCouldn't create GraphEvolver (bad mutation rate?)\n", RED);
                    }

                }
                ulAfterGenomeBuildPeak  = pSys->getPeakVM();
                ulAfterGenomeBuildUsage = pSys->getProcUsedVirtualMemory();

                dTimeGraphEvolve = omp_get_wtime();
                printf("wtime after graphevolve: %f\n", dTimeGraphEvolve);                     

                if (iResult == 0) {
                    tTime = time(NULL);
                    printf("writing genomes at %s\n", asctime(localtime(&tTime)));
                    printf("--- writing output ...\n");fflush(stdout);
                    /*@@@ single pop 
                    iResult = GeneWriter::writeGenes(sOutputFormat, pGEF, sOutputFile, mvIDs, mLocData, mAgentData);
                    @@@*/
                    /*@@@ multi pop*/
                    iResult = GeneWriter::writeGenes(sOutputFormat, pGEF, sOutputFile, mLocData, pSample);
                    /*@@@*/

                    if (iResult == 0) {
                        printf("%s  Written output OK\n", GREEN);
                    } else {
                        printf("%sFailed to write output\n", RED);
                    }
                } else {
                    printf("%sCouldn't calculate Genomes\n", RED);
                }

                dTimeWrite = omp_get_wtime();
                printf("wtime after write: %f\n", dTimeWrite);                     
                
              
                if (bNiceCleanUp) {
                    printf("cleaning up at %s\n", asctime(localtime(&tTime)));
                    printf("--- cleaning up\n"); fflush(stdout);
                    
                    delete pGEF;
                    delete pSample;


                    // GE has been using AG, can only delete it here
                    delete pDAG;
                }                    
                dTimeClean = omp_get_wtime();

                if (iResult == 0) {
                    printf("T0:         %fs\n", dTime0);
                    printf("Sampling:   %fs\n", dTimeSampling - dTime0);
                    printf("KillPoints: %fs\n", dTimeKillPoints - dTimeSampling);
                    printf("Building:   %fs\n", dTimeGraphBuild - dTimeKillPoints);
                    printf("Evolving:   %fs\n", dTimeGraphEvolve- dTimeGraphBuild);
                    printf("Writing:    %fs\n", dTimeWrite - dTimeGraphEvolve);
                    printf("Cleaning:   %fs\n", dTimeClean - dTimeWrite);
                    printf("Total:      %fs\n", omp_get_wtime() - dTime0);
                    printf("+++ success +++\n");
                    fflush(stdout);
                }

            } else {
                if (sWhich != NULL) {
                    if (strcmp(sWhich, "out") == 0) {
                        showFormats(true);
                    } else if (strcmp(sWhich, "in") == 0) {
                        showFormats(false);
                    } else {
                        printf("Use \"in\" or \"out\" for option '--show-formats'\n");
                    }
                } else {
                    usage(apArgV[0]);
                }
            }
        } else {
            printf("%sCouldn't set ParamReader options\n",RED);
        }
        delete pPR;
    } else {
        printf("%sCouldn't create ParamReader\n", RED);
    }
    pSys->showCurrent();
    printf("Memory in use / peak memory:\n");
    printf("Init:               %lu/%lu\n", ulStartUsage, ulStartPeak);
    printf("After Sample:       %lu/%lu\n", ulAfterSampleUsage, ulAfterSamplePeak);
    printf("After Graph Build:  %lu/%lu\n", ulAfterGraphBuildUsage, ulAfterGraphBuildPeak);
    printf("After Genome Build: %lu/%lu\n", ulAfterGenomeBuildUsage, ulAfterGenomeBuildPeak);

    delete pSys;
    return iResult;
}
