#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <omp.h>

#include <hdf5.h>

#include "EventConsts.h"
#include "MessLogger.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "ids.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "GeoGroupReader.h"
#include "ClimateGroupReader.h"
#include "VegGroupReader.h"
#include "QDFUtils.h"
#include "StatusWriter.h"

#include "SCellGrid.h"

#include "PopBase.h"
#include "SPopulation.h"
#include "PopLooper.h"
#include "EventManager.h"

#include "L2List.h" //@@ (for now) for DUMP_MODE_XXX

#include "SimParams.h"
#include "Simulator.h"

#define GENOMESIZE 20

//----------------------------------------------------------------------------
// constructor
//
Simulator::Simulator()
    : SimParams(),
      m_iLastCommTime(-1),
      m_bInterrupt(false)  {

}


//----------------------------------------------------------------------------
// destructor
//
Simulator::~Simulator() {

}


//----------------------------------------------------------------------------
// isReady
//   - make sure there is a grid
//   - make sure there is a poplooper
//   - make sure there ae populations
//   - create IDGens and pass them to the pops
//
bool Simulator::isReady() {
    bool bResult = false;
    if ((m_pCG != NULL) && (m_pPopLooper != NULL) && (m_pPopLooper->getNumPops() > 0)) {
        bResult = true;

        //@@ NOTE: for MPI apps we have to get:
        //         the global Max id, 
        //         the total number of threads,
        //         the total number of threads in the "preceding" nodes
        
        int    iNumThreads = omp_get_max_threads();
        int    iOffsBase   = 0;
        idtype iMaxID      = m_pPopLooper->getMaxID();
        printf("Setting Base for IDGens: %ld\n", iMaxID);
        printf("NOTE: for MPI apps we have to get:\n");
        printf("         the global Max id,\n"); 
        printf("         the total number of threads,\n");
        printf("         the total number of threads in the \"preceding\" nodes\n");

        printf("have %d threads\n", omp_get_max_threads());

        // only set IDGen data if we're not resuming
        // (restoreSpeciesDataQDF() sets the current values of the IDGens)
        if (!m_bResume) {
            for (int iT = 0; iT < iNumThreads; iT++) {
                m_apIDG[iT]->setData(iMaxID+1, iOffsBase+iT, iNumThreads);
            }      
        }

    } else {
        printf("No CellGrid or no Populations: can't start Simulation loop\n");
        LOG_ERROR("No CellGrid or no Populations: can't start Simulation loop\n");
    }
    return bResult;
}


//----------------------------------------------------------------------------
// startLoop
//
int Simulator::startLoop() {
    int iResult = 0;
    
    iResult = preLoop();
    
    if (iResult == 0) {
        iResult = runLoop();
    } else {
        printf("[Simulator] Error during preLoop()\n");
        LOG_ERROR("[Simulator] Error during preLoop()");
    }

    postLoop();
    return iResult;
}


