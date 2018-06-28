#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "GeneWriter2.h"
#include "QDFGenomeExtractor2.h"
#include "QDFSequenceExtractor.cpp"


#define ATTR_GENOME_SIZE  "Genetics_genome_size"
#define ATTR_BITS_PER_NUC "Genetics_bits_per_nuc"
#define DATASET_GENOME    "Genome"
#define DEFAULT_PHRASE    "just a random phrase 1 2 3!?"

#define OUT_BIN   1
#define OUT_ASC   2
#define OUT_PLINK 4
#define OUT_NUM   8


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - Extract genome samples from a QDF population file\n", pApp);
    printf("Usage:\n");
    printf("%s -i <QDFPopFile> [-s <SpeciesName>] -o <OutputName> [-f <format>(\":\"<format>)*]\n", pApp);
    printf("      --location-file=<LocationFile>\n");
    printf("      [-g <QDFGeoFile>]\n");
    printf("      [--seed=<phrase>]\n");
    printf("      [--ref-location=<RefLocationFile>\n");
    printf("      [--dense]\n");
    printf("      [--bit-nucs]\n");
    printf("      [--attr-genome-size=<NameAttrGenomeSize>]\n");
    printf("      [--dataset-genome=<NameDSGenome>]\n");
    printf("      [--genomes-per-buf=<NumGenomesPerBuf>]\n");
    printf("      [-c]\n");
    printf("where\n");
    printf("  QDFPopFile   QDF Population file with genome info\n");
    printf("  SpeciesName  Species name ( if omitted, first species will be used)\n");
    printf("  OutputName   Name for output files\n");
    printf("  format               output format; one of \"ped\", \"bin\", \"asc\", and/or \"num\"\n");
    printf("  Locationfile         name of location file (format: see below)\n");
    printf("  QDFGeoFile           QDF grid file \n");
    printf("  phrase               arbitrary sequence of characters to seed random generator (use quotes if pPhrase contains spaces) (default: [%s])\n", DEFAULT_PHRASE);
    printf("  --dense              use if selected genomes dense in all genomes\n");
    printf("  --bit-nucs           use Bit-Nucleotides instead of normal 2-Bit Nucleotides\n");
    printf("  RefLocationfile      name of location file for reference genome (format: see below)\n");
    printf("  NameAttrGenomeSize   name of the genome size attribute in the QDF file (default \"%s\")\n", ATTR_GENOME_SIZE);
    printf("  NameDSGenome         name of the genome data set in the QDF file (default \"%s\")\n", DATASET_GENOME);
    printf("  NumGenomesPerBuf     determines size of buffer: NumGenomesPerBuf*2*NumLongspergenome (default 1000000)\n");
    printf("  -c                   use cartesian instead of spherical distances (default false)\n");
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
// getFormats
//
int getFormats(char *pFormatList) {
    int iWhat = 0;

    char *p = strtok(pFormatList, ":");
    while (p != NULL) {
        if (strcmp(p, FORMAT_PLINK) == 0) {
            iWhat |= OUT_PLINK;
        }  else if (strcmp(p, FORMAT_BIN) == 0) {
            iWhat |= OUT_BIN;
        }  else if (strcmp(p, FORMAT_ASC) == 0) {
            iWhat |= OUT_ASC;
        }  else if (strcmp(p, FORMAT_NUM) == 0) {
            iWhat |= OUT_NUM;
        }
        p = strtok(NULL, ":");
    }

    return iWhat;
}


