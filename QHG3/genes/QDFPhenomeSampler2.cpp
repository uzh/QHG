#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "PheneWriter2.h"
#include "QDFPhenomeExtractor2.h"
#include "QDFSequenceExtractor.cpp"


#define ATTR_PHENOME_SIZE  "Phenetics_phenome_size"
#define DATASET_PHENOME    "Phenome"
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
    printf("      [--attr-phenome-size=<NameAttrPhenomeSize>]\n");
    printf("      [--dataset-phenome=<NameDSPhenome>]\n");
    printf("      [--phenomes-per-buf=<NumPhenomesPerBuf>]\n");
    printf("      [-c]\n");
    printf("where\n");
    printf("  QDFPopFile           QDF Population file with genome info\n");
    printf("  SpeciesName          Species name ( if omitted, first species will be used)\n");
    printf("  OutputName           Name body for output files\n");
    printf("  format               output format; one of \"bin\" and/or \"asc\"\n");
    printf("  Locationfile         name of location file (format: see below)\n");
    printf("  QDFGeoFile           QDF grid file \n");
    printf("  --dense              use if selected genomes dense in all genomes\n");
    printf("  RefLocationfile      name of location file for reference genome (format: see below)\n");
    printf("  NameAttrPhenomeSize  name of the phenome size attribute in the QDF file (default \"%s\")\n", ATTR_PHENOME_SIZE);
    printf("  NameDSPhenome        name of the phenome data set in the QDF file (default \"%s\")\n", DATASET_PHENOME);
    printf("  NumPhenomesPerBuf    determines size of buffer: NumPhenomesPerBuf*2*phenomesize (default 1000000)\n");
    printf("  phrase               arbitrary sequence of characters to seed random generator (use quotes if pPhrase contains spaces) (default: [%s])\n", DEFAULT_PHRASE);
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
        if (strcmp(p, FORMAT_BIN) == 0) {
            iWhat |= OUT_BIN;
        }  else if (strcmp(p, FORMAT_ASC) == 0) {
            iWhat |= OUT_ASC;
        }
        p = strtok(NULL, ":");
    }

    return iWhat;
}


//----------------------------------------------------------------------------
// writeOutput
//
int writeOutput(QDFPhenomeExtractor2 *pQPE, const char *pOutputBody, int iWhat, bool bRef) {
    int iResult = 0;

    bool bVerbose = false;

    char sOutAsc[256];
    char sOutBin[256];

    *sOutAsc = '\0';
    *sOutBin = '\0';

    int iErr = 0;

    // get locations
    const loc_data    &mLocDefs = pQPE->getLocData();

    // get agent data
    const IDSample *pSample =  NULL;
    if (bRef) {
        pSample = pQPE->getRefSample();
    } else {
        pSample = pQPE->getSample();
    }

    if ((iWhat & OUT_BIN) != 0) {
        // create name
        sprintf(sOutBin, "%s.%sbin", pOutputBody, bRef?"ref.":"");
        if (bVerbose) printf("writing %s\n", sOutBin);
        int iResult1 = PheneWriter2::writeSequence(FORMAT_BIN, pQPE, sOutBin, mLocDefs, pSample, false); // false: reduced output
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
        int iResult1 = PheneWriter2::writeSequence(FORMAT_ASC, pQPE, sOutAsc, mLocDefs, pSample, false); // false: reduced output
        if (iResult1 != 0) {
            iResult += iResult1;
            iErr |=  OUT_ASC;
        }
        printf("asc file [%s] written\n", sOutAsc);
    }


    if (iResult != 0) {
        fprintf(stderr, "writing failed for ");
        if ((iErr & OUT_BIN) != 0) {
            fprintf(stderr, "[%s] ", sOutBin);
        } 
        if ((iErr & OUT_ASC) != 0) {
            fprintf(stderr, "[%s] ", sOutAsc);
        } 
        fprintf(stderr, "\n");
         
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
    char *pAttPhen = NULL;
    char *pDSPhen  = NULL;
    char *pRefLoc = NULL;
    int  iNumPhenomesPerBuf = -1;
    bool bQuiet      = false;
    char *pMapFile   = NULL;
    bool bDense      = false;
    char *pPhrase    = NULL;
    bool bCartesian  = false;
    char *pInSamp    = NULL;
    char *pOutSamp   = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(17,
                                   "-i:S!",  &pPopFile,
                                   "-g:S",   &pGeoFile,
                                   "-s:S",   &pSpecies,
                                   "-o:S!",  &pOutputBody,
                                   "-f:S",   &pFormatList,
                                   "--location-file:S!",   &pLocationFile,
                                   "--ref-location:S",     &pRefLoc,
                                   "--attr-phenome-size:S", &pAttPhen,
                                   "--dataset-phenome:S",   &pDSPhen,
                                   "--phenomes-per-buf:i",  &iNumPhenomesPerBuf,
				   "--write-index-id-map:S", &pMapFile,
                                   "--dense:0", &bDense, 
                                   "-c:0", &bCartesian, 
                                   "--seed:S", &pPhrase,
                                   "--read-samp:S",         &pInSamp,
                                   "--write-samp:S",        &pOutSamp,
                                   "-q:0", &bQuiet);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                if (!bQuiet) pPR->display();

                // set defaults for unset string  params
                char *pAttPhen2 = defaultIfNULL(pAttPhen, ATTR_PHENOME_SIZE);
                char *pDSPhen2  = defaultIfNULL(pDSPhen,  DATASET_PHENOME);
                char *pPhrase2  = defaultIfNULL(pPhrase,  DEFAULT_PHRASE);
 
                iResult = -1;
                QDFPhenomeExtractor2 *pQPE = NULL;
                WELL512 *pWELL = createWELL(pPhrase2);

                if (pWELL != NULL) {
                    if (pGeoFile != NULL) {
                        pQPE = QDFPhenomeExtractor2::createInstance(pGeoFile, pPopFile, pSpecies, pAttPhen2, pDSPhen2, pWELL, bCartesian);
                    } else {
                        pQPE = QDFPhenomeExtractor2::createInstance(pPopFile, pSpecies, pAttPhen2, pDSPhen2, pWELL, bCartesian);
                    }
                }
                bool bHasRef = (pRefLoc != NULL);

                iResult = pQPE->createSelection(pLocationFile, pRefLoc, bDense, iNumPhenomesPerBuf, pInSamp, pOutSamp);
    
                if (iResult == 0) {
                    if (!bQuiet) {
                        printf("selected %d ids:", pQPE->getNumSelected());
                        const idset &sSelected = pQPE->getSelectedIDs();
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

                    if (pFormatList != NULL) {
                        iWhat = getFormats(pFormatList);
                    }
                    iResult = writeOutput(pQPE, pOutputBody, iWhat, false); //false: not a reference
                    if (iResult == 0) {
                        if (pMapFile != NULL) {
                            iResult = writeIndexIDMap(pQPE->getIndexIDMap(), pMapFile);
                        }    
                    } else {
                        fprintf(stderr, "failed writing outputs\n");
                    }
                    
                    if (bHasRef && (iResult == 0)) {
                        iResult = writeOutput(pQPE, pOutputBody, iWhat, true);
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
                delete[] pDSPhen2;
                delete[] pAttPhen2;
                delete   pQPE;
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
