#include <stdio.h>

#include "ParamReader.h"

#include "utils.h"
#include "TempRainCap.h"

void usage(char *pName) {
    printf("%s - creating or checking temperature-rain-capacity relations\n", pName);
    printf("Usage:\n");
    printf("  %s -t <tempfile> -p <precfile> -o <outputbody>\n", pName);
    printf("   -c <capfile> \n");
    printf("   --temp-bin=<tempbin> --prec-bin=<precbin> \n");
    printf("   -r <initialrad> -k <radiusfactor> \n");
    printf("   [-T <tempdiff>] [-P <precdiff>]\n");
    printf("or\n");
    printf("  %s -t <tempfile> -p <precfile> -o <outputbody>\n", pName);
    printf("   -a <avgfile> [-s <stdfile>])\n");
    printf("   [-D | -M]\n");
    printf("   [-T <tempdiff>] [-P <precdiff>]\n");
    printf("where\n");
    printf("  tempfile     qmap of temperatures\n");
    printf("  precfile     qmap of precipitaion\n");
    printf("  outputbody   name body for output files\n");
    printf("  capfile      qmap of capacities or capacity densities\n");
    printf("  tempbin      size of temperature bins\n");
    printf("  precbin      size of precipitation bins\n");
    printf("  initialrad   initial search radius to fill empty cells\n");
    printf("  radfactor    factor by which search radius is multiplied at each fill iteration\n");
    printf("  avgfile      a capacity average file created by %s -c\n", pName);
    printf("  stdfile      a capacuty standard dev file created by %s -c\n", pName);
    printf("  -D           calculate density qmap (check)\n");
    printf("  -M           calculate mass qmap (check)\n");
    printf("  tempdiff     temperature difference to be added to tempfile values\n");
    printf("  precdiff     precipitation difference to be added to precfile values\n");
    printf("\n");
    printf("Acceptable values for radius and factor are 2 and 1.1 to 1.5 (very slow), 5 and 1.0 or 1.1\n");
    printf("The QMaps must have equal size.\n");
    printf("The output will consist of two files, \\n");
    printf("  <outputbody>_avgc.qmap and <outputbody>_stdc.qmap\n");
    printf("The former contains average capacity for each (temperature,rain) combination\n");
    printf("The latter contains standard deviation of capacity for each (temperature,rain) combination\n");
    printf("\n");
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char sTempFile[LONG_INPUT];
    char sPrecFile[LONG_INPUT];
    char sCapacityFile[LONG_INPUT];
    char sAverageFile[LONG_INPUT];
    char sStdDevFile[LONG_INPUT];
    char sOutputBody[LONG_INPUT];
    double dTempBin  = dNaN;
    double dPrecBin  = dNaN;
    *sTempFile       = '\0';
    *sPrecFile       = '\0';
    *sCapacityFile   = '\0';
    *sAverageFile    = '\0';
    *sStdDevFile     = '\0';
    *sOutputBody     = '\0';
    int iRadius      = -1;
    double dRadIncr  = 1;
    bool bCheckDens  = false;
    bool bCheckMass  = false;
    double dTempDiff = 0;
    double dPrecDiff = 0;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(14,  
                               "-t:s!",         sTempFile,
                               "-p:s!",         sPrecFile,
                               "-c:s",          sCapacityFile,
                               "-a:s",          sAverageFile,
                               "-s:s",          sStdDevFile,
                               "-o:s!",         sOutputBody,
                               "--temp-bin:d",  &dTempBin,
                               "--prec-bin:d",  &dPrecBin,
                               "-r:i",          &iRadius,
                               "-k:d",          &dRadIncr,
                               "-D:0",          &bCheckDens,
                               "-M:0",          &bCheckMass,
                               "-T:d",          &dTempDiff,
                               "-P:d",          &dPrecDiff);

    if (bOK) {     
        iResult = pPR->getParams(iArgC, apArgV);
        
        if (iResult >= 0) {
            if ((*sCapacityFile != '\0') || (*sAverageFile != '\0')) {
                TempRainCap *pTPC = new TempRainCap();
                if (pTPC != NULL) {
                    
                    if (*sCapacityFile != '\0') {
                        if (!isnan(dTempBin) && !isnan(dPrecBin) && (iRadius > 0)) {
                            iResult = pTPC->setFiles(sTempFile, sPrecFile, sCapacityFile);
                            if (iResult == 0) {
                                pTPC->setDiffs(dTempDiff, dPrecDiff);
                                iResult = pTPC->calculate(dTempBin, dPrecBin);
                                if (iResult == 0) {
                                    int iK = 0;
                                    double dRadius = iRadius;
                                    int iEP =pTPC->getNumEmpty()+1; 
                                    while ((iK < 300) && (pTPC->getNumEmpty() != iEP)) {
                                        iEP = pTPC->getNumEmpty();
                                        iResult = pTPC->fill(dRadius);
                                        iK++;
                                        dRadius *= dRadIncr;
                                    }
                                    if (iResult == 0) {
                                        iResult = pTPC->writeFiles(sOutputBody);
                                    }
                                }
                            }
                            sprintf(sAverageFile, "%s_avgc.qmap", sOutputBody);
                            sprintf(sStdDevFile,  "%s_stdc.qmap", sOutputBody);
                        } else {
                            printf("Option -c requires specification of --temp-bin and --prec-bin and -r\n");
                        }
                    } else {
                        iResult = pTPC->setFiles(sTempFile, sPrecFile);  
                        pTPC->setDiffs(dTempDiff, dPrecDiff);
                    }
                    
                    
                    
                    if ((iResult == 0) && (bCheckDens || bCheckMass)) {
                        char sCheck[LONG_INPUT];
                        sprintf(sCheck, "%s_check.qmap", sOutputBody);
                        pTPC->checkResult(sAverageFile, sCheck, bCheckMass);
                    }
                    
                    delete pTPC;
                } else {
                    printf("Couldn't initalize TempRainCap\n");
                }
            } else {
                iResult =-1;
                printf("Must specify -c or -a\n");
            }
        } else {
            iResult =-1;
            usage(apArgV[0]);
        }
    } else {
        printf("ParamReader option error\n");
    }

    delete pPR;
    if (iResult ==0) {
        printf("+++ success +++\n");
    }
    return iResult;
}

