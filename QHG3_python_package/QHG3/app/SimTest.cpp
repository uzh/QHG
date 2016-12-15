#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "ParamReader.h"
#include "strutils.h"
#include "colors.h"
#include "MessLogger.h"
#include "Simulator.h"
#include "StatusWriter.h"


#define OPT_LOG "--log-file="

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        char sLogFile[256];
        strcpy(sLogFile, DEF_LOG_FILE);
        // is a log-file set?
        for (int i = 0; i < iArgC; i++) {
            if (strstr(apArgV[i], OPT_LOG) == apArgV[i]) {
                char *p = strchr(apArgV[i], '=');
                if (p != NULL) {
                    p++;
                    strcpy(sLogFile, p);
                }
                    
            }
        }
        MessLogger::create(sLogFile);
        time_t t = time(NULL);
        LOG_DISP("-----------------------------------------\n");
        LOG_DISP("Starting Logfile %s\n", trim(ctime(&t)));
        LOG_DISP("-----------------------------------------\n");

        Simulator *pSim = new Simulator();
        iResult = pSim->readOptions(iArgC, apArgV);

        if (iResult == 0) {
            
            pSim->showInputs();
            iResult = pSim->startLoop();
            if (iResult == 0) {
                //            iResult = pSim->writeState(apArgV[3+iOffs], WR_POP);
                if (iResult == 0) {
                    printf("%s+++success+++%s\n", GREEN, OFF);
                }
            } else {
                printf("%serror in loops%s\n", RED, OFF);
            }
            
        } else {
            if (iResult == 3) {
                printf("%susage%s: %d\n", RED, OFF, PARAMREADER_ERR_MANDATORY_MISSING);
                printf("%s", RED);
                Simulator::helpParams();
                printf("%s\n", OFF);
            } else if (iResult == 2) {
                Simulator::showTopicHelp(pSim->getHelpTopic());
            }
        }
        delete pSim;
       
    } else {
        printf("%susage%s\n", RED, OFF);
        printf("%s", RED);
        Simulator::helpParams();
        printf("%s\n", OFF);
        
    }
    return iResult;
}
