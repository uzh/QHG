#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <omp.h>

#include <hdf5.h>

#include "MessLogger.h"
#include "LineReader.h"
#include "ids.h"
#include "strutils.h"
#include "QMapUtils.h"
#include "ValReader.h"
#include "QDFUtils.h"
#include "StatusWriter.h"

#include "SCellGrid.h"

#include "PopBase.h"
#include "SPopulation.h"
#include "PopLooper.h"
#include "EventManager.h"


#include "SimParams.h"
#include "Simulator.h"

#define GENOMESIZE 20


//----------------------------------------------------------------------------
// constructor
//
Simulator::Simulator()
    : SimParams(),
      m_iLastCommTime(-1) {

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

#ifdef OMP_A
#pragma omp parallel 
        {
#endif
            int iT = omp_get_thread_num();
            m_apIDG[iT] = new IDGen(iMaxID+1, iOffsBase+iT, iNumThreads);
#ifdef OMP_A
        }      
#endif
        
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
        m_pSW = StatusWriter::createInstance(m_pCG, m_pPopLooper->getPops());

        // call the preloop methods of the pops
        iResult = 0;
        for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
            iResult = m_pPopLooper->getPops()[j]->preLoop();
        }
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
// runLoop
//
int Simulator::runLoop() {
    int iResult = 0;
    int iInterval = 20;

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
    printf("------ %d snap interval (hard-wired) \n", iInterval);
    printf("---------------------------\n");

    m_iCurStep = 0;
    checkEvents((float)m_iCurStep); // later maybe m_fCurTime

    LOG_STATUS("Number of agents before starting\n");
    const popvec &vp0 = m_pPopLooper->getPops();
    for (uint i = 0; i < vp0.size(); i++) {
        LOG_STATUS("  %s: %d\n", vp0[i]->getSpeciesName(), vp0[i]->getNumAgentsTotal());
    }
    LOG_STATUS("Starting Simulation ...\n");
    LOG_STATUS("-----\n");
    double dStart = omp_get_wtime();

    for (m_iCurStep = 0; m_iCurStep < m_iNumIters; ) {
        if (m_pCG->m_pVegetation != NULL) {
            m_pCG->m_pVegetation->update((float)m_iCurStep);
        }
        /*
        for (uint j = 0; j < m_pPopLooper->getNumPops(); j++) {
            m_pPopLooper->getPops()[j]->showAgents();
        }
        */
        fprintf(stderr, "Step %03d ---------------------------\n", m_iCurStep);
        m_pPopLooper->doStep((float)m_iCurStep);

        m_iCurStep++;

        // count total number of agents
        ulong iTotalTotal = 0;
        if (m_pPopLooper->getNumPops() > 0) {
            for (uint j = 0; j < m_pPopLooper->getNumPops(); j++) {
                iTotalTotal +=  m_pPopLooper->getPops()[j]->getNumAgentsTotal();
            }
            printf("After step %d: total %lu agents\n", m_iCurStep, iTotalTotal);
        }

		if (m_pCG->m_pVegetation != NULL) {
	        m_pCG->m_pVegetation->resetUpdated();
		} 
		if (m_pCG->m_pClimate != NULL) {
	        m_pCG->m_pClimate->resetUpdated();
		}
		if (m_pCG->m_pGeography != NULL) {
			m_pCG->m_pGeography->resetUpdated();
		}

        // check the events
        checkEvents((float)m_iCurStep);

        if (iTotalTotal == 0) {
            // avoid crash by stopping simulation if no agents survive
            m_iCurStep = m_iNumIters;
        }
        
	/*
        if ((m_iCurStep%iInterval) == 0) {
            int iNumCur = m_pPopLooper->getPops()[0]->getNumAgentsTotal();
            double dCur = omp_get_wtime();
            int iB = m_pPopLooper->getPops()[0]->getNumEvents(EVMSK_BIRTHS);
            int iM = m_pPopLooper->getPops()[0]->getNumEvents(EVMSK_MOVES);
            int iD = m_pPopLooper->getPops()[0]->getNumEvents(EVMSK_DEATHS);
            printf("At step %d POp[0] has %d agents,  used time: %f #B %d #M %d #D %d #T %d\n", 
                   m_iCurStep, iNumCur, dCur - dStart, iB, iM,iD, iB+iM+iD);
 
        }
	*/
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
    LOG_STATUS("Finished Simulation\n");
    LOG_STATUS("Number of threads: %d\n", omp_get_max_threads());
    LOG_STATUS("Number of iterations: %d\n", m_iNumIters);
    LOG_STATUS("Used time: %f (%02d:%02d:%02d)\n", dEnd - dStart, iHoursUsed, iMinutesUsed, iSecondsUsed);
    LOG_STATUS("Number of agents after last step\n");
    const popvec &vp = m_pPopLooper->getPops();
    for (uint i = 0; i < vp.size(); i++) {
        LOG_STATUS("  %s: %d\n", vp[i]->getSpeciesName(), vp[i]->getNumAgentsTotal());
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
int Simulator::handleWriteEvent(const char *pDesc) {
    int iResult = -1;
    printf("Handling writeEvent [%s]\n", pDesc);
    char sTemp[256];
    char sPops[256];
    char sOther[256];
    *sPops = '\0';
    *sOther = '\0';
    
    strcpy(sTemp, pDesc);
    char *pCtx0;
    char *p0 = strtok_r(sTemp, "|", &pCtx0);
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
            } else if (strcasecmp(p0, EVENT_PARAM_WRITE_ALL) == 0) {
                iWhat |= WR_ALL;
            }

            p0 = strtok_r(NULL, "|", &pCtx0);
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
            
            char sName[256];
            sprintf(sName, "%s%s%s_%s_%06d.qdf", m_sOutputDir, m_sOutputPrefix, sPops, sOther, m_iCurStep);
       
            iResult = writeState(sName, pSub, iWhat);
        } else {
            printf("Unknown output [%s]\n", p0);
            LOG_ERROR("Unknown output [%s]\n", p0);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleGeographyEvent
//   currently, this can be a sea-level event (double value), 
//   or a altitude event (QMap)
//
int Simulator::handleGeographyEvent(const char *pDesc) {
    int iResult = -1;
    char sTemp[256];
    strcpy(sTemp, pDesc);

    char *p1 = strchr(sTemp, ':');
    if (p1 != NULL) {
        *p1 = '\0';
        p1++;
        
        if (strcasecmp(sTemp, EVENT_PARAM_GEO_ALT) == 0) {
            ValReader *pVRAlt  = QMapUtils::createValReader(p1, true);  // prepare to read
            if (pVRAlt != NULL) {
                for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                    double dLon = m_pCG->m_pGeography->m_adLongitude[i];
                    double dLat = m_pCG->m_pGeography->m_adLatitude[i];
                    double dV = pVRAlt->getDValue(dLon, dLat);
                    m_pCG->m_pGeography->m_adAltitude[i] = dV;
                }
                iResult = 0;
                delete pVRAlt;
            } else {
                printf("couldn't open QMap for reading [%s]\n", p1);
                LOG_ERROR("couldn't open QMap for reading [%s]\n", p1);
            }
        } else if (strcasecmp(sTemp, EVENT_PARAM_GEO_SEA) == 0) {
            double dSea= 0;
            if (strToNum(p1, &dSea)) {
                m_pCG->m_pGeography->m_dSeaLevel = dSea;
                // maybe this needs some other changes?
                iResult = 0;
            } else {
                printf("invalid sea level value [%s]\n", p1);
                LOG_ERROR("invalid sea level value [%s]\n", p1);
            }
        } else if (strcasecmp(sTemp, EVENT_PARAM_GEO_QDF) == 0) { 
            char sExistingFile[MAX_PATH];
            if (exists(p1, sExistingFile)) {
                hid_t hFile = qdf_openFile(sExistingFile);
                iResult = setGrid(hFile, true);
            }
            if (iResult != 0) {
                printf("failed to update grid with file [%s]\n", p1);
                LOG_ERROR("failed to update grid with file [%s]\n", p1);                
            }

        }  else {
            printf("unknown geo event [%s]\n", pDesc);
            LOG_ERROR("unknown geo event [%s]\n", pDesc);
        }

    } else {
        printf("expected ':' in geo event [%s]\n", pDesc);
        LOG_ERROR("expected ':' in geo event [%s]\n", pDesc);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleClimateEvent
//
int Simulator::handleClimateEvent(const char *pDesc) {
    int iResult = -1;
        printf("ClimateEvent not implemented yet\n");
        LOG_ERROR("ClimateEvent not implemented yet\n");

    return iResult;
}



//----------------------------------------------------------------------------
// handleCommEvent
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
                    if (strstr(pLine, "SET ITERS") == pLine) {
                        int iNum = 0;
                        int iRead = sscanf(pLine, "SET ITERS %d", &iNum);
                        if (iRead == 1) {
                            if (iNum < m_iCurStep) {
                                m_iNumIters = m_iCurStep+1;
                            } else {
                                m_iNumIters = iNum;
                            }
                            m_pEM->triggerAll(m_iNumIters);
                        } else {
                        }
                    } else {
                        // expect event description (as in parameter "--events=")
                        iResult = addEventTriggers(pLine, m_iCurStep);
                        iResult = 0;
                    }
                    pLine = pLR->getNextLine();
                }
                delete pLR;
            } else {
                printf("Couldn'get open [%s]\n", pCommFile);
            }
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
    }
    return iResult;
}


//----------------------------------------------------------------------------
// processEvent
//   EVENT_TYPE_WRITE   (output)
//   EVENT_TYPE_GEO     (geography change)
//   EVENT_TYPE_CLIMATE (climate change)
//
int Simulator::processEvent(EventData *pData) {
    int iResult = 0;

    // look at the event type
    switch (pData->m_iEventType) {
    case EVENT_ID_WRITE:
        handleWriteEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_CLIMATE:
        handleClimateEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_GEO:
        handleGeographyEvent(pData->m_sData.c_str());
        break;
    case EVENT_ID_COMM:
        handleCommEvent(pData->m_sData.c_str());
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
int Simulator::writeState(const char *pQDFOut, char *pSub, int iWhat) {
    int iResult = 0;

    iResult = m_pSW->write(pQDFOut, (float)m_iCurStep, pSub, iWhat);
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