//----------------------------------------------------------------------------
// writeOutput
//
int writeOutput(QDFGenomeExtractor2 *pQGE, const char *pOutputBody, int iWhat, bool bRef, bool bBitNucs) {
    int iResult = 0;

    bool bVerbose = false;

    char sOutAsc[256];
    char sOutBin[256];
    char sOutPed[256];
    char sOutMap[256];
    char sOutNum[256];

    *sOutAsc = '\0';
    *sOutBin = '\0';
    *sOutPed = '\0';
    *sOutMap = '\0';
    *sOutNum = '\0';

    int iErr = 0;

    // get locations
    const loc_data    &mLocDefs = pQGE->getLocData();

    // get agent data
    const IDSample *pSample =  NULL;
    if (bRef) {
        pSample = pQGE->getRefSample();
    } else {
        pSample = pQGE->getSample();
    }

    if ((iWhat & OUT_BIN) != 0) {
        // create name
        sprintf(sOutBin, "%s.%sbin", pOutputBody, bRef?"ref.":"");
        if (bVerbose) printf("writing %s\n", sOutBin);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_BIN, pQGE, sOutBin, mLocDefs, pSample, false, bBitNucs); // false: reduced output
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_BIN;
        }
        printf("bin file [%s] written\n", sOutBin);
    }
    if ((iWhat & OUT_ASC) != 0) {
        // create name
        sprintf(sOutAsc, "%s.%sasc", pOutputBody, bRef?"ref.":"");
        if (bVerbose) printf("writing %s\n", sOutAsc);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_ASC, pQGE, sOutAsc, mLocDefs, pSample, false, bBitNucs); // false: reduced output
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_ASC;
        }
        printf("asc file [%s] written\n", sOutAsc);
    }

    if ((iWhat & OUT_NUM) != 0) {
        // create name
        sprintf(sOutNum, "%s.%sdat", pOutputBody, bRef?"ref.":"");
        if (bVerbose) printf("writing %s\n", sOutNum);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_NUM, pQGE, sOutNum, mLocDefs, pSample, true, bBitNucs); // true: add ID as frist col
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_NUM;
        }
        printf("num file [%s] written\n", sOutNum);
    }

    // write genes to ped file
    if ((iWhat & OUT_PLINK) != 0) {
        // create name
        sprintf(sOutPed, "%s.%sped", pOutputBody, bRef?"ref.":"");
        if (bVerbose) printf("writing %s\n", sOutPed);
        int iResult1 = GeneWriter2::writeSequence(FORMAT_PLINK, pQGE, sOutPed, mLocDefs, pSample, false, bBitNucs); // false is ignored
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_PLINK;
        } else {

            printf("ped file [%s] written\n", sOutPed);
            
            // create name for map file
            sprintf(sOutMap, "%s.%smap", pOutputBody, bRef?"ref.":"");
            if (bVerbose) printf("writing %s\n", sOutMap);
            // write map file
            iResult1 = GeneUtils::writePlinkMapFile(sOutMap, pQGE->getSequenceSize());
            if (iResult1 != 0) {
                iResult += iResult1;
                iErr |=  OUT_PLINK;
            } else {
                printf("map file [%s] written\n", sOutMap);
            }
        }
    }

    if (iResult != 0) {
        fprintf(stderr, "writing failed for ");
        if ((iErr & OUT_BIN) != 0) {
            fprintf(stderr, "[%s] ", sOutBin);
        } 
        if ((iErr & OUT_ASC) != 0) {
            fprintf(stderr, "[%s] ", sOutAsc);
        } 
        if ((iErr & OUT_PLINK) != 0) {
            fprintf(stderr, "[%s], [%s]", sOutPed, sOutMap);
        } 
        if ((iErr & OUT_NUM) != 0) {
            fprintf(stderr, "[%s] ", sOutNum);
        } 
        fprintf(stderr, "\n");
         
        // we don't care about the return value of remove
        remove(sOutPed);
        remove(sOutMap);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeIndexIDMap
//
int writeIndexIDMap(const arrpos_ids &mSelected, const char *pMapOut) {
     int iResult = -1;
     FILE *f = fopen(pMapOut, "wt");
     if (f != NULL) {
          arrpos_ids::const_iterator it;
	  for (it = mSelected.begin(); it != mSelected.end(); ++it) {
              fprintf(f, "%u %lu\n", it->first, it->second);
	  }
	  fclose(f);
	  iResult = 0;
     } else {
         fprintf(stderr, "Couldn't open file [%s] for writing\n", pMapOut);
	 iResult = -1;
     }
     return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char *pPopFile = NULL;
    char *pGeoFile = NULL;
    char *pSpecies = NULL;
    char *pOutputBody = NULL;
    char *pFormatList = NULL;
    char *pLocationFile = NULL;
    char *pAttGen = NULL;
    char *pAttBitsPerNuc = NULL;
    char *pDSGen  = NULL;
    char *pRefLoc = NULL;
    int  iNumGenomesPerBuf = -1;
    bool bQuiet      = false;
    char *pMapFile   = NULL;
    bool bDense      = false;
    bool bCartesian  = false;
    char *pPhrase    = NULL;
    char *pInSamp    = NULL;
    char *pOutSamp   = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(18,
                                   "-i:S!",  &pPopFile,
                                   "-g:S",   &pGeoFile,
                                   "-s:S",   &pSpecies,
                                   "-o:S!",  &pOutputBody,
                                   "-f:S",   &pFormatList,
                                   "--location-file:S!",    &pLocationFile,
                                   "--ref-location:S",      &pRefLoc,
                                   "--attr-genome-size:S",  &pAttGen,
                                   "--dataset-genome:S",    &pDSGen,
                                   "--genomes-per-buf:i",   &iNumGenomesPerBuf,
				   "--write-index-id-map:S", &pMapFile,
                                   "--dense:0", &bDense, 
				   "--attr-bits-per-nuc:S", &pAttBitsPerNuc, 
                                   "-c:0",                  &bCartesian, 
                                   "--seed:S",              &pPhrase,
                                   "--read-samp:S",         &pInSamp,
                                   "--write-samp:S",        &pOutSamp,
                                   "-q:0", &bQuiet);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (!bQuiet) pPR->display();
               
                // set defaults for unset string  params
                char *pAttGen2        = defaultIfNULL(pAttGen,        ATTR_GENOME_SIZE);
                char *pAttBitsPerNuc2 = defaultIfNULL(pAttBitsPerNuc, ATTR_BITS_PER_NUC);
                char *pDSGen2         = defaultIfNULL(pDSGen,         DATASET_GENOME);
                char *pPhrase2        = defaultIfNULL(pPhrase,        DEFAULT_PHRASE);
                
                iResult = -1;
                QDFGenomeExtractor2 *pQGE = NULL;
                WELL512 *pWELL = createWELL(pPhrase2);

                if (pWELL != NULL) {
                    if (pGeoFile != NULL) {
                        pQGE = QDFGenomeExtractor2::createInstance(pGeoFile, pPopFile, pSpecies, pAttGen2, pAttBitsPerNuc2, pDSGen2, pWELL, bCartesian);
                    } else {
                        pQGE = QDFGenomeExtractor2::createInstance(pPopFile, pSpecies, pAttGen2, pAttBitsPerNuc, pDSGen2, pWELL, bCartesian);
                    }
                }
                pQGE->setVerbose(!bQuiet);
                bool bHasRef = (pRefLoc != NULL);

                iResult = pQGE->createSelection(pLocationFile, pRefLoc, bDense, iNumGenomesPerBuf, pInSamp, pOutSamp);
    
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
                    int iWhat = OUT_BIN;
                    bool bBitNucs = (pQGE->getBitsPerNuc()==1);


                    if (pFormatList != NULL) {
                        iWhat = getFormats(pFormatList);
                    }
                    iResult = writeOutput(pQGE, pOutputBody, iWhat, false, bBitNucs); //false: not a reference
                    if (iResult == 0) {
                        if (pMapFile != NULL) {
                            iResult = writeIndexIDMap(pQGE->getIndexIDMap(), pMapFile);
                        }    
                    } else {
                        fprintf(stderr, "failed writing outputs\n");
                    }
                    
                    if (bHasRef && (iResult == 0)) {
                        iResult = writeOutput(pQGE, pOutputBody, iWhat, true, bBitNucs);
                        if (iResult == 0) {
                            //ok
                        } else {
                            fprintf(stderr, "couldn't create reference selection\n");
                        }
                    }
                }



                if (iResult == 0) {
                    printf("+++ success +++\n");
                }

               
                delete[] pPhrase2;
                delete[] pDSGen2;
                delete[] pAttGen2;
                delete[] pAttBitsPerNuc2;
                delete   pQGE;
                delete   pWELL;
                
                
                
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
