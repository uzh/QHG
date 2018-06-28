#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include "strutils.h"
#include "ranges.h"
#include "ParamReader.h"
#include "TempRainBiome.h"
//#include "ImageDisplay.h"


const char *SUFFIX_IMG = ".img";
const char *SUFFIX_DOC = ".doc";

float EURO_LAT_MIN =   30.0f;
float EURO_LAT_MAX =   60.0f;
float EURO_LON_MIN =  -15.0f;
float EURO_LON_MAX =   40.0f;


static const char *SUFF_BIO   = "_bio";
static const char *SUFF_CHK   = "_chk";
static const char *SUFF_LEG   = "_leg.png";


void Usage(char *pName) {
    printf("%s -t [-b][-c]\n", pName);
    printf("   --temp-file=<tempfile> --prec-file=<precfile> --biom-file=<biomfile>]\n");
    printf("   --temp-bin=<tempbin> --prec-bin=<precbin> [-n] \n");
    printf("   [--trans-file=<transfile>]\n");
    printf("   [--biom-doc=<biometable>] [--smooth=<smoothing>]\n");
    printf("   -o <outputfile> [-v] [-V] \n");
    printf("or\n");
    printf("%s -b [-l] --trb-table=<trbtable>\n", pName);
    printf("   [--temp-file=<tempfile>] [--prec-file=<precfile>]\n");
    printf("   [--trans-file=<transfile>]\n");
    printf("   -o <outputfile> [-v] [-V] \n");
    printf("where\n");
    printf("  -t           create temperature-rain-biome table\n");
    printf("  -b           create biome map with given rain/temp and possibly temp-shift/rain-scale\n");
    printf("  -c           check: create biome map using table and same rain/temp (only together with -t)\n");
    
    printf("  tempfile     QMap file with temperature data\n");
    printf("  precfile     QMap file with precipitation data\n");
    printf("  biomfile     QMap file with biome data\n");
    printf("  tempbin      size of temperature bins\n");
    printf("  deltaT       unfiorm temperature shift\n");
    printf("  precbin      size of precipitation bins\n");
    printf("  kR           uniform precipitation factor\n");
    printf("  -n           extend precipitations to include 0 (if necessary)\n");
    printf("  -l           draw legend for entries in biome table\n");
    printf("  biometable   QMap file of biomes\n");
    printf("  trbtable     QMap file of temperature-rain-biome relations (as created with '-t')\n");
    printf("  transfile    translation file(e.g., to transform holdridge life zones to simplified biomes\n");
    printf("  outputfile   body for output files\n");
    printf("               trb outputfile will be       <output>_trb.qmap\n");
    printf("               recreated biome file will be <output>_rec.qmap\n");
    printf("  -v           verbose mode\n");
    printf("  -V           display images (trb-table, biome map, legend)\n");

    printf("The biome table is a simple file assigning uchar values to names:\n");
    printf("  biometable ::= <line>*\n");
    printf("  line       ::= category <n> : <n> <name>\n");
    printf("  n         : number (0-255)\n");
    printf("  name      : biome name\n");
    printf("(all lines not starting with 'category' are ignored)\n");

    printf("The translation file is a simple file which maps uchar values to uchar values:\n");
    printf("  transfile ::= <line>*\n");
    printf("  line      ::= <comment> | <entry>*\n");
    printf("  comment   ::= \"#\" <char>* <NL>\n");
    printf("  entry     ::= <from> <to><NL>\n");
    printf("  from      : number (0-255)\n");
    printf("  to        : number (0-255)\n");
    printf("  char      : character\n");
    printf("Examples:\n");
    printf("  %s -t -r --temp-file=temp_today.qmap --prec-file=precip_today.qmap  --biom-file=lholdag_f.qmap --temp-bin=0.125 --prec-bin=1  --trans-file=trans_LH2QHG.txt   -o qtoday -v\n", pName);

    printf("%s -r --temp-file=tempalt.qmap --prec-file=rainpadx.qmap --temp-bin=0.5 --prec-bin=5 -o trn1 -v --trans-file=../data/trans_LH2QHG.txt --trb-table=trn1_trb.qmap -r --temp-shift=-4\n", pName);
}