//----------------------------------------------------------------------------
// preLoop
//   - create an instance of status writer
//
int Simulator::preLoop()  {
    int iResult = -1;
    if (isReady()) {
        // fast forward events to current start time
        // one more in case of resume, because those things have been handled in the original run
        m_pEM->forwardTo(m_fStartTime+(m_bResume?1:0));

        // call the preloop methods of the pops
        iResult = 0;
        for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
            iResult = m_pPopLooper->getPops()[j]->preLoop();
        }
        m_pSW = StatusWriter::createInstance(m_pCG, m_pPopLooper->getPops());

    } else {
        printf("Not ready to run:\n");
        if (m_pCG == NULL) {
            printf("  no Grid\n");
        }
        if ((m_pPopLooper == NULL) || (m_pPopLooper->getNumPops() == 0)) {
            printf("  no Populations\n");
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// checkEvents
//
void Simulator::checkEvents(float fTime) {
    // check for events
    if (m_pEM != NULL) {
        if (m_pEM->hasNewEvent(fTime)) {
            std::vector<EventData*> vEDSet;
            vEDSet = m_pEM->getEventData();
            for (unsigned int i = 0; i < vEDSet.size(); i++) {
                int iOK = processEvent(vEDSet[i]);
                if (iOK != 0) {
                    printf("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData.c_str());
                    LOG_WARNING("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData.c_str());
                }
            }
        }
    }
}


//----------------------------------------------------------------------------
// checkFinalEvents
//
void Simulator::checkFinalEvents(float fTime) {
    // check for events
    printf("[Simulator::checkFinalEvents] checking final events\n");
    if (m_pEM != NULL) {
        m_pEM->findFinalEvents();
        std::vector<EventData*> vEDSet;
        vEDSet = m_pEM->getEventData();
        printf("[Simulator::checkFinalEvents] have %zd final events\n", vEDSet.size());
        for (unsigned int i = 0; i < vEDSet.size(); i++) {
            int iOK = processEvent(vEDSet[i]);
            if (iOK != 0) {
                printf("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData.c_str());
                LOG_WARNING("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData.c_str());
            }
        }
    }
}


//----------------------------------------------------------------------------
// runLoop
//
int Simulator::runLoop() {
    int iResult = 0;

    double dEnd   = 0;
#ifdef OMP_A
    printf("-------OMPA VERSION---------\n");
#else
#ifdef OMP_C
    printf("-------OMPC VERSION---------\n");
#else
    printf("-------OMP0 VERSION---------\n");
#endif
#endif
    printf("---------------------------\n");
    printf("------ starting loop ------\n");
    printf("------ %d iterations \n",m_iNumIters);
    printf("---------------------------\n");

    if (m_bResume) {
        m_iCurStep = m_fStartTime;
    } else {
        m_iCurStep = 0;
    }
    int iCompletedIterations = 0;

    checkEvents((float)m_iCurStep); // later maybe m_fCurTime

    LOG_STATUS("Number of agents before starting\n");
    const popvec &vp0 = m_pPopLooper->getPops();
    for (uint i = 0; i < vp0.size(); i++) {
        LOG_STATUS("  %s: %d\n", vp0[i]->getSpeciesName(), vp0[i]->getNumAgentsEffective());
    }
    
    LOG_STATUS("Starting Simulation ...\n");
    LOG_STATUS("-----\n");
    double dStart = omp_get_wtime();

    // start value for m_iCurStep set above
    while ((m_iCurStep < m_iNumIters) && (!m_bInterrupt)) {

        fprintf(stderr, "Step %03d ---------------------------\n", m_iCurStep);

        double dStart2 = omp_get_wtime();
        m_pPopLooper->doStep((float)m_iCurStep);
        double dEnd2 = omp_get_wtime();
        //        printf("step used: %f s\n", dEnd2-dStart2);

        m_iCurStep++;

        // count total number of agents
        ulong iTotalTotal = 0;
        if (m_pPopLooper->getNumPops() > 0) {
            for (uint j = 0; j < m_pPopLooper->getNumPops(); j++) {
                iTotalTotal +=  m_pPopLooper->getPops()[j]->getNumAgentsEffective();
            }
        }
        
        printf("After step %d (%f s): total %lu agents\n", m_iCurStep, dEnd2-dStart2, iTotalTotal);


        if (m_pCG->m_pVegetation != NULL) {
            m_pCG->m_pVegetation->resetUpdated();
        } 
        if (m_pCG->m_pClimate != NULL) {
            m_pCG->m_pClimate->resetUpdated();
        }
        if (m_pCG->m_pGeography != NULL) {
            m_pCG->m_pGeography->resetUpdated();
        }
        // save the stats for this step
        if (m_pCG->m_pMoveStats != NULL) {
            m_pCG->m_pMoveStats->insertNewStats(m_iCurStep);
        }
        
        // check the events
        checkEvents((float)m_iCurStep);
 
        // notify end of events
        for (uint j = 0; j < m_pPopLooper->getNumPops(); j++) {
            m_pPopLooper->getPops()[j]->flushEvents((float)m_iCurStep);
        }
 
        iCompletedIterations = m_iCurStep;

        if (iTotalTotal == 0) {
            // avoid crash by stopping simulation if no agents survive
            checkFinalEvents((float)m_iCurStep);
            m_iCurStep = m_iNumIters;
        }
        
        fflush(stdout);

        if (m_bInterrupt) {
            printf("\n");
            printf("Creating dump\n");
            handleDumpEvent("SIG_INT");
            printf("Gracefully exiting\n");
        }
    }
    dEnd = omp_get_wtime();
    

    double dTimeUsed =  dEnd - dStart;
    int iHoursUsed = (int)(dTimeUsed/3600);
    dTimeUsed -= iHoursUsed*3600;
    int iMinutesUsed = (int)(dTimeUsed/60);
    dTimeUsed -= iMinutesUsed*60;
    int iSecondsUsed = (int)(dTimeUsed);

    printf("Used time: %f (%02d:%02d:%02d)\n", dEnd - dStart, iHoursUsed, iMinutesUsed, iSecondsUsed);
    
    LOG_STATUS("-----\n");
    if (iCompletedIterations >= m_iNumIters) {
        LOG_STATUS("Finished Simulation\n");
    } else {
        LOG_STATUS("Stopped  Simulation\n");
    }
    LOG_STATUS("Number of threads: %d\n", omp_get_max_threads());
    LOG_STATUS("Number of iterations: %d\n", iCompletedIterations);
    LOG_STATUS("Used time: %f (%02d:%02d:%02d)\n", dEnd - dStart, iHoursUsed, iMinutesUsed, iSecondsUsed);
    LOG_STATUS("Number of agents after last step\n");
    const popvec &vp = m_pPopLooper->getPops();
    for (uint i = 0; i < vp.size(); i++) {
        LOG_STATUS("  %s: %d\n", vp[i]->getSpeciesName(), vp[i]->getNumAgentsEffective());
    }
    
    
    return iResult;
}


//----------------------------------------------------------------------------
// postLoop
//
int Simulator::postLoop() {
    int iResult = 0;

    for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
        iResult = m_pPopLooper->getPops()[j]->postLoop();
    }

    return iResult;
}


//----------------------------------------------------------------------------
// handleWriteEvent
//
int Simulator::handleWriteEvent(const char *pDesc, int iDumpMode) {
    int iResult = -1;
    printf("Handling writeEvent [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling writeEvent [%s] at step %d\n", pDesc, m_iCurStep);
    char sTemp[256];
    char sPops[256];
    char sOther[256];
    *sPops = '\0';
    *sOther = '\0';
    
    strcpy(sTemp, pDesc);
    char *pCtx0;
    char *p0 = strtok_r(sTemp, "+", &pCtx0);
    if (p0 != NULL) {
        char *pSub=NULL;
        uint iWhat = WR_NONE;
        while (p0 != NULL) {
            if (strncasecmp(p0, EVENT_PARAM_WRITE_POP, 3) == 0) {
                pSub = strchr(p0, ':');
                if (pSub != NULL) {
                    pSub++;
                }
                iWhat |= WR_POP;
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_GRID) == 0) {
                iWhat |= WR_GRID;
                strcat(sOther, "S");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_GEO) == 0) {
                iWhat |= WR_GEO;
                strcat(sOther, "G");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_CLIMATE) == 0) {
                iWhat |= WR_CLI;
                strcat(sOther, "C");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_VEG) == 0) {
                iWhat |= WR_VEG;
                strcat(sOther, "V");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_STATS) == 0) {
                iWhat |= WR_STAT;
                strcat(sOther, "M");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_NAV) == 0) {
                iWhat |= WR_NAV;
                strcat(sOther, "N");
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_ENV) == 0) {
                iWhat |= WR_ALL;
                strcat(sOther, "env");
            }

            p0 = strtok_r(NULL, "+", &pCtx0);
        }
        if (iWhat != WR_NONE) {
            if (pSub != NULL) {
                char *q = strchr(pSub, ':');
                while (q != NULL) {
                    *q++ = '+';
                    q = strchr(q, ':');
                }
                strcpy(sPops, "_pop-");
                    strcat(sPops, pSub);
            }
            
            char sName[8192];
            sprintf(sName, "%s%s%s%s_%s_%06d.qdf", m_sOutputDir, m_sOutputPrefix, (iDumpMode != DUMP_MODE_NONE)?"_dump":"", sPops, sOther, m_iCurStep);
            printf("[Simulator::handleWriteEvent] writing file [%s] with %sdump\n", sName, (iDumpMode != DUMP_MODE_NONE)?"":"no ");
            iResult = writeState(sName, pSub, iWhat, iDumpMode);
            if (iDumpMode != DUMP_MODE_NONE) {
                m_vDumpNames.push_back(sName);
            }
        } else {
            printf("Unknown output type [%s] (%s)\n", p0, pDesc);
            LOG_ERROR("Unknown output type [%s] (%s)\n", p0, pDesc);
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// handleDumpEvent
//
int Simulator::handleDumpEvent(const char *pDesc) {
    int iResult = 0;
    int iDumpMode = DUMP_MODE_NONE;
    printf("Handling dumpEvent [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling dumpEvent [%s] at step %d\n", pDesc, m_iCurStep);
    if (strcmp(pDesc, "flat") == 0) {
        iDumpMode = DUMP_MODE_FLAT;
    } else if (strcmp(pDesc, "smart") == 0) {
        iDumpMode = DUMP_MODE_SMART;
    } else if (strcmp(pDesc, "free") == 0) {
        iDumpMode = DUMP_MODE_FREE;
    } else {
        printf("Illegal DumpMode [%s]\n", pDesc);
        LOG_STATUS("Illegal DumpMode [%s]\n", pDesc);
        iResult = -1;
    }
    
    if (iResult == 0) {
        char sTemp[8192];
        m_vDumpNames.clear();
        // write a nice environment thingy
        iResult = handleWriteEvent(EVENT_PARAM_WRITE_ENV, iDumpMode);
        if (iResult == 0) {
            // concatenate all pop names for dump name
            strcpy(sTemp, EVENT_PARAM_WRITE_POP);
            for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
                strcat(sTemp, ":");
                strcat(sTemp, m_pPopLooper->getPops()[j]->getSpeciesName());
            }
            // writing pop dump
            printf("sending off dump writeevent [%s]\n", sTemp); 
            iResult = handleWriteEvent(sTemp, iDumpMode);
        }
        // now write param config file for resume
        if (iResult == 0) {
            char sConfigOut[8192];
            sprintf(sConfigOut, "%s%s_dump_%06d.cfg", m_sOutputDir, m_sOutputPrefix, m_iCurStep);
            printf("Writing resume config file [%s]\n", sConfigOut);
            writeResumeConfig(sConfigOut);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// handleEnvironmentEvent
//   Full format  "env"|<type>[+<type>]:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleEnvironmentEvent(const char *pDesc) {
    int iResult = -1;
    char sTemp[256];
    strcpy(sTemp, pDesc);

    printf("Handling environmentEvent [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling environmentEvent [%s] at step %d\n", pDesc, m_iCurStep);

    char *pFile =  strchr(sTemp, ':');
    if (pFile != NULL) {
        *pFile = '\0';
        pFile++;
            
        std::vector<std::string> vTypes;
        char *p = strtok(sTemp, "+");
        while (p != NULL) {
            vTypes.push_back(p);
            p = strtok(NULL, "+");
        }

        double dStart = omp_get_wtime();
        char sExistingFile[MAX_PATH];
        if (exists(pFile, sExistingFile)) {
            hid_t hFile = qdf_openFile(sExistingFile);
            iResult = 0;

            std::set<int> vEvents;
            for (uint i = 0; (iResult == 0) && (i < vTypes.size()); i++) {
                const char *pCurType = vTypes[i].c_str();
                if (strcasecmp(pCurType, EVENT_PARAM_NAME_ALL) == 0)  {
                    vEvents.insert(EVENT_ID_GEO);
                    iResult = setGeo(hFile, true, true);  // true,true: isRequired, isUpdate
                    if (iResult == 0) {
                        vEvents.insert(EVENT_ID_CLIMATE);
                        iResult = setClimate(hFile, true, true);  // true,true: isRequired, isUpdate
                        if (iResult == 0) {
                            vEvents.insert(EVENT_ID_VEG);
                            iResult = setVeg(hFile, true, true);  // true,true: isRequired, isUpdate
                            if (iResult == 0) {
                                vEvents.insert(EVENT_ID_NAV);
                                iResult = setNav(hFile, true);  // true,true: isRequired, isUpdate
                            }
                        }
                    }
                } else if (strcasecmp(pCurType, EVENT_PARAM_NAME_GEO) == 0)  {
                    vEvents.insert(EVENT_ID_GEO);
                    iResult = setGeo(hFile, true, true);  // true,true: isRequired, isUpdate
                } else if (strcasecmp(pCurType, EVENT_PARAM_NAME_CLIMATE) == 0) {
                    vEvents.insert(EVENT_ID_CLIMATE);
                    iResult = setClimate(hFile, true, true);  // true,true: isRequired, isUpdate
                } else if (strcasecmp(pCurType, EVENT_PARAM_NAME_VEG) == 0) {
                    vEvents.insert(EVENT_ID_VEG);
                    iResult = setVeg(hFile, true, true);  // true,true: isRequired, isUpdate
                } else if (strcasecmp(pCurType, EVENT_PARAM_NAME_NAV) == 0) {
                    vEvents.insert(EVENT_ID_NAV);
                    iResult = setNav(hFile, true);  // true: isUpdate
                } else {
                    // iResult = -1;
                    printf("ignoring unknown environment type: [%s]\n", pCurType);
                    LOG_ERROR("ignoring unknown environment type [%s]\n", pCurType);                
                }               
            }
            double dEnd = omp_get_wtime();
            qdf_closeFile(hFile);
            printf("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
            LOG_WARNING("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
            if (vEvents.size() > 0) {
                for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
                    for (intset::iterator it = vEvents.begin(); it != vEvents.end(); ++it) {
                        iResult = m_pPopLooper->getPops()[j]->updateEvent(*it, NULL, m_iCurStep);
                    }
                }
            }
            
        } else {
            printf("couldn't find file [%s]\n", pFile);
            LOG_ERROR("couldn't find file [%s]\n", pFile);
        }


    } else {
        printf("expected ':' in env event [%s]\n", pDesc);
        LOG_ERROR("expected ':' in env event [%s]\n", pDesc);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// handleEnvArrayEvent
//   Full format  "arr"|<type>:<arrname>:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleEnvArrayEvent(const char *pDesc) {
    int iResult = -1;
    char sTemp[8192];
    strcpy(sTemp, pDesc);

    printf("Handling envArrayEvent [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling envArrayEvent [%s] at step %d\n", pDesc, m_iCurStep);

    char *pGroup =  strtok(sTemp, ":");
    if (pGroup != NULL) {
        char *pArrName =  strtok(NULL, ":");
        if (pArrName != NULL) {
            char *pFile =  strtok(NULL, ":");
            if (pFile != NULL) {


                double dStart = omp_get_wtime();
                char sExistingFile[MAX_PATH];
                if (exists(pFile, sExistingFile)) {
                    hid_t hFile = qdf_openFile(sExistingFile);
                    iResult = 0;
                    int iEvent = 0;
                    if (strcasecmp(pGroup, EVENT_PARAM_NAME_GEO) == 0)  {
                        iEvent = EVENT_ID_GEO;
                        iResult = setGeoArray(hFile, pArrName);
                    } else if (strcasecmp(pGroup, EVENT_PARAM_NAME_CLIMATE) == 0) {
                        iEvent = EVENT_ID_CLIMATE;
                        iResult = setClimateArray(hFile, pArrName);
                    } else if (strcasecmp(pGroup, EVENT_PARAM_NAME_VEG) == 0) {
                        iEvent = EVENT_ID_VEG;
                        iResult = setVegArray(hFile, pArrName);
                    } else if (strcasecmp(pGroup, EVENT_PARAM_NAME_NAV) == 0) {
                        printf("no array setting  for [%s]\n", pGroup);
                        LOG_ERROR("no array setting  for [%s]\n", pGroup);
                    } else {
                        // iResult = -1;
                        printf("ignoring unknown environment type: [%s]\n", pGroup);
                        LOG_ERROR("ignoring unknown environment type [%s]\n", pGroup);                
                    }                                   
                    
                    double dEnd = omp_get_wtime();
                    qdf_closeFile(hFile);
                    printf("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
                    LOG_WARNING("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
                    if (iResult == 0) {
                        for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
                            iResult = m_pPopLooper->getPops()[j]->updateEvent(iEvent, NULL, m_iCurStep);
                        }
                    } else {

                        printf("Couldn't open array [%s]  for group [%s] in file [%s]\n", pArrName, pGroup, sExistingFile);
                    }
                } else {
                    printf("couldn't find file [%s]\n", pFile);
                    LOG_ERROR("couldn't find file [%s]\n", pFile);
                }
            } else {
                printf("no file name in eventstring [%s]? Expected \"arr\"|<group>:<arrname>:<qdf-file>\n", pDesc);
                LOG_ERROR("no file name in eventstring [%s]? Expected \"arr\"|<group>:<arrname>:<qdf-file>\n", pDesc);
            }
        } else {
            printf("no array name in eventstring [%s]? Expected \"arr\"|<group>:<arrname>:<qdf-file>\n", pDesc);
            LOG_ERROR("no array name in eventstring [%s]? Expected \"arr\"|<group>:<arrname>:<qdf-file>\n", pDesc);
        }
    } else {
        printf("expected ':' in env event [%s]\n", pDesc);
        LOG_ERROR("expected ':' in env event [%s]\n", pDesc);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// handlePopEvent
//   Full format  "pop"|<speciesname>[+<speciesname>]:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handlePopEvent(const char *pDesc) {
    char sTemp[256];
    int iResult = -1;

    strcpy(sTemp, pDesc);

    printf("Handling populationEvenr [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling populationEvent [%s] at step %d\n", pDesc, m_iCurStep);

    char *pFile =  strchr(sTemp, ':');
    if (pFile != NULL) {
        *pFile = '\0';
        pFile++;
            
        std::vector<std::string> vSpecies;
        char *p = strtok(sTemp, "+");
        while (p != NULL) {
            vSpecies.push_back(p);
            p = strtok(NULL, "+");
        }

        double dStart = omp_get_wtime();
        char sExistingFile[MAX_PATH];
        if (exists(pFile, sExistingFile)) {
            hid_t hFile = qdf_openFile(sExistingFile);
            iResult = 0;

            for (uint i = 0; (iResult == 0) && (i < vSpecies.size()); i++) {

                iResult  = setPops(hFile, vSpecies[i].c_str(),true);
            }
                
            double dEnd = omp_get_wtime();
            qdf_closeFile(hFile);
            printf("[%d]population load took %f s\n", m_iCurStep, dEnd -dStart);
            LOG_WARNING("[%d]population load took %f s\n", m_iCurStep, dEnd -dStart);
            if (iResult == 0) {
                for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
                    iResult = m_pPopLooper->getPops()[j]->updateEvent(EVENT_ID_POP, NULL, m_iCurStep);
                }
            }
        } else {
            printf("couldn't find file [%s]\n", pFile);
            LOG_ERROR("couldn't find file [%s]\n", pFile);
        }


    } else {
        printf("expected ':' in pop event [%s]\n", pDesc);
        LOG_ERROR("expected ':' in pop event [%s]\n", pDesc);
    }


    return iResult;
}



//----------------------------------------------------------------------------
// handleUserEvent
//   Full format  "user"|<eventid>:<data>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleUserEvent(const char *pDesc) {
    char sTemp[256];
    int iResult = -1;

    strcpy(sTemp, pDesc);

    printf("Handling userEvent [%s] at step %d\n", pDesc, m_iCurStep);
    LOG_STATUS("Handling userEvent [%s] at step %d\n", pDesc, m_iCurStep);
    char *pData =  strchr(sTemp, ':');
    if (pData != NULL) {
        *pData = '\0';
        pData++;
        
        int iUserEvent = -1;
        if (strToNum(sTemp, &iUserEvent) && ((iUserEvent >= EVENT_ID_USR_MIN) && (iUserEvent <= EVENT_ID_USR_MAX))) {


        } else {
            printf("the first part should be a user event id in [%d, %d]\n", EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
            LOG_ERROR("the first part should be a user event id in [%d, %d]\n", EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
        }
     
        
        
    } else {
        printf("expected ':' in user event [%s]\n", pDesc);
        LOG_ERROR("expected ':' in user event [%s]\n", pDesc);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setGeoArray
//     an array event
//
int Simulator::setGeoArray(hid_t hFile, const char *pArrName) {
    int iResult = -1;
    // try for array event
    GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
    if (pGR != NULL) {
        GeoAttributes geoatt;
        iResult = pGR->readAttributes(&geoatt);
        if (iResult == 0) {                
            if (m_pGeo != NULL) {
                m_pGeo = m_pCG->m_pGeography;
                iResult = pGR->readArray(m_pGeo, pArrName);
                if (iResult == 0) {
                    printf("Successfully read array [%s] form file\n", pArrName);
                    LOG_STATUS("Successfully read array [%s] form file\n", pArrName);
                } else {
                    printf("unknown geo array [%s]\n", pArrName);
                        LOG_ERROR("unknown geo arrayevent [%s]\n", pArrName);
                }
            } else {
                printf("A Geography object must exist to add array\n");
                LOG_ERROR("A Geography object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            printf("Couldn't read attributes from file\n");
            LOG_ERROR("Couldn't read attributes from file\n");
        }
        delete pGR;
    } else {
        printf("Couldn't open GeoGroupReader for file\n");
        LOG_ERROR("Couldn't open GeoGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setClimateArray
//     an array event
//
int Simulator::setClimateArray(hid_t hFile, const char *pArrName) {
    int iResult = -1;
    // try for array event
    ClimateGroupReader *pCR = ClimateGroupReader::createClimateGroupReader(hFile);
    if (pCR != NULL) {
        ClimateAttributes climatt;
        iResult = pCR->readAttributes(&climatt);
        if (iResult == 0) {                
            if (m_pCli != NULL) {
                iResult = pCR->readArray(m_pCli, pArrName);
                if (iResult == 0) {
                    printf("Successfully read array [%s] from file]\n", pArrName);
                    LOG_STATUS("Successfully read array [%s] form [%s]\n", pArrName);
                } else {
                    printf("unknown  array [%s]\n", pArrName);
                    LOG_ERROR("unknown geo arrayevent [%s]\n", pArrName);
                }
            } else {
                printf("A Climate object must exist to add array\n");
                LOG_ERROR("A Climate object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            printf("Couldn't read attributes from file\n");
            LOG_ERROR("Couldn't read attributes from file\n");
        }
        delete pCR;
    } else {
        printf("Couldn't open ClimateGroupReader for file\n");
        LOG_ERROR("Couldn't open ClimateGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setVegArray
//     an array event
//
int Simulator::setVegArray(hid_t hFile, const char *pArrName) {
    int iResult = -1;
    // try for array event
    VegGroupReader *pVR = VegGroupReader::createVegGroupReader(hFile);
    if (pVR != NULL) {
        VegAttributes vegatt;
        iResult = pVR->readAttributes(&vegatt);
        if (iResult == 0) {                
            if (m_pVeg != NULL) {
                iResult = pVR->readArray(m_pVeg, pArrName);
                
                if (iResult == 0) {
                    printf("Successfully read array [%s] from file]\n", pArrName);
                    LOG_STATUS("Successfully read array [%s] form [%s]\n", pArrName);
                } else {
                    printf("unknown  array [%s]\n", pArrName);
                    LOG_ERROR("unknown geo arrayevent [%s]\n", pArrName);
                }
            } else {
                printf("A Vegetation object must exist to add array\n");
                LOG_ERROR("A Vegetation object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            printf("Couldn't read attributes from file\n");
            LOG_ERROR("Couldn't read attributes from file\n");
        }
        delete pVR;
    } else {
        printf("Couldn't open VegGroupReader for file\n");
        LOG_ERROR("Couldn't open VegGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleCheckEvent
// Currently supported:
//   LISTS
//
int Simulator::handleCheckEvent(const char *pChecks) {
    int iResult = 0;

    char sTemp[256];
    strcpy(sTemp, pChecks);
    
    std::vector<std::string> vChecks;
    char *p = strtok(sTemp, "+");
    while (p != NULL) {
        vChecks.push_back(p);
        p = strtok(NULL, "+");
    }

    std::vector<std::string>::const_iterator it;
    for (it = vChecks.begin(); it != vChecks.end(); ++it) {
        if (strcasecmp(EVENT_PARAM_CHECK_LISTS, it->c_str()) == 0) {
            for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
                iResult = m_pPopLooper->getPops()[j]->checkLists();
            }
        } else {
            iResult = -1;
            printf("unknown check type: [%s]\n", it->c_str());
            LOG_ERROR("unknown check type: [%s]\n", it->c_str());
        }
    }

 
    return iResult;
}


//----------------------------------------------------------------------------
// handleCommEvent
// Currently supported:
//   SET ITERS <numiters>
//   REMOVE ACTION <population>:<actionname>
//
int Simulator::handleCommEvent(const char *pCommFile) {
    int iResult = 0;

    struct stat buf;
    iResult = stat(pCommFile, &buf);
    if (iResult == 0) {
        int iNewTime = buf.st_mtim.tv_sec;
        if (iNewTime > m_iLastCommTime) {
            m_iLastCommTime = iNewTime;
            LineReader *pLR = LineReader_std::createInstance(pCommFile, "rt");
            if (pLR != NULL) {
                char *pLine = pLR->getNextLine();
                while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
                    iResult = handleCommLine(pLine);
                    iResult = 0;
                    pLine = pLR->getNextLine();
                }
                delete pLR;
            } else {
                printf("Couldn'get open [%s]\n", pCommFile);
            }
            //
            std::vector<std::string> vs;
            m_pEM->toString(vs);
            for (uint i = 0; i < vs.size(); ++i) {
                std::string &sCur = vs[i];
                printf("%s\n", addEventName(sCur).c_str());
            }

        }
    } else {
        iResult = 0;
        printf("Couldn'get stat for [%s]\n", pCommFile);
        printf("Trying [%s] as command\n", pCommFile);
        handleCommLine(pCommFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleCommLine
// Currently supported:
//   SET ITERS <numiters>
//   REMOVE ACTION <population>:<actionname>
//
int Simulator::handleCommLine(const char *pLine) {
    int iResult = 0;

    if (strstr(pLine, CMD_SET_ITERS) == pLine) {
        int iNum = 0;
        pLine += strlen(CMD_SET_ITERS);
        int iRead = sscanf(pLine, "%d", &iNum);
        if (iRead == 1) {
            if (iNum < m_iCurStep) {
                m_iNumIters = m_iCurStep+1;
            } else {
                m_iNumIters = iNum;
            }
            m_pEM->triggerAll(m_iNumIters);
        } else {
            printf("Bad argument to command '%s': [%s]\n", CMD_SET_ITERS, pLine);
            LOG_ERROR("Bad argument to command '%s': [%s]\n", CMD_SET_ITERS, pLine);
        }
    } else if  (strstr(pLine, CMD_REMOVE_ACTION) == pLine) {
        
        char sPopulation[256];
        strncpy(sPopulation, pLine + strlen(CMD_REMOVE_ACTION), 255);
        char *pPopulation = trim(sPopulation);
        char *pAction = strchr(pPopulation, ':');
        if (pAction != NULL) {
            *pAction = '\0';
            pAction++;
            // now delete action
            PopBase *pPop = m_pPopLooper->getPopByName(pPopulation);
            if (pPop != NULL) {
                iResult = pPop->removeAction(pAction);
                if (iResult == 0) {
                    printf("Successfully removed action [%s]\n", pAction);
                }
                iResult = 0;
            } else {
                printf("Couldn't find population [%s]\n", pPopulation);
                LOG_ERROR("Couldn't find population [%s]\n", pPopulation);
            }
        } else {
            printf("BadArgument to command '%s': [%s]\n", CMD_REMOVE_ACTION, sPopulation);
            LOG_ERROR("BadArgument to command '%s': [%s]\n", CMD_REMOVE_ACTION, sPopulation);
        }
    } else {
        char sLine[256];
        strncpy(sLine, pLine, 255);
  
        // expect event description (as in parameter "--events=")
        iResult = addEventTriggers(sLine, m_iCurStep);
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// processEvent
//   EVENT_ID_WRITE   (output)
//   EVENT_ID_ENV     (geography,climate,veg,nav change)
//   EVENT_ID_POP     (add populations)
// 
//   EVENT_ID_GEO     (geography change)
//   EVENT_ID_CLIMATE (climate change)
//   EVENT_ID_VEG     (vegetation change)
//   EVENT_ID_NAV     (navigation change)
//
//   EVENT_ID_DUMP    (dump all data)
//
//   EVENT_ID_USER    (user-defined event+data)
//
int Simulator::processEvent(EventData *pData) {
    int iResult = 0;

    // look at the event type
    switch (pData->m_iEventType) {
    case EVENT_ID_WRITE:
        handleWriteEvent(pData->m_sData.c_str(), DUMP_MODE_NONE);
        break;
    case EVENT_ID_ENV:
        handleEnvironmentEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_ARR:
        handleEnvArrayEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_POP:
        handlePopEvent(pData->m_sData.c_str());
        break;
        
        // to extract specific arrays
        /*
    case EVENT_ID_GEO:
        handleGeographyEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_CLIMATE:
        handleClimateEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_VEG:
        handleVegetationEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_NAV:
        handleNavigationEvent(pData->m_sData.c_str());
        break;
        */
        // more debuggy stuff
    case EVENT_ID_COMM:
        handleCommEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_CHECK:
        handleCheckEvent(pData->m_sData.c_str());
        break;
        
    case EVENT_ID_DUMP:
        handleDumpEvent(pData->m_sData.c_str());
        break;
        
    case EVENT_ID_USER:
        handleUserEvent(pData->m_sData.c_str());
        break;
        
    default:
        printf("Unknown event type [%d]\n", pData->m_iEventType);
        LOG_WARNING("Unknown event type [%d]\n", pData->m_iEventType);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// writeState
//  write output
//
 int Simulator::writeState(const char *pQDFOut, char *pSub, int iWhat, int iDumpMode) {
    int iResult = 0;

    double dStartW = omp_get_wtime();

    iResult = m_pSW->write(pQDFOut, (float)m_iCurStep, pSub, iWhat, iDumpMode);

    double dEndW = omp_get_wtime();
    printf("[%d] Writing of [%s] took %fs\n", m_iCurStep, pQDFOut, dEndW - dStartW);
    LOG_WARNING("[%d] Writing of [%s] took %fs\n", m_iCurStep, pQDFOut, dEndW - dStartW);
    
    if (iResult != 0) {
        printf("StatusWriter [%s]\n", m_pSW->getError().c_str());
        if (iResult < 0) {
            LOG_ERROR("StatusWriter [%s]\n", m_pSW->getError().c_str());
        } else {
            LOG_WARNING("StatusWriter [%s]\n", m_pSW->getError().c_str());
        }
    }

    if (iResult >= 0) {
        if (m_bZipOutput) {
            double dStartZ = omp_get_wtime();

            char *pCommand = new char[strlen(pQDFOut) + 9];
            sprintf(pCommand, "gzip -f %s", pQDFOut);
            iResult = system(pCommand);
            delete[] pCommand;

            double dEndZ = omp_get_wtime();
            printf("[%d] zipping [%s] took %fs\n", m_iCurStep, pQDFOut, dEndZ - dStartZ);
            LOG_WARNING("[%d] zipping [%s] took %fs\n", m_iCurStep, pQDFOut, dEndZ - dStartZ);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// showInputs
//  write output
//
void Simulator::showInputs() {
    printf("--------Inputs--------\n"); 
    if (m_pCG != NULL) {
        printf("Grid %d cells\n", m_pCG->m_iNumCells);
    }
    if (m_pGeo != NULL) {
        printf("Geography\n");
    }
    if (m_pCli != NULL) {
        printf("Climate\n");
    }
    if (m_pVeg != NULL) {
        printf("Vegetation\n");
    }
    if (m_pPopLooper != NULL) {
        printf("Populations: %zd\n   ", m_pPopLooper->getNumPops());
        for (uint i =0; i < m_pPopLooper->getNumPops(); i++) {
            printf("%s  ", m_pPopLooper->getPops().at(i)->getSpeciesName());
        }
        printf("\n");
    }
    if (m_bHasMoveStats) {
        printf("MoveStats\n");
    }
    printf("----------------------\n"); 
}

//----------------------------------------------------------------------------
// rangeEliminate
//   modify the time range such that iStep is not contained anymore
//   <iStep>        remove
//   [<iStep>]      remove
//   [<iStep>:<max>]  -> [<iStep+1>:<max>]
//   [<min>:<iStep>]  -> [<min>:<iStep-1>]
//
bool rangeEliminate(std::string &sRange, int iStep) {
    bool bEliminated = false;
    bool bBrackets = false;
    size_t iPos = sRange.find("[");
    if (iPos != std::string::npos) {
        bBrackets=true;
        sRange[iPos] = ' ';
    }
    iPos = sRange.find("]");
    if (iPos != std::string::npos) {
        bBrackets=true;
        sRange[iPos] = ' ';
    }

    iPos = sRange.find(":");
    if (iPos == std::string::npos) {
        int r = atol(sRange.c_str());
        if (r == iStep) {
            bEliminated = true;
            sRange = "";
        } else {
            sRange = trim(sRange);
            if (bBrackets) {
                sRange = "["+sRange+"]";
            }
        }
    } else {
        std::string s1 = trim(sRange.substr(0, iPos));
        std::string s2 = trim(sRange.substr(iPos+1, std::string::npos));
        int r1 = atol(s1.c_str());
        int r2 = atol(s2.c_str());
        char scSub[256];
        sprintf(scSub, "%d", iStep-1);
        std::string sSub = scSub;
        char scSup[256];
        sprintf(scSup, "%d", iStep+1);
        std::string sSup = scSup;
        if ((r1 < iStep) && (iStep < r2) ) {
            sRange = "["+s1+":"+sSub+"]+["+sSup+":"+s2+"]";
        } else if (r1 == iStep) {
            sRange = "["+sSup+":"+s2+"]";
        } else if (r2 == iStep) {
            sRange = "["+s1+":"+sSub+"]";
        } else {
            bEliminated = false;
        }
    }
    return bEliminated;
}


//----------------------------------------------------------------------------
// writeResumeConfig
//  write output
//
void Simulator::writeResumeConfig(const char *pConfigOut) {
    
    // retrieve the dump file names
    std::string sPop  = "";
    std::string sGrid = "";
    
    for (uint i = 0; i < m_vDumpNames.size(); i++) {
        if (m_vDumpNames[i].find("pop") != std::string::npos) {
            if (sPop.size() > 0) {
                sPop = sPop + ",";
            }
            sPop  = sPop + m_vDumpNames[i];
        } else if (m_vDumpNames[i].find("env") != std::string::npos) {
            sGrid = m_vDumpNames[i];
        }
    }
    std::vector<std::string> vsOptions;
    m_pPR->collectOptions(vsOptions);
    for (unsigned int j = 0; j < vsOptions.size(); j++) {
        std::string sCur = trim(vsOptions[j]);
        if (sCur.find("--grid=") == 0) {
            vsOptions[j] =  "--grid="+sGrid;
        } else if (sCur.find("--pops=") == 0) {
            vsOptions[j] = "--pops="+sPop;
        } else if (sCur.find("--log-file=") == 0) {
            size_t iPos = sCur.find("=");
            std::string sLog = sCur.substr(iPos+1, std::string::npos);
            if (!strReplace(sLog, "_o.log", "_r.log")) {
                if (!strReplace(sLog, ".log", "_r.log")) {
                    sLog = sLog+"_r.log";
                }
            }
            vsOptions[j] = "--log-file="+sLog;
        } else if (sCur.find("--data-dirs=") == 0) {
            size_t iPos = sCur.find("=");
            std::string sDDirs = sCur.substr(iPos+1, std::string::npos);
            int i0 = 0;
            int l = sDDirs.size()-1;
            if (sDDirs[l] == '\'') {
                l--;
            }
            if (sDDirs[i0] == '\'') {
                i0++;
                l--;
            }
            vsOptions[j] = "--data-dirs="+sDDirs.substr(i0, l);

        } else if (sCur.find("--events=") == 0) {
            std::string sOut = "--events=";
            std::vector<std::string> vSingleOptions;
            size_t iPos =  vsOptions[j].find("=");
            iPos++;
            splitString(vsOptions[j].substr(iPos, std::string::npos), vSingleOptions, ','); 
            bool bFollower = false;
            for (unsigned int i = 0; i < vSingleOptions.size(); i++) {
                std::string sNew = "";
                if (vSingleOptions[i].find("dump") == std::string::npos) {
                    sNew = vSingleOptions[i];
                } else {
                    std::vector<std::string> vSingleEvents;
                    size_t iPos2 = vSingleOptions[i].find("@");
                    if (iPos2 != std::string::npos) {
                        iPos2++;
                        std::string sFirst = vSingleOptions[i].substr(0, iPos2);
                        std::string sTimes = "";
                        
                        splitString(vSingleOptions[i].substr(iPos2, std::string::npos), vSingleEvents, '+'); 
                        bool bFollower2 = false;
                        for (unsigned int k = 0; k < vSingleEvents.size(); k++) {
                            if (!rangeEliminate(vSingleEvents[k], m_iCurStep)) {
                                if (bFollower2) {
                                    sTimes = sTimes+"+";
                                } else {
                                    bFollower2 = true;
                                }
                                sTimes = sTimes + vSingleEvents[k];
                            } else {
                                //printf("%s is removed\n", vSingleEvents[k].c_str());
                            }
                        }
                        if (sTimes != "") {
                            sNew = sFirst+sTimes;
                        }    
                    }
                }
                if (sNew != "") {
                    if (bFollower) {
                        sOut = sOut + ",";
                    } else {
                        bFollower = true;
                    }
                    sOut = sOut + sNew;
                }
            }
           
            vsOptions[j] = sOut;
        } else {
            vsOptions[j] = trim( vsOptions[j]);
        }
        
    }
    vsOptions.push_back("--resume");
    FILE *fOut = fopen(pConfigOut, "wt");
    for (uint i = 0; i< vsOptions.size(); ++i) {
        fprintf(fOut, "%s\n", vsOptions[i].c_str());
    }
    fclose(fOut);
}


