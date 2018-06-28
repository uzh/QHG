#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "geomutils.h"
#include "Vec3D.h"


#include "AltiTemp.h"
#include "GradiTemp.h"

#include "ValReader.h"
#include "QMapUtils.h"

const char *TEMP_FILE = "__TEMP_TOP__";
const double TEMP_AMPL= 2.0;

int doAltitude(ValReader *pVRAltitude, ValReader *pVRTemperature, char *pOutput, double dSeaLevel, bool bAccumulateTemp) {
    int iResult = -1;
    AltiTemp *pAT = new AltiTemp(pVRAltitude, pVRTemperature);
    if (pAT->isReady()) {
        iResult = pAT->adjustTemperature(dSeaLevel, bAccumulateTemp);
        if (iResult == 0) {
            iResult = pAT->writeToFile(pOutput);
        }
    }
    delete pAT;
    return iResult;
}

int doGradient(ValReader *pVRAltitude, ValReader *pVRTemperature, char *pOutput, double dAmplitude, bool bAccumulateTemp) {
    int iResult = -1;
    GradiTemp *pGT =  new GradiTemp(pVRAltitude, pVRTemperature);
    if (pGT->isReady()) {
        
        iResult = pGT->adjustTemperature(dAmplitude, bAccumulateTemp);
        if (iResult == 0) {
            iResult = pGT->writeToFile(pOutput);
        }
    }
    delete pGT;
    return iResult;
}




int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    bool bDoGrad    = false;
    bool bDoAlt     = false;
    bool bShowDiff  = false;
    bool bBadParams = true;
    double dSeaLevel = 0;
    char sOutput1[SHORT_INPUT];
    char sOutput2[SHORT_INPUT];

    if (iArgC > 4) {
        char *p = apArgV[1];
        if (*p == '-') {
            p++;
            bDoGrad   = (strchr(p, 'g') != NULL);
            bDoAlt    = (strchr(p, 'a') != NULL);
            bShowDiff = (strchr(p, 'd') != NULL);
            int iStart = 1;
            bool bOK = true;
            if (strchr(p, 'h') != NULL) {
                bOK = false;
                if (iArgC > 5) {
                    iStart++;
                    char *pEnd;
                    dSeaLevel = strtod(apArgV[iStart], &pEnd);
                    if (*pEnd == '\0') {
                        bOK = true;
                    }
                }
            }
            if (bOK && (bDoGrad || bDoAlt)) {
                if (bDoGrad && bDoAlt) {
                    strcpy(sOutput1, TEMP_FILE);
                    strcpy(sOutput2, apArgV[iStart+3]);
                } else {
                    strcpy(sOutput1, apArgV[iStart+3]);
                    strcpy(sOutput2, apArgV[iStart+3]);
                }

                bBadParams = false;
                printf("Altitude file:    %s\n",apArgV[iStart+1]); 
                printf("Temperature file: %s\n",apArgV[iStart+2]); 
                printf("Output file:      %s\n",apArgV[iStart+3]);
                printf("Action:           %s%s%s,%s\n", bDoAlt?"Altitude":"", (bDoAlt&&bDoGrad)?",":"", bDoGrad?"Gradient":"", bShowDiff?"difference only":"added to temp");
                printf("                  Sealevel %f\n", dSeaLevel);
                iResult = 0;
                ValReader *pVRAltitude = QMapUtils::createValReader(apArgV[iStart+1], true); // no interpolation
                if (pVRAltitude != NULL) {
                    ValReader *pVRTemperature = QMapUtils::createValReader(apArgV[iStart+2], true); // no interpolation
                    if (pVRTemperature != NULL) {
                        if ((iResult == 0) && bDoAlt) {
                            iResult = doAltitude(pVRAltitude, pVRTemperature, sOutput1, dSeaLevel, !bShowDiff);
                            if (bDoGrad) {
                                delete pVRTemperature;
                                pVRTemperature = QMapUtils::createValReader(sOutput1, true);
                            }
                        }
                        if ((iResult == 0) && bDoGrad) {
                            iResult = doGradient(pVRAltitude, pVRTemperature, sOutput2, TEMP_AMPL, !bShowDiff);
                        }
                        
                        if (bDoGrad && bDoAlt) {
                            remove(TEMP_FILE);
                        }


                        delete pVRTemperature;
                    } else {
                        printf("Couldn't open temp file [%s] for reading\n", apArgV[iStart+2]);
                    }
                    delete pVRAltitude;
                } else {
                    printf("Couldn't open alt file [%s] for reading\n", apArgV[iStart+1]);
                }

                if (iResult == 0) {
                    printf("++++ Success ++++\n");
                } else {
                    printf("---- Failure ----\n");
                }

            }
        }
    }

    if (bBadParams) {
        printf("%s - add altitude temperature decrease to temp map\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <opts> <AltitudeMapFile> <TemperatureMapFile> <OutputFile>\n", apArgV[0]);
        printf("or\n");
        printf("  %s <opts2> <sealevel> <AltitudeMapFile> <TemperatureMapFile> <OutputFile>\n", apArgV[0]);
        printf("where\n");
        printf("opts:  -g[d] | -a[d] | -ag[d] | -ga[d]\n");
        printf("opts2: -gh[d] | -ah[d] | -agh[d] | -gah[d]\n");
        printf("        g:  adjust temperature for gradient\n");
        printf("        a:  adjust temperature for altitude\n");
        printf("        ag, ga: adjust temperature for gradient and temperature\n");
        printf("        d: output temperature difference only\n");
        printf("        h:  use next parameter as sealevel\n");
    }
    return iResult;
}