int main(int iArgC, char *apArgV[]) {
    
    char sTempFile[MAX_PATH];
    char sPrecFile[MAX_PATH];
    char sBiomFile[MAX_PATH];
    char sBiomDoc[MAX_PATH];
    char sTransFile[MAX_PATH];
    char sTempFile0[MAX_PATH];
    char sPrecFile0[MAX_PATH];
    char sBiomFile0[MAX_PATH];
    char sBiomDoc0[MAX_PATH];
    char sTransFile0[MAX_PATH];

    char sOutputFile[MAX_PATH];
    char sTRBTable[MAX_PATH];
    char sTRBTable0[MAX_PATH];

    float fTempBin = fNaN;
    float fPrecBin = fNaN;

    bool bVerbose      = false;
    bool bPExtend      = false;
    bool bVisualize    = false;
    bool bLegend       = false;
    bool bCreateTable  = false;
    bool bCreateBiomes = false;
    bool bCheck        = false;
    int  iSmoothness   = 0;
    float fDeltaTemp   = 0;
    float fScaleRain   = 1;

    *sTempFile0    = '\0';
    *sPrecFile0    = '\0';
    *sBiomFile0    = '\0';
    *sBiomDoc0     = '\0';
    *sOutputFile   = '\0';
    *sTransFile0   = '\0';
    *sTRBTable0    = '\0';

    int iResult = -1;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(19,  
                               "--temp-file:s",        sTempFile0,
                               "--prec-file:s",        sPrecFile0,
                               "--biom-file:s",        sBiomFile0,
                               "--biom-doc:s",         sBiomDoc0,
                               "-o:s!",                sOutputFile,
                               "--temp-bin:f",         &fTempBin,
                               "--prec-bin:f",         &fPrecBin,
                               "-n:0",                 &bPExtend,
                               "-v:0",                 &bVerbose,
                               "-t:0",                 &bCreateTable,
                               "-b:0",                 &bCreateBiomes,
                               "-c:0",                 &bCheck,
                               "-l:0",                 &bLegend,
                               "--trb-table:s",        sTRBTable0,
                               "--trans-file:s",       sTransFile0,
                               "--smooth:i",           &iSmoothness,
                               "--temp-shift:f",       &fDeltaTemp,
                               "--rain-scale:f",       &fScaleRain,
                               "-V:0",                 &bVisualize
                               );
                       
    if (bOK) {     
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (bCreateTable || bCreateBiomes) {

                char *pDataDir = getenv("DATA_DIR");
                searchFile(sTempFile0,  pDataDir, sTempFile);
                searchFile(sPrecFile0,  pDataDir, sPrecFile);
                searchFile(sBiomFile0,  pDataDir, sBiomFile);
                searchFile(sBiomDoc0,   pDataDir, sBiomDoc);
                searchFile(sTransFile0, pDataDir, sTransFile);
                searchFile(sTRBTable0,  pDataDir, sTRBTable);
                
                

                bool bOK = false;
                if ((bCreateTable && (*sTempFile != '\0') &&(*sPrecFile != '\0') && (*sBiomFile != '\0')) ||
                    (bCreateBiomes && (*sTempFile != '\0') &&(*sPrecFile != '\0') && (*sTRBTable != '\0'))) {
                    bOK = true;
                }
                if (bOK) {
                    if ((*sBiomDoc == '\0') && (*sBiomFile != '\0')) {
                        strcpy(sBiomDoc, sBiomFile);
                        char *p = strstr(sBiomDoc, SUFFIX_IMG);
                        printf("p:%s, s:%s, p - s:%ld\n", p, sBiomDoc, p-sBiomDoc);
                        if ((p != NULL) && (strlen(sBiomDoc)-(p-sBiomDoc) == strlen(SUFFIX_IMG))) {
                            *p = '\0';
                            strcat(sBiomDoc, SUFFIX_DOC);
                        } else {
                            *sBiomDoc = '\0';
                        }
                    }
                        
                    if (bCreateTable) {
                        printf("Create temp-rain-biome table%s%s\n", bCheck?" with test":"", bCreateBiomes?" and recreate":"");
                    } else {
                        printf("Recreate biome map\n");
                    }
                    printf("Parameters:\n");
                    printf("  Temperature file:   %s\n", sTempFile);
                    printf("  Precipitation file: %s%s\n", sPrecFile, (bPExtend?"(extend to include 0)":""));
                    printf("  Biome file:         %s\n", sBiomFile);
                        
                    printf("  Output files:\n");
                    printf("    Temp-Rain-Biome table  %s%s\n", sOutputFile, SUFF_TRBQ);
                    //                    printf("    Final TempRain map     %s%s\n", sOutputFile, SUFF_TRBI);
                    if (bCheck) {
                        printf("    Check biome image  %s%s.png\n", sOutputFile, SUFF_CHK);
                        printf("    Check biome map    %s%s.qmap\n", sOutputFile, SUFF_CHK);
                    }
                    if (bCreateBiomes) {
                        printf("    New biome image        %s%s.png\n", sOutputFile, SUFF_BIO);
                        printf("    New biome map          %s%s.qmap\n", sOutputFile, SUFF_BIO);
                        if (fDeltaTemp != 0) {
                            printf("    Temperature shift      %f\n", fDeltaTemp);
                        }
                        if (fScaleRain != 1) {
                            printf("    Rain scaling           %f\n", fScaleRain);
                        }
                                    
                    }

                    if (bCreateTable) {
                        printf("  Temperature bin size   %f\n", fTempBin);
                        printf("  Precipitation bin size %f\n", fPrecBin);
                    }
                    printf("  Translation file    %s\n", sTransFile); 
                        
                    if (*sBiomDoc != '\0') {
                        printf("  Biome doc file:     %s%s\n", sBiomDoc, (*sBiomDoc == '\0')?"(no legend)":"");
                        printf("    Legend            %s%s\n", sOutputFile, SUFF_LEG);
                    }
                        
                    printf("\n");
                        
                    TempRainBiome *pTRB = new TempRainBiome();
                        
                    if (bCreateTable) {
                        if (isnan(fTempBin) || isnan(fPrecBin)) {
                            printf("***Need bins to create  table\n");
                            bOK = false;
                        } else {
                            bOK = pTRB->init(sTempFile, sPrecFile, sBiomFile);
                                
                            if (bOK) {
                                printf("Init ok\n");
                                pTRB->process();
                                    
                                pTRB->makeTable(fTempBin, fPrecBin, bPExtend, iSmoothness, sOutputFile, sTransFile, bVerbose);
                                /*
                                if (bVisualize) {
                                        
                                    char sName[64];
                                    sprintf(sName, "%s%s",sOutputFile, SUFF_TRBI);
                                    ImageDisplay::displayImage(sName);
                                }
                                */
  
                                if (bCheck) {
                                    pTRB->checkFiles(sTempFile, sPrecFile, sOutputFile, SUFF_CHK);
                                    /*
                                    if (bVisualize) {
                                        char sName[64];
                                        sprintf(sName, "%s%s.png",sOutputFile, SUFF_CHK);
                                        ImageDisplay::displayImage(sName);
                                    }
                                    */
                                }
                                    
                                if (bCreateBiomes) {
                                    pTRB->checkFiles(sTempFile, sPrecFile, sOutputFile, SUFF_BIO, fDeltaTemp, fScaleRain);
                                    /*
                                    if (bVisualize) {
                                        char sName[64];
                                        sprintf(sName, "%s%s.png",sOutputFile, SUFF_BIO);
                                        ImageDisplay::displayImage(sName);
                                    }
                                    */
                                }
                                    
                            }
                        }
                    } else if (bCreateBiomes) {
                        bOK = pTRB->init(sTempFile, sPrecFile);
                        if (bOK) {
                            printf("recreating\n");
                            //pTRB->getExtremes();
                            pTRB->createBiomes(sTRBTable, sOutputFile, SUFF_BIO, fDeltaTemp, fScaleRain);
                            /*
                            if (bVisualize) {
                                char sName[64];
                                sprintf(sName, "%s%s.png",sOutputFile, SUFF_BIO);
                                ImageDisplay::displayImage(sName);
                            }
                            */
                        }
                    }
                    /*
                    if (bLegend && (*sBiomDoc != '\0')) {
                        char sName[64];
                        sprintf(sName, "%s%s",sOutputFile, SUFF_LEG);
                        pTRB->makeLegend(sBiomDoc, sName);

                        if (bVisualize) {
                            ImageDisplay::displayImage(sName);
                        }

                    }
                    */
                    if (bOK) {
                        printf("+++ success +++\n");
                        iResult = 0;
                    } else {
                        printf("init failed\n");
                        iResult = -1;
                    }
                    delete pTRB;
                } else {
                    if (bCreateTable) {
                        printf("require Temp-file, Prec-file and Biome-file\n");
                    } else {
                        printf("require Temp-file, Prec-file and TRB-file\n");
                    }
                }
            } else {
                printf("No action (-t or -b) given\n");
            }
        } else {
            Usage(apArgV[0]);
        }

    } else {
        printf("ParamReader option error\n");
    }
    delete pPR;


    return iResult;
}
