#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "utils.h"
#include "strutils.h"
#include "ParamReader.h"
#include "GRegion.h"
#include "GRegionizer.h"





//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - test regionizer\n", pApp);
    printf("usage:\n");
    printf("  %s -g <gridfile> [-p <popfile>] [-s <numsteps>] -m <mode>\n", pApp);
    printf("     -i <numregions>[\":\"<id>(\",\"<id>)*\n");
    printf("where\n");
    printf("  gridfile   a qdf file containing a grid group, and,\n");
    printf("             optionally, a population group\n");
    printf("  popfile    a qdf file containing a population group for the grid\n");
    printf("             if the '-p' option is missing, gridfile must have a population\n");
    printf("  mode       a string describing the selection mode:\n");
    printf("             \"all\" | \"rand\" | \"smallest\" | \"largest\"\n");
    printf("  numsteps   number of growth steps to perform\n");
    printf("             if the '-s' option is missing, growth steps\n");
    printf("             are repeated until no more changes occur\n");
    printf("  initials   how to initialize the regions.\n");
    printf("               numregions  number of regions to create\n");
    printf("               id          any cell id\n");
    printf("             if no ids are given, each region is filled with\n");
    printf("             one random cell id.\n");
    printf("             otherwise, numregions*k must be given, and then\n");
    printf("             then ids #i*k to #(i+1)*k-1 are assigned to region i\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -g BigGrid.qdf -p BigPop.qdf -s 10 -m all -i 6\n", pApp);
    printf("     Use BigGrid.qdf for the grid and BigPop.qdf for the poulations\n");
    printf("     Perform 10 steps where all neighbors are added to the region\n");
    printf("     with the least number of agents. Initialize 6 region with one\n");
    printf("     randomly chosen cell id\n");
    printf("\n");
    printf("  %s -g BigGrid.qdf -m largest -i 3:0,2,4,6,8,10\n", pApp);
    printf("     Use BigGrid.qdf for both grid and populations. Create 3 regions\n");
    printf("     and fill them with ids 0 and 2, 4 and 6, 8 and 10, respectively\n");
    printf("     At each step, add the neighbor with the most agents to the region\n");
    printf("     with the least number of agents. Repeat until nothing changes anymore\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// getMode
//   
int getMode(char *sMode) {
    int iMode = 0;
    
    if (strcasecmp(sMode, "all") == 0) {
        iMode = MODE_ALL;
    } else if (strcasecmp(sMode, "rand") == 0) {
        iMode = MODE_RAND;
    } else if (strcasecmp(sMode, "smallest") == 0) {
        iMode = MODE_SMALLEST;
    } else if (strcasecmp(sMode, "largest") == 0) {
        iMode = MODE_LARGEST;
    } 

    return iMode;
}
   
//----------------------------------------------------------------------------
// getInits
//   the initialisations string can either be
//     <numregions>
//   or
//     <numregions>:<id1>,...<idn>
//   In the first case the regions are initialized with a single 
//   randomly picked cell id.
//   In the second case, the provided ids are used to initialize 
//   the regions
//
int *getInits(char *sMode, int *piNumRegions, int *piNumInitials) {
    int *pInits = NULL;
    char *p = strchr(sMode, ':');
    if (p != NULL) {
        *p = '\0';
        p++;
    }
    int iNR;
    if (strToNum(sMode, &iNR)) {
        *piNumRegions = iNR;
        if (p != NULL) {
            // use a set to eliminate duplicates
            std::set<int> v;
            bool bOK = true;
            char *p1 = strtok(p, ",");
            while (bOK && (p1 != NULL)) {
                int i = 0;
                bOK = strToNum(p1, &i);
                if (bOK) {
                    v.insert(i);
                    p1 = strtok(NULL, ",");
                }
            }
            
            div_t q = div(v.size(), iNR);
            if (q.rem == 0) {
                pInits = new int[v.size()];
                std::set<int>::const_iterator it;
                int i = 0;
                for (it = v.begin(); it != v.end(); ++it) {
                    pInits[i++] = *it;
                }
                *piNumInitials = q.quot;
            } else {
                printf("Number of regions (%d) should divide number of initializers (%zd)\n", iNR, v.size());
                printf("(maybe there were duplicate integers in the initializers)\n");
            }
        } else {
            *piNumInitials = 1;
        }
    } else {
        printf("Invalid number of regions: [%s]\n", sMode);
    }
    return pInits;
}

//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    /*
    char sGridFile[MAX_PATH];
    char sPopFile[MAX_PATH];
    char sMode[64];
    char sInitials[MAX_PATH];
    char sOutput[MAX_PATH];
    */
    char *sGridFile = NULL;
    char *sPopFile  = NULL;
    char *sMode     = NULL;
    char *sInitials = NULL;
    char *sOutput   = NULL;

    int  iNumSteps = 0;   
    /*
    *sGridFile = '\0';
    *sPopFile  = '\0';
    *sMode     = '\0';
    *sInitials = '\0';
    *sOutput   = '\0';
    */
    int iMode   = 0;
    int *pInits = NULL;
    int iNumRegions  = 0;
    int iNumInitials = 0;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,  
                               "-g:S!",   &sGridFile,
                               "-p:S",    &sPopFile,
                               "-m:S!",   &sMode,
                               "-s:i",    &iNumSteps,
                               "-i:S!",   &sInitials,
                               "-o:S",    &sOutput);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (*sPopFile == '\0') {
                strcpy(sPopFile, sGridFile);
            }
            iMode = getMode(sMode);
            if (iMode <= 0) {
                iResult = -1;
                printf("Invalid mode [%s]\n", sMode);
            }
            if (iResult == 0) {
                pInits = getInits(sInitials, &iNumRegions, &iNumInitials);
                printf("getInits: numregions %d numinits %d\n", iNumRegions, iNumInitials);
            } else {
                iResult = -1;
                printf("Invalid initialisation string [%s]\n", sInitials);
            }
            if ((iResult == 0) && (iNumRegions >0) && (iNumInitials > 0)) {
                GRegionizer *pGR = GRegionizer::createInstance(sGridFile, sPopFile, NULL);
                if (pGR != NULL) {
                    iResult = pGR->initializeRegions(iNumRegions, pInits, iNumInitials);
                    if (iResult == 0) {
                        printf("Doing %d steps with mode %d\n", iNumSteps, iMode);
                        printf("---- step 0 ----\n");
                        pGR->showRegions();
                        double dTot1 = omp_get_wtime();
                        int i = 0;
                        for (i = 0; (iResult == 0) && ((iNumSteps == 0) || (i < iNumSteps)); i++) {
                            iResult = pGR->growStep(iMode);
                            /*                            
                            if ((i+1)%100 == 0) {
                                printf("---- step %d ----\n", i);
                                pGR->showRegions();
                            }
                            */
                        }
                        dTot1 = omp_get_wtime() - dTot1;
                        if (iResult == 2) {
                            printf("no more neighbors\n");
                            iResult = 0;
                        }
                        printf("---- finally (%d steps) ----\n", i);
                        pGR->showRegions();
                        printf("NF  Time : %f\n", pGR->getNFTime());
                        printf("Tot Time : %f\n", dTot1);
                        if (*sOutput != '\0') {
                            pGR->writeRegions(sOutput);
                        }
                        printf("+++ success +++\n");
                    } else {
                        printf("Couldn't initialize GRegionizer\n");
                    }
                    delete pGR;
                } else {
                    iResult = -1;
                    printf("Couldn't create GRegionizer\n");
                }
            }


            delete[] pInits;
            
        } else {
            usage(apArgV[0]);
        }
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
    
    return iResult;
}



