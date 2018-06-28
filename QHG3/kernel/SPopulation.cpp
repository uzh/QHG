#ifndef __SPOPULATION_CPP__
#define __SPOPULATION_CPP__

#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#include <map>
#include <vector>
#include <algorithm>

#include "types.h"
#include "ids.h"
#include "strutils.h"
#include "geomutils.h"
#include "LineReader.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "PopFinder.h"
#include "IDGen.h"
#include "stacktrace.h"

#include "QDFUtils.h"
#include "LayerBuf.h"
#include "LBController.h"

#include "SPopulation.h"

static const char *KEY_CLASS      = "CLASS";
static const char *KEY_SPECIES    = "SPECIES";
static const char *KEY_SENSING    = "SENSING";
static const char *KEY_PARAMS     = "BEGIN_PARAMS";
static const char *KEY_ENDPARAMS  = "END_PARAMS";
static const char *KEY_PRIO       = "PRIO";

static const int BQD_SIZE = 3; // birth queue data size (cellindex, motherindex, fatherindex)

static double fPerfBirthTime = 0;


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
SPopulation<T>::SPopulation(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, 
                            IDGen **apIDGen, uint32_t *aulState, uint *piSeeds) 
      : m_iNumCells(pCG->m_iNumCells), 
        m_iTotal(0),
        m_iNumBirths(0),
        m_iNumDeaths(0),
        m_iNumMoves(0),
        m_dNumAgentsPerCell(NULL),
        m_fCurTime(-1),
        m_pCG(pCG),
        m_pPopFinder(pPopFinder),
        m_pAgentController(NULL),
        m_pWriteCopyController(NULL),
        m_iMaxID(0),
        m_apIDGen(apIDGen),
        m_iMaxReuse(1024),
        m_iMaxNormalB(1024),
        m_iMaxNormalD(1024),
        m_iNumPrevDeaths(0),
        m_bRecycleDeadSpace(true),
        m_dDistScale(1.0),
        m_fCalcDist(NULL) {

    m_iNumThreads = omp_get_max_threads();
    printf("Have %d threads\n", m_iNumThreads);
    m_iSpeciesID = SPC_NONE;
    *m_sSpeciesName = '\0';

//    m_dRecycleDeadTime = 0;
    
    memcpy(m_aulInitialState, aulState, STATE_SIZE*sizeof(uint32_t));
    memcpy(m_aiSeeds, piSeeds, NUM_SEEDS*sizeof(int));

    prepareLists(iLayerSize, iLayerSize, aulState);    

    // these are needed for movestats
    if (m_pCG->m_pGeography != NULL) {
        if (pCG->isCartesian()) {
            m_fCalcDist = &cartdist;
            m_dDistScale = 1.0;
        } else { 
            m_fCalcDist = &spherdistDeg;
            m_dDistScale = m_pCG->m_pGeography->m_dRadius;
        }
    }

}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
SPopulation<T>::~SPopulation() {

//    printf("TIME TAKEN TO RECYCLE DEAD SPACE: %f\n",m_dRecycleDeadTime);

    delete[] m_dNumAgentsPerCell;
    delete m_pAgentController;
    delete m_pWriteCopyController;

    for (int iT = 0; iT < m_iNumThreads; iT++) {
        delete m_apWELL[iT];
        delete m_vMoveList[iT];
        delete m_vBirthList[iT];
        delete m_vDeathList[iT];
    }
    delete[] m_apWELL;

    delete[] m_vBirthList;
    delete[] m_vDeathList;
    delete[] m_vMoveList;

    delete[] m_pReuseB;
    delete[] m_pNormalB;
    delete[] m_pPrevD;

    printf("*** time spent in performBirths(ulong iNumBirths, int *piBirthData): %f\n", fPerfBirthTime);
    printf("*** total number of births  %lu\n", m_iNumBirths);
    printf("*** total number of deaths  %lu\n", m_iNumDeaths);
    printf("*** total number of moves   %lu\n", m_iNumMoves);

}


//----------------------------------------------------------------------------
// prepareLists
//
template<typename T>
void SPopulation<T>::prepareLists(int iAgentLayerSize,int iListLayerSize, uint32_t *aulState) {

    m_dNumAgentsPerCell = new double[m_iNumCells];

    m_pAgentController = new LBController;

    m_aAgents.init(iAgentLayerSize);
    m_pAgentController->init(iAgentLayerSize);
    m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aAgents));

    m_pWriteCopyController = new LBController;

    m_aWriteCopy.init(iAgentLayerSize);
    m_pWriteCopyController->init(iAgentLayerSize);
    m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
    m_pWriteCopyController->addLayer();

    printf("Making array lists (%d entries)\n", m_iNumThreads);
    m_vMoveList  = new std::vector<int>*[m_iNumThreads];
    m_vBirthList = new std::vector<int>*[m_iNumThreads];
    m_vDeathList = new std::vector<int>*[m_iNumThreads];
    m_apWELL = new WELL512*[m_iNumThreads];
#ifdef OMP_A
#pragma omp parallel 
    {
#endif
        int iT = omp_get_thread_num();
        unsigned int temp[STATE_SIZE]; 
        for (unsigned int j = 0; j < STATE_SIZE; j++) {
            temp[j] = aulState[(iT+13*j)%16];
        }
        // create new objects to make sure array[i] is on processor i
        m_apWELL[iT]     = new WELL512(temp);
        m_vMoveList[iT]  = new std::vector<int>;
        m_vBirthList[iT] = new std::vector<int>;
        m_vDeathList[iT] = new std::vector<int>;
#ifdef OMP_A
    }
#endif
    m_pReuseB  = new int[BQD_SIZE*m_iMaxReuse];
    m_pNormalB = new int[BQD_SIZE*m_iMaxNormalB];
    m_pPrevD   = new int[m_iMaxNormalD];

}

//----------------------------------------------------------------------------
// showStates
//
template<typename T>
void SPopulation<T>::showStates() {
    for (int i = 0; i < m_iNumThreads; i++) {
        printf("[%08x] ", m_apWELL[i]->getIndex());
        const uint32_t *p = m_apWELL[i]->getState();
        for (uint j = 0; j < STATE_SIZE;j++) {
            printf("%08x ", p[j]);
        }
        printf("\n");
    }
}


//----------------------------------------------------------------------------
// randomize
//  do some random numbers 
//  (this leads to a shifted random number sequence for the simulation)
//
template<typename T>
void SPopulation<T>::randomize(int i) {
    printf("WARNING!!!  RANDOMIZE WAS CALLED!!!\n");
    int iT = omp_get_thread_num();
    double dDummy = 0;
    for (unsigned int j = 0; j < STATE_SIZE; j++) {
        for (int k = 0; k < i; k++) {
            dDummy += m_apWELL[iT]->wrandd();
        }
    }
    
}


//----------------------------------------------------------------------------
// getUID
//  thread i gives IDs i, numthr+i, 2*NumThr+i etc
//
template<typename T>
idtype SPopulation<T>::getUID() {
    int iThread = omp_get_thread_num();
    return m_apIDGen[iThread]->getID();
}


//----------------------------------------------------------------------------
// getNumAgents
//
template<typename T>
ulong SPopulation<T>::getNumAgents(int iCellIndex) {

    return (ulong)(m_dNumAgentsPerCell[iCellIndex]);
}


//----------------------------------------------------------------------------
// getNumAgents
//
template<typename T>
ulong SPopulation<T>::getNumEvents(int iEventMask) {
    ulong iNum = 0;
    if ((iEventMask & EVMSK_BIRTHS) != 0) {
        iNum += m_iNumBirths;
    }
    if ((iEventMask & EVMSK_MOVES) != 0) {
        iNum += m_iNumMoves;
    }
    if ((iEventMask & EVMSK_DEATHS) != 0) {
        iNum += m_iNumDeaths;
    }
    return iNum;
}


//----------------------------------------------------------------------------
// setPrioList
//
template<typename T>
uint SPopulation<T>::setPrioList() {

    std::map<std::string, int>::const_iterator it;
    for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
        // record priority information into the Prioritizer class m_prio
        m_prio.setPrio(it->second, it->first);
    }
    return 0;
}


//----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int SPopulation<T>::preLoop() {

   int iResult = 0;

   for (uint i = 0; (iResult == 0) && (i <= m_prio.getMaxPrio()); i++) {
       for (uint j = 0; (iResult == 0) && (j < m_prio.getNumMethodsForPrio(i)); j++) { 
           Action<T> *pA = m_prio.getMethod(i,j);
           iResult = pA->preLoop();
       }
   }
   
   updateTotal();
   updateNumAgentsPerCell();

   return iResult;
}


//----------------------------------------------------------------------------
// postLoop
//
template<typename T>
int SPopulation<T>::postLoop() {

   int iResult = 0;
 
   for (uint i = 0; (iResult == 0) && (i <= m_prio.getMaxPrio()); i++) {
       for (uint j = 0; (iResult == 0) && (j < m_prio.getNumMethodsForPrio(i)); j++) { 
            Action<T> *pA = m_prio.getMethod(i,j);
            iResult = pA->postLoop();
        }
   }

   return iResult;
}

//----------------------------------------------------------------------------
// showWELLStates
//
template<typename T>
void SPopulation<T>::showWELLStates(const char *pCaption, bool bNice) {
    for (int iThread = 0; iThread < m_iNumThreads; ++iThread) {
        printf("%s State of thread %02d:", pCaption,  iThread);
        if (bNice) {
            printf("\n");
        } else {
            printf("  ");
        }
        const uint32_t *pCurState = m_apWELL[iThread]->getState();
        char sState[128];
        for (uint i = 0; i < STATE_SIZE/4; i++) {
            *sState = '\0';                
            for (uint j = 0; j < 4; j++) {
                char sDig[16];
                sprintf(sDig, " %08x", pCurState[4*i+j]);
                strcat(sState, sDig);
            }
            printf("    %s", sState);
            if (bNice) {
                printf("\n");
            }
        }
        printf("\n");
    }
}

//----------------------------------------------------------------------------
// initializeStep
//
template<typename T>
int SPopulation<T>::initializeStep(float fTime) {
    printf("initializeStep (%d)[%s]\n", (int)fTime, m_sSpeciesName);   
    m_fCurTime = fTime;

    int iResult = 0;

    for (uint i = 0; i <= m_prio.getMaxPrio(); i++) {
        for (uint j = 0; j < m_prio.getNumMethodsForPrio(i) && iResult == 0; j++) { 
            Action<T> *pA = m_prio.getMethod(i,j);
            iResult = pA->initialize(fTime);
        }
    }
    
    /*
    updateTotal();
    updateNumAgentsPerCell();
    initListIdx();
    printf("%d %s agents ready for step\n", m_iTotal, m_sSpeciesName);
    */

    // perhaps som statistic stuff etc.

    return iResult;
}


//----------------------------------------------------------------------------
// initLists
//
template<typename T>
void SPopulation<T>::initListIdx() {

#ifdef OMP_A
#pragma omp parallel for
#endif
   for (int i = 0; i < m_iNumThreads; i++) {
        m_vBirthList[i]->clear();
        m_vDeathList[i]->clear();
        m_vMoveList[i]->clear();
    }
}


//----------------------------------------------------------------------------
// finalizeStep
//
template<typename T>
int SPopulation<T>::finalizeStep() {    

    int iResult = 0;

    for (uint i = 0; i <= m_prio.getMaxPrio(); i++) {
        for (uint j = 0; j < m_prio.getNumMethodsForPrio(i) && iResult == 0; j++) {
            Action<T> *pA = m_prio.getMethod(i,j);
            iResult = pA->finalize(m_fCurTime);
        }
    }

    if (iResult == 0) {
        if (m_bRecycleDeadSpace) {
            recycleDeadSpaceNew();
        } else {
	    performBirths();
            performDeaths();
        }
        performMoves();
    
        // compacting at every step is too expeensive:
        //   compactData();
        
        updateTotal();
        updateNumAgentsPerCell();
        initListIdx();
        printf("%lu (effective %lu) %s agents ready after step\n", m_iTotal, m_iTotal - m_iNumPrevDeaths, m_sSpeciesName);
    } else {
        printf("finalize result: %d!!!!\n", iResult);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// compactData
//
template<typename T>
void SPopulation<T>::compactData() {
    if (m_pAgentController->getNumUsed() > 0) {
        m_pAgentController->compactData();
    }
    
    /*
    m_pAgentController->checkLists();
    fprintf(stderr,"after compact data: first index %d, last index %d, numagents %d\n",getFirstAgentIndex(),getLastAgentIndex(),m_iTotal);
    fprintf(stderr,"PASSIVE: first %d, last %d\n",m_pAgentController->getFirstIndex(PASSIVE), m_pAgentController->getLastIndex(PASSIVE));
    if (m_iTotal != getLastAgentIndex()+1) {
        for (int iA = getFirstAgentIndex(); iA <= getLastAgentIndex(); iA++) {
            if (m_aAgents[iA].m_iLifeState == 0) {
                fprintf(stderr,"agent %d born at %f is dead\n",iA,m_aAgents[iA].m_fBirthTime);
            }
        }
    }
    */
}


//----------------------------------------------------------------------------
// updateNumAgentsPerCell
//  recalculate number of agents in each cell
//
template <typename T>
void SPopulation<T>::updateNumAgentsPerCell() {
    bzero(m_dNumAgentsPerCell, m_iNumCells * sizeof(double));
    int iFirstAgent = getFirstAgentIndex();
    if (iFirstAgent != NIL) {
        int iLastAgent = getLastAgentIndex();
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
            if (m_aAgents[iAgent].m_iLifeState > 0) {
#ifdef OMP_A
#pragma omp atomic
#endif
                m_dNumAgentsPerCell[m_aAgents[iAgent].m_iCellIndex]+=1;
            }
        }
    }
}


//----------------------------------------------------------------------------
// updateTotal
//  recalculate number of agents in each cell
//
template <typename T>
void SPopulation<T>::updateTotal() {
    m_iTotal = m_pAgentController->getNumUsed();
}


//----------------------------------------------------------------------------
// doActions 
// VERY IMPORTANT METHOD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
template<typename T>
int SPopulation<T>::doActions(uint iPrio, float fTime) {

    int iResult = 0;

    int iFirstAgent = getFirstAgentIndex();
    if (iFirstAgent != NIL) {
        int iLastAgent = getLastAgentIndex();
        for (uint iMethod = 0; iMethod < m_prio.getNumMethodsForPrio(iPrio); iMethod++) {
            Action<T> *pA = m_prio.getMethod(iPrio, iMethod);
#ifdef OMP_A
            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
#pragma omp parallel for schedule(static, iChunk) 
#endif
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    (*pA)(iAgent,fTime);
                }
            }
        }
    }
    

    return iResult;
}


//----------------------------------------------------------------------------
// recycleDeadSpaceNew
//     use the space of a dying agent to place a baby (no need for
//     unlinking and relinking; parallelizable)
//
//  - find out how many spaces can be reused (min(NumBirths,NumPrevDeaths))
//  - allocate arrays for holding birth data (m_pReuseB) for reused spaces
//  - allocate arrays for holding birth data (m_pNormalB) for normal birth
//  - collect iNumReuse birth infos from the m_vBirthList into m_pReuseB
//  - move the rest to m_pNormalB 
//  - create iNumReuse babies at indexes taken from m_pPrevD
//  - perform normal births for all in pNormalB
//  - perform normal death for all left overs in m_pPrevD
//  - collect all deaths for next step in m_pPrevD
//
template<class A>
int  SPopulation<A>::recycleDeadSpaceNew() {
    ulong iNumDeaths = 0;
    ulong iNumBirths = 0;
    // first count number of births and deaths
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        iNumDeaths += m_vDeathList[iThread]->size();
        iNumBirths += m_vBirthList[iThread]->size() / BQD_SIZE;
    }

    printf("  %s  % 6zd births r\n", m_sSpeciesName, iNumBirths);
    m_iNumBirths += iNumBirths;
    printf("  %s  % 6zd deaths r\n", m_sSpeciesName, iNumDeaths);
    m_iNumDeaths += iNumDeaths;

    // number of reusable dead spaces: min(iNumDeaths, iNumBirths)
    ulong iNumReuse = (m_iNumPrevDeaths<iNumBirths)?m_iNumPrevDeaths:iNumBirths;

    if (iNumReuse > 0) {
        // resize data vectors for dead space recycling if necessary
        if (iNumReuse > m_iMaxReuse) {
            m_iMaxReuse = 2 * iNumReuse;
            delete[] m_pReuseB;
            // data vectors for dead space recycling
            m_pReuseB = new int[m_iMaxReuse*BQD_SIZE];
        }
    }
    
    // we know: iNumBirths >= iNumReuse
    ulong iNumBNormal = iNumBirths - iNumReuse;
    // allocate space for indexes of "normally" born agents
    if (iNumBNormal > 0) {
        if (iNumBNormal > m_iMaxNormalB) {
            m_iMaxNormalB = 2 * iNumBNormal;
            delete[] m_pNormalB;
            m_pNormalB = new int[m_iMaxNormalB*BQD_SIZE];
        }
    }

    // calculate number of agents who have to die normally (unlinking etc)
    // we know: iNumPrevDeaths >= iNumReuse
    ulong iNumDNormal = m_iNumPrevDeaths - iNumReuse;

    
    // copy from birthlist to pReuseB until we have iNumReuse items;
    // copy rest to pNormalB
    ulong iDoneB = 0;
    ulong iDoneBN = 0;
    ulong iToDoB = iNumReuse*BQD_SIZE;
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        ulong iMax = m_vBirthList[iThread]->size();
        if (iMax > iToDoB) {
            iMax = iToDoB;
        }
        if (iMax > 0) {
            // we still need more data for the recycling
            std::copy(m_vBirthList[iThread]->begin(), 
                      m_vBirthList[iThread]->begin()+iMax, 
                      m_pReuseB+iDoneB);
            iToDoB -= iMax;
            iDoneB += iMax;
        } else {
            iMax = 0;
        } 

        if (iMax < m_vBirthList[iThread]->size()) {
            // rest of data is for normal births
            std::copy(m_vBirthList[iThread]->begin()+iMax,  
                      m_vBirthList[iThread]->end(), 
                      m_pNormalB+iDoneBN);
            iDoneBN +=  m_vBirthList[iThread]->size()-iMax;
        }
        
    }
    
#ifdef OMP_A
    int iChunk = (int)ceil((iNumReuse+1)/(double)m_iNumThreads);
#pragma omp parallel for schedule(static, iChunk)
#endif
    // recycling: create new baby at index of killed agent
    // only use iNumReuse of previous 
    for (uint i = 0; i < iNumReuse; i++) {
#ifdef AGCHECK
        //@@ only needed for AGChecker
        printf("agcAgentDied %lu rd\n", m_aAgents[m_pPrevD[i]].m_ulID);
        //@@
#endif
        makeOffspringAtIndex(m_pPrevD[i],
                             m_pReuseB[BQD_SIZE*i],
                             m_pReuseB[BQD_SIZE*i+1],
                             m_pReuseB[BQD_SIZE*i+2]);
    }


    // now do normal births
    if (iNumBNormal > 0) {
        performBirths(iNumBNormal, m_pNormalB);
    }
    
    // now do normal deaths
    // the first iNumReuse places need not be unlinked,
    // they were overwritten with new agents
    if (iNumDNormal > 0) {
        performDeaths(iNumDNormal, m_pPrevD+iNumReuse);
    }
   
    // now prepare new deaths for reuse in next step
    // reallocate pPrevD if necessary
    if (iNumDeaths > m_iNumPrevDeaths) {
        delete[] m_pPrevD;
        m_pPrevD = new int[iNumDeaths];
    }
    // copy all new death info tom m_pPrevD
    ulong iCurOffs = 0;
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        std::copy(m_vDeathList[iThread]->begin(), 
                  m_vDeathList[iThread]->end(), 
                  m_pPrevD+iCurOffs);
        iCurOffs += m_vDeathList[iThread]->size();
    }
    // iCurOffs should be the same as iNumDeaths
    m_iNumPrevDeaths = iCurOffs;

    // loop through list and mark them as dead
    // this should not be needed since registerDeath already sets LIFE_STATE_DEAD
#ifdef OMP_A
#pragma omp parallel for 
#endif
    for (uint i = 0; i < m_iNumPrevDeaths; i++) {
        m_aAgents[m_pPrevD[i]].m_iLifeState = LIFE_STATE_DEAD;
    }
    
    return 0;   
}


//----------------------------------------------------------------------------
// registerBirth
//  register mother, father (-1 if not needed) and cell index
//
template<typename T>
void SPopulation<T>::registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex) {

    int iThreadNum = omp_get_thread_num();

    m_vBirthList[iThreadNum]->push_back(iCellIndex);
    m_vBirthList[iThreadNum]->push_back(iMotherIndex);
    m_vBirthList[iThreadNum]->push_back(iFatherIndex);

}


//----------------------------------------------------------------------------
// performBirths
//
template<typename T>
int SPopulation<T>::performBirths() {
    int   iResult = 0;
    ulong iNumBirths = 0;
    int   iBirthQueueDataSize = 3;
    
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        if (m_vBirthList[iThread]->size() > 0) {

            iNumBirths += m_vBirthList[iThread]->size() / iBirthQueueDataSize;

            for (unsigned int iIndex = 0; iIndex < m_vBirthList[iThread]->size() && iResult == 0; iIndex += iBirthQueueDataSize) {
                
                makeOffspring((*m_vBirthList[iThread])[iIndex], 
                              (*m_vBirthList[iThread])[iIndex+1], 
                              (*m_vBirthList[iThread])[iIndex+2]);

            }
        }
    }
    
    printf("  %s  %6lu births\n", m_sSpeciesName, iNumBirths);
    m_iNumBirths += iNumBirths;

    return iResult;
}


//----------------------------------------------------------------------------
// performBirths
//
template<typename T>
int SPopulation<T>::performBirths(ulong iNumBirths, int *piBirthData) {
    int iResult = 0;

    double fT0 = omp_get_wtime();
    bool bOldBirths = true;
    if ((bOldBirths) || ((int)iNumBirths < 10*m_iNumThreads)) {
        //#ifdef OLDBIRTHS
        // this loop must not be parallelized, because the linked list is manipulated    
        for (ulong iIndex = 0; iIndex < iNumBirths*BQD_SIZE; iIndex += BQD_SIZE) {
            //@@        printf("makeOffSpring %d: %d + %d\n",  piBirthData[iIndex], piBirthData[iIndex+1], piBirthData[iIndex+2]);
            makeOffspring(piBirthData[iIndex], 
                          piBirthData[iIndex+1], 
                          piBirthData[iIndex+2]);
        }
        printf(bOldBirths?"n\n":"r2\n");
        
        //#else
    } else {
        int iStart = m_pAgentController->reserveSpace2(iNumBirths);
        //        checkLists();
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (ulong iIndex0 = 0; iIndex0 < iNumBirths; iIndex0++) {
            ulong iIndex = iIndex0 * BQD_SIZE;
            makeOffspringAtIndex(iStart + iIndex0,
                                 piBirthData[iIndex], 
                                 piBirthData[iIndex+1], 
                                 piBirthData[iIndex+2]);
        }
        printf("r\n");fflush(stdout);
        //#endif
    }
    fPerfBirthTime += (omp_get_wtime() - fT0);
    printf("fPerfBirthTime[%d] now %f\n", omp_get_thread_num(), fPerfBirthTime);

    printf("  %s    %6lu births p\n", m_sSpeciesName, iNumBirths);

    return iResult;
}


///----------------------------------------------------------------------------
// makeOffspring
// crate a new agent wherever possible
//
template<typename T>
void SPopulation<T>::makeOffspring(int iCellIndex, int iMotherIndex, int iFatherIndex) {
    
    int iAgentIndex = createNullAgent(iCellIndex);

    makePopSpecificOffspring(iAgentIndex, iMotherIndex, iFatherIndex);
}


///----------------------------------------------------------------------------
// makeOffspringAtIndex
// crate a new agent wherever possible
//
template<typename T>
void SPopulation<T>::makeOffspringAtIndex(int iAgentIndex, int iCellIndex, int iMotherIndex, int iFatherIndex) {
    
    createAgentAtIndex(iAgentIndex, iCellIndex);
    
    makePopSpecificOffspring(iAgentIndex, iMotherIndex, iFatherIndex);
}


//----------------------------------------------------------------------------
// creatNullAgent
//  find free index and reset data at that location
//
template<typename T>
int  SPopulation<T>::createNullAgent(int iCellIndex) {
    
    int iFreeIndex = m_pAgentController->getFreeIndex();
    
    createAgentAtIndex(iFreeIndex, iCellIndex);
    
    return iFreeIndex;
    
}


//----------------------------------------------------------------------------
// creatAgentAtIndex
//     reset data at that location
//
template<typename T>
int  SPopulation<T>::createAgentAtIndex(int iAgentIndex, int iCellIndex) {

    resetAgent(iAgentIndex);

    // set location
    m_aAgents[iAgentIndex].m_ulCellID = this->m_pCG->m_aCells[iCellIndex].m_iGlobalID;
    m_aAgents[iAgentIndex].m_iCellIndex = iCellIndex;
    m_aAgents[iAgentIndex].m_fBirthTime = m_fCurTime;  
    int iT = omp_get_thread_num();

    m_aAgents[iAgentIndex].m_iGender = (uchar)(2*m_apWELL[iT]->wrandd());
    if (m_aAgents[iAgentIndex].m_iGender == 0) {
        // by default females are fertile so they can be mated (RandomPair)
        m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_FERTILE;
    }
    return iAgentIndex;
}


//----------------------------------------------------------------------------
// resetAgent
//  reset the agent data at specified index 
//  (called by createNullAgent)
//
template<typename T>
Agent *SPopulation<T>::resetAgent(int iAgentIndex) {

    // set ID
    m_aAgents[iAgentIndex].m_ulID = getUID(); 
    
    // set life state
    m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_ALIVE;

    return &m_aAgents[iAgentIndex];
}


//----------------------------------------------------------------------------
// registerDeath
//  register agent
//
template<typename T>
void SPopulation<T>::registerDeath(int iCellIndex, int iAgentIndex) {

    if (m_aAgents[iAgentIndex].m_iLifeState == LIFE_STATE_DEAD) {
        printf("\e[0;31mWARNING - trying to kill an already dead agent!\nCheck action priorities: all pairing actions should occur before killing actions\e[0m\n");
    }
    int iThreadNum = omp_get_thread_num();
    
    // mark agent as killed: not to be moved or otherwise used
    m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_DEAD;
    
    m_vDeathList[iThreadNum]->push_back(iAgentIndex);

}


//----------------------------------------------------------------------------
// performDeaths
//  kill all the agents on the listz
//
template<typename T>
int SPopulation<T>::performDeaths() {
    int iResult = 0;
    ulong iNumDeaths = 0;

    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        if (m_vDeathList[iThread]->size() > 0) {
            iNumDeaths += m_vDeathList[iThread]->size();

            for (unsigned int iIndex = 0; iIndex < m_vDeathList[iThread]->size() && iResult == 0; iIndex++) {
                
                iResult = m_pAgentController->deleteElement((*m_vDeathList[iThread])[iIndex]);
            }
        }
    }

    printf("  %s  %6lu deaths\n", m_sSpeciesName, iNumDeaths);
    m_iNumDeaths += iNumDeaths;

    return iResult;
}


//----------------------------------------------------------------------------
// performDeaths
//  kill all the agents on the listz
//
template<typename T>
int SPopulation<T>::performDeaths(ulong iNumDeaths, int *piDeathData) {
    int iResult = 0;

    // this loop must not be parallelized, because the linked list is manipulated    
    for (ulong iIndex = 0; (iIndex < iNumDeaths); iIndex++) {
        if (m_aAgents[piDeathData[iIndex]].m_iLifeState > 0) {
            fprintf(stderr,"deleting live agent %d!!!\n",piDeathData[iIndex]);
        }

        iResult = m_pAgentController->deleteElement(piDeathData[iIndex]);

#ifdef AGCHECK
        //@@ only needed for AGChecker
        printf("agcAgentDied %d pd\n", m_aAgents[piDeathData[iIndex]].m_ulID);
        //@@
#endif
    }
    printf("  %s    %6lu deaths p\n", m_sSpeciesName, iNumDeaths);
    return iResult;
}


//----------------------------------------------------------------------------
// flushDeadSpace
//  kill all the agents on the recycling list 
//
template<typename T>
int SPopulation<T>::flushDeadSpace() {
    int iResult = performDeaths(m_iNumPrevDeaths, m_pPrevD);
    m_iNumPrevDeaths = 0;
    updateTotal();

    return iResult;
}


//----------------------------------------------------------------------------
// moveAgent
//  move agent to different cell
//  and
//  update the move statistics
//
//  MoveStats::addStatCandidates() collects all changes to the stats.
//  If there are several changes to the same cell, only the one with the
//  shortest distance is used. That way the travelled distances should
//  be the same if hte simulation is repeated on an other machine with the
//  same parameters and the same number of threads.
//
template<typename T>
int SPopulation<T>::moveAgent(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    int iResult = 0;

    m_aAgents[iAgentIndex].m_iCellIndex = iCellIndexTo;
    m_aAgents[iAgentIndex].m_ulCellID = m_pCG->m_aCells[iCellIndexTo].m_iGlobalID;
    m_aAgents[iAgentIndex].m_iLifeState &= ~LIFE_STATE_MOVING;


    
    // do arrival hops/dist/time
    if  ((m_pCG->m_pGeography != NULL) && (m_pCG->m_pMoveStats != NULL)) {
        
       //@@@ MoveStat new begin
        if ( m_pCG->m_pMoveStats->m_aiHops[iCellIndexTo] < 0) {
            double dDist = m_pCG->m_pMoveStats->m_adDist[iCellIndexFrom];
            double dLon1 = m_pCG->m_pGeography->m_adLongitude[iCellIndexFrom];
            double dLat1 = m_pCG->m_pGeography->m_adLatitude[iCellIndexFrom];
            double dLon2 = m_pCG->m_pGeography->m_adLongitude[iCellIndexTo];
            double dLat2 = m_pCG->m_pGeography->m_adLatitude[iCellIndexTo];
            
            int    iNewHops =  m_pCG->m_pMoveStats->m_aiHops[iCellIndexFrom]+1;
            double dNewTime = m_fCurTime;
            double dNewDist = dDist + m_fCalcDist(dLon1, dLat1, dLon2, dLat2, m_dDistScale);
            
            // remember to call MoveStats::insertNewStats() after the step
            // e.g. in Simulator::unLoop()
            m_pCG->m_pMoveStats->addStatCandidates(iCellIndexTo, dNewDist, iNewHops, dNewTime);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// registerMove
//  register move parameters
//
template<typename T>
void SPopulation<T>::registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    
    //    printf("[SPopulation<T>::registerMove] registering move: %i %i %i\n", iCellIndexFrom, iAgentIndex, iCellIndexTo);

    int iThreadNum = omp_get_thread_num();
    
    // LifeState should be reset after move is completed (moveAgent())
    m_aAgents[iAgentIndex].m_iLifeState |= LIFE_STATE_MOVING;

    m_vMoveList[iThreadNum]->push_back(iCellIndexFrom);
    m_vMoveList[iThreadNum]->push_back(iAgentIndex);
    m_vMoveList[iThreadNum]->push_back(iCellIndexTo);
}


//----------------------------------------------------------------------------
// performMoves
//  do all the moves registered by registerMove()
//
template<typename T>
int SPopulation<T>::performMoves() {
    int iResult = 0;
    ulong iNumMoves = 0;  
    int iMoveQueueDataSize = 3;

    
#ifdef OMP_A
#pragma omp parallel reduction(+:iNumMoves,iResult)
#endif
    {
        int iThreadNum = omp_get_thread_num();

        if (m_vMoveList[iThreadNum]->size() > 0) {
            
            //            iNumMoves += m_auMoveListIndex[iThreadNum]/3;
            iNumMoves += m_vMoveList[iThreadNum]->size() / iMoveQueueDataSize;

            for (unsigned int iIndex = 0; iIndex < m_vMoveList[iThreadNum]->size() && iResult == 0; iIndex += iMoveQueueDataSize) {
                
                //                if (m_aAgents[ (*m_vMoveList[iThreadNum])[iIndex+1] ].m_iLifeState > 0) {
                iResult = moveAgent((*m_vMoveList[iThreadNum])[iIndex], 
                                    (*m_vMoveList[iThreadNum])[iIndex+1], 
                                    (*m_vMoveList[iThreadNum])[iIndex+2]);
                //                }
            }
        }
        //        m_pMoveListController[iThreadNum]->clear();
    }

    printf("  %s  %6lu moves\n", m_sSpeciesName, iNumMoves);
    m_iNumMoves += iNumMoves;

    
    return iResult;
}

//----------------------------------------------------------------------------
//   getNumAgentsMax
//
template<typename T>
ulong SPopulation<T>::getNumAgentsMax() { 
    return m_pAgentController->getNumLayers()*m_pAgentController->getLayerSize(); 
};


//----------------------------------------------------------------------------
// readSpeciesData
//   read data from cls file
//   First, ClassID, Class name, species ID, species name, and sensing are read
//   then each line between "BEGIN_PARAMS" and "END_PARAMS" is passed to
//   readSpeciesLine
//
template<typename T>
int SPopulation<T>::readSpeciesData(LineReader *pLR) {
    int iResult = 0;
    char *pLine = pLR->getCurLine();
   
    while ((iResult == 0) && (pLine != NULL) && (strcmp(pLine, KEY_PARAMS) != 0)) {

        if (strstr(pLine, KEY_CLASS) == pLine) {
            pLine = trim(pLine + strlen(KEY_CLASS));
            char sClassName[64];
            int iR = sscanf(pLine, "%s", sClassName);
            if (iR == 1) {
                strcpy(m_sClassName, sClassName);
                
                spcid si =  clsValue(sClassName);
                if (si != SPC_NONE) {
                    m_iClassID = si;
                    iResult = 0;
                } else {
                    printf("Not a known class [%s]\n", sClassName);
                    iResult = -1;
                }
            } else {
                printf("Expected class name after prefix \"%s\" [%s]\n", KEY_CLASS, pLine);
                iResult = -1;
            }
            
        } else if (strstr(pLine, KEY_SPECIES) == pLine) {
            pLine = trim(pLine + strlen(KEY_SPECIES));
            char sID[64];
            char sName[64];
    
            int iR = sscanf(pLine, "%s %s", sID, sName);
            if (iR == 2) {
                strcpy(m_sSpeciesName, sName);
                
                if (strToNum(sID, &m_iSpeciesID)) {
                    iResult = 0;
                } else {
                    spcid si =  spcValue(sID);
                    if (si != SPC_NONE) {
                        m_iSpeciesID = si;
                        iResult = 0;
                    } else {
                        printf("Not a known ID [%s]\n", sID);
                        iResult = -1;
                    }
                }
            } else {
                printf("Expected [id and] name after prefix \"%s\" [%s]\n", KEY_SPECIES, pLine);
                iResult = -1;
            }
        } else if (strstr(pLine, KEY_SENSING) == pLine) {
            pLine = trim(pLine + strlen(KEY_SENSING));
            if (strToNum(pLine, &m_iSensingDistance)) {
                iResult = 0;
            } else {
                printf("Invalid \"%s\"-line [%s]\n", KEY_SENSING, pLine);
                iResult = -1;
            }
        }
        pLine = pLR->getNextLine();
    }
    if ((iResult == 0) && (pLine != NULL)) {
        // no let the rest check the lines
        m_mPrioInfo.clear();
        pLine = pLR->getNextLine();
 
        while ((iResult == 0) && (pLine != NULL) && (strcmp(pLine, KEY_ENDPARAMS) != 0)) {

            // let's try to read prio info in this line
            iResult = this->readPrioInfo(pLine);

            // if not, maybe the line contains a parameter? 
            if (iResult != 0) {
                iResult = m_prio.readActionParamLine(pLine) - 1;
            }

            pLine = pLR->getNextLine();
        }

    } else {
        printf("No \"%s\" found\n", KEY_PARAMS);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addAgent
//
template<typename T>
int  SPopulation<T>::addAgent(int iCellIndex, char *pData) {
    int iResult = 0;
    
    uint iFreeIndex = m_pAgentController->getFreeIndex();
    m_aAgents[iFreeIndex].m_ulCellID = m_pCG->m_aCells[iCellIndex].m_iGlobalID;
    m_aAgents[iFreeIndex].m_iCellIndex = iCellIndex;

    if (m_pCG->m_pMoveStats != NULL) {
        if (m_pCG->m_pMoveStats->m_aiHops[iCellIndex] < 0) {
            m_pCG->m_pMoveStats->m_adDist[iCellIndex] = 0;
            m_pCG->m_pMoveStats->m_aiHops[iCellIndex] = 0;
            m_pCG->m_pMoveStats->m_adTime[iCellIndex] = 0;
        }
    }

    iResult = addAgentData(iCellIndex, iFreeIndex, &pData);
    // remember highest ID
    if (m_aAgents[iFreeIndex].m_ulID > m_iMaxID) {
        m_iMaxID = m_aAgents[iFreeIndex].m_ulID;
    }

    updateTotal();
    updateNumAgentsPerCell();
    return iResult;
}


//----------------------------------------------------------------------------
// addAgentData
//
template<typename T>
int  SPopulation<T>::addAgentData(int iCellIndex, int iAgentIndex, char **ppData) {
    int iResult = 0;

    //    printf("[SPopulation::addAgentData] got [%s]\n", *ppData);

    // must read
    //  uint   m_iLifeState;
    //  gridtype m_ulID;

    //  alerady set by in addAgent()
    //  int    m_iCellIndex;
    //  gridtype m_ulCellID;

    iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iLifeState);
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_ulID);
    } else {
        printf("[addAgentData] Couldn't read m_iLifeState from [%s]\n", *ppData);
    }
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fBirthTime);
    } else {
        printf("[addAgentData] Couldn't read m_ulID from [%s]\n", *ppData);
    }
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iGender);
    } else {
        printf("[addAgentData] Couldn't read m_fBirthTime from [%s]\n", *ppData);
    }

    if (iResult == 0) {
        iResult = this->addPopSpecificAgentData(iAgentIndex,ppData);
    } else {
        printf("[addAgentData] Couldn't read m_iGender from [%s]\n", *ppData);
    }

    if (iResult != 0) {
        printf("[addAgentData] Couldn't read iAgentIndexr from [%s]\n", *ppData);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// addAgentDataSingle 
// multi-use template of addAgentData basic functionality
//
template<typename T>
template<typename T1>
int  SPopulation<T>::addAgentDataSingle(char **ppData, T1 *pAgentDataMember) {
    int iResult = 0;

    char *pEnd;
    
    // LifeState
    if (**ppData != '\0') {
        char *p = nextWord(ppData);
        T1 x = (T1) strtod(p, &pEnd);
        if (*pEnd == '\0') {
            *pAgentDataMember = x;
        } else {
            iResult = -1;
        }
    } else {
        iResult = -1;
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createAgentDataTypeQDF
//  Create the HDF5 datatype for Agent data
//
template<typename T>
hid_t  SPopulation<T>::createAgentDataTypeQDF() {

    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, agentRealSizeQDF());

    T ta;

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta, m_iGender),    H5T_NATIVE_UCHAR);

    this->addPopSpecificAgentDataTypeQDF(&hAgentDataType);

    return hAgentDataType;
}


//----------------------------------------------------------------------------
// checkLists
//
template<typename T>
int SPopulation<T>::checkLists() { 
    int iResult = 0;
    printf("m_pAgentController check:\n");
    int i1 = m_pAgentController->checkLists();
    if (i1 == 0) {
        printf("ok\n");
    }

    iResult += i1;
    printf("m_pWriteCopyController check:\n");
    int i2 = m_pWriteCopyController->checkLists();
    if (i2 == 0) {
        printf("ok\n");
    }
    iResult += i2;
    
    fflush(stdout);

    return iResult;
}


//----------------------------------------------------------------------------
// writeAgentDataQDF
//
template<typename T>
int  SPopulation<T>::writeAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;

    printf("[SPopulation<T>::writeAgentDataQD][%s] at step %d:\n", m_sSpeciesName, (int) m_fCurTime);
    printf("  total number of births  %lu\n", m_iNumBirths);
    printf("  total number of deaths  %lu\n", m_iNumDeaths);
    printf("  total number of moves   %lu\n", m_iNumMoves);

    iResult = writeAgentDataQDFSafe(hDataSpace, hDataSet, hAgentType);

    return iResult;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------- I/O-------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//-- writeAgentDataQDFSafe
//-- readAgentDataQDF
//-- writeSpeciesDataQDF
//-- readSpeciesDataQDF
//-- writeAdditionalDataQDF
//-- readAdditionalDataQDF
//-- readPrioInfo
//-- insertPrioDataAttribute
//-- extractPrioDataAttribute
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// writeAgentDataQDF
//  write data using hyperslabs
//  - create a file handle
//  - create a dataspace for the file (sized to hold entire array aData)
//  - create a dataspace for the slab (sized to hold one slab)
//  - create the data type for the structure
//  - create the data set
//  - write the data
//  This method copies each layer of m_aAgents to the first layer of m_aWriteCopy
//  The copied layer is then compacted and passed to  H5Dwrite
//  
template<typename T>
int  SPopulation<T>::writeAgentDataQDFSafe(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;
    printf("[SPopulation::writeAgentDataQDFSafe]\n");
    fflush(stdout);
    m_pAgentController->calcHolyness();
    // make sure there is a  layer in WriteCopyController 
    //(it may have been freed in the previous call to writeAgentDataQDFSafe, if the layer was empty after killing the dead)
    if (m_pWriteCopyController->getNumLayers() == 0) {
    	m_pWriteCopyController->addLayer();
    }

    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    hsize_t offset = 0;
    hsize_t count  = 0;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;
    
    // make sure there are no holes and no forgotten dead
    // m_vMergedDeadList is used by other objects (e.g. Genetics)
    m_vMergedDeadList.clear();
    m_vMergedDeadList.insert(m_vMergedDeadList.end(), m_pPrevD, m_pPrevD+m_iNumPrevDeaths);
    std::sort(m_vMergedDeadList.begin(), m_vMergedDeadList.end());
        
    int iTotalKill = 0;
    uint iNumWritten = 0;
    herr_t status = H5P_DEFAULT;
    uint iD = 0;
    int iLayerSize = m_pWriteCopyController->getLayerSize();
    
    if (m_pAgentController->getNumUsed() > 0) {
        
        for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumUsedLayers()); j++) {
            // empty layers will be removed in compactData, which leads to an undefinde state when used
            if (m_pAgentController->getNumUsed(j) > 0) {

                // write agents of layer j as hyperslab
                const T* pSlab0 = m_aAgents.getLayer(j);
            
                m_aWriteCopy.copyLayer(0, pSlab0);
                m_pWriteCopyController->setL2List(m_pAgentController->getL2List(j), 0);

                // remove dead in slab
                uint iNumKilled  = 0;
                while ((iD < m_vMergedDeadList.size()) && (m_vMergedDeadList[iD] < (int)(j+1)*iLayerSize)) {
                    iResult = m_pWriteCopyController->deleteElement(m_vMergedDeadList[iD] - j*iLayerSize);
                    iD++;
                    iNumKilled++;
                }
                iTotalKill += iNumKilled;

                // here it is important not to have an empty slab
                m_pWriteCopyController->compactData();
            
                const T* pSlab = m_aWriteCopy.getLayer(0);
                count =  m_pWriteCopyController->getNumUsed(0);
                iNumWritten +=  count;

                // adapt memspace if size of slab is different
                if (count != dimsm) {
                    qdf_closeDataSpace(hMemSpace); 
                    dimsm = count;
                    hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                }
            
                //			printf("selecting hyperslab:\n offset %d\n count %d\n", offset, count);
                
                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &offset, &stride, &count, &block);
            
                status = H5Dwrite(hDataSet, hAgentType, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pSlab);
                offset += count;
            } else {
                printf("[SPopulation<T>::writeAgentDataQDFSafe] ignored layer %d because it's empty\n", j);
            }
        }
        
    } else {
        // no agents at all
    }
    
    qdf_closeDataSpace(hMemSpace); 

    printf("[SPopulation<T>::writeAgentDataQDFSafe] written %u agents, killed %d\n", iNumWritten, iTotalKill); fflush(stdout);
    printf("[SPopulation<T>::writeAgentDataQDFSafe] end with status %d\n", status); fflush(stdout);

    return (status >= 0)?iResult:-1;
}



#define ABUFSIZE 16384
//----------------------------------------------------------------------------
// readAgentDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {

    int iResult = 0;
    T aBuf[ABUFSIZE];
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    hsize_t iOffset = 0;
    hsize_t iCount  = 0;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
    // compacting seems ok, since we're changing the occupancy anyway
    compactData();

    updateTotal();

    while ((iResult == 0) && (dims > 0)) {
        if (dims > ABUFSIZE) {
            iCount = ABUFSIZE;
        } else {
            iCount = dims;
        }

        // read a buffer full
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dread(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, aBuf);
        if (status >= 0) {

            uint iFirstIndex = m_pAgentController->reserveSpace2((uint)iCount);
            m_aAgents.copyBlock(iFirstIndex, aBuf, (uint)iCount);
            for (uint j =0; j < iCount; j++) {
#ifdef AGCHECK
                //@@ only needed for AGChecker
                printf("agcInitial %d\n", aBuf[j].m_ulID);
                //@@
#endif
                if (aBuf[j].m_ulID > m_iMaxID) {
                    m_iMaxID = aBuf[j].m_ulID;
                }
            }
 
            dims -= iCount;
            iOffset += iCount;

        } else {
            iResult = -1;
        }
    }
 
    updateTotal();

    updateNumAgentsPerCell();

    return iResult;
}

//----------------------------------------------------------------------------
// writeSpeciesDataQDF
//  write species data as attributes
//
template<typename T>
int  SPopulation<T>::writeSpeciesDataQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    int iC = m_iClassID;
    int iS = m_iSpeciesID;
    if (iResult == 0) {
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_ID, 1, &iC);
    }
    if (iResult == 0) {
        iResult =  qdf_insertSAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_NAME, m_sClassName);
    }
    if (iResult == 0) {
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_ID, 1, &iS);
    }
    if (iResult == 0) {
        iResult =  qdf_insertSAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_NAME, m_sSpeciesName);
    }
    if (iResult == 0) {
        int i = m_iSensingDistance;
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_SENS_DIST, 1, &i);
    }
    if (iResult == 0) {
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_NUM_CELL, 1, &m_iNumCells);
    }

    if (iResult == 0) {
        iResult =  insertPrioDataAttribute(hSpeciesGroup);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_INIT_SEED, STATE_SIZE, m_aulInitialState); 
    }

    if (iResult == 0) {
        iResult = m_prio.writeActionParamsQDF(hSpeciesGroup);
    }

    return iResult;

}

//----------------------------------------------------------------------------
// readSpeciesDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::readSpeciesDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (iResult == 0) {
        int iC;
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_ID, 1, &iC);
        m_iClassID = (spcid) iC;
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_NAME, MAX_NAME, m_sClassName);
    }

    if (iResult == 0) {
        int iS;
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_ID, 1, &iS);
        m_iSpeciesID = (spcid) iS;
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_NAME, MAX_NAME, m_sSpeciesName);
    }        

    if (iResult == 0) {
        int iD;
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_SENS_DIST, 1, &iD);
        m_iSensingDistance = (size) iD;
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_NUM_CELL, 1, &m_iNumCells);
    }

    // now read action-specific parameters

    if (iResult == 0) {
        iResult = m_prio.extractActionParamsQDF(hSpeciesGroup);
    }

    // finally, read priority information

    if (iResult == 0) {
        iResult =  extractPrioDataAttribute(hSpeciesGroup);
    }
 
    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
template<typename T>
int  SPopulation<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
      
    return 0;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  read additional data from the group
//  (data stored separately from agent data)
//
template<typename T>
int  SPopulation<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    return 0;
}


//----------------------------------------------------------------------------
// readPrioInfo
//  Read priority info ("PRIO <NAME> <PRIORITY>")
//  and fill m_mPrioInfo (map<string, uint>)
//
template<typename T>
int  SPopulation<T>::readPrioInfo(char *pLine) {
    int iResult = -1;
    if (strstr(pLine, KEY_PRIO) == pLine) {
        char *p0 = strtok(pLine, " \t\n"); // strtok
        p0 = strtok(NULL,  " \t\n"); // action 
        char *p1 = strtok(NULL,  " \t\n"); // prio
        int iPrio=-1;
        if ((p0 != NULL) && (p1 != NULL) && strToNum(p1, &iPrio)) {
            if (iPrio >= 0) {
                m_mPrioInfo[p0] = iPrio;
                iResult = 0;
            } else {
                iResult = -1;
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertPrioDataAttribute
//  create an array suitable for HDF5 reading/writing
//
template<typename T>
int SPopulation<T>::insertPrioDataAttribute(hid_t hSpeciesGroup) {
    int iResult = -1;
 
  
    ulong iNum = m_mPrioInfo.size();
    // find length of longest string
    ulong iMaxLen = 0;
    std::map<std::string, int>::const_iterator it;
    for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
        ulong iL = it->first.length();
        if (iL > iMaxLen) {
            iMaxLen = iL;
        }
    }
    iMaxLen++; // account for terminating 0
    
    if (iMaxLen > MAX_FUNC_LEN) {
        printf("Can't translate PrioInfo to array: function name too long (%lu > %u)\n", iMaxLen, MAX_FUNC_LEN);
    } else {
 
        // create an array to hold the map's data
        PrioData *paPD = new PrioData[iNum];
        PrioData *pCur = paPD;
        // bytes between string end and number are uninitialised
        // to prevent valgrind nagging: initialise all
        memset(paPD, 77, iNum*sizeof(PrioData));
        for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
            strcpy(pCur->m_sFunction, it->first.c_str());
            pCur->m_iPrioVal = it->second;
            pCur++;
        }

        // string type used in PrioData
        hid_t strtype = H5Tcopy (H5T_C_S1);         /* Make a copy of H5T_C_S1 */
        hsize_t sizeMF = MAX_FUNC_LEN;
        herr_t status = H5Tset_size (strtype, sizeMF); 
        // define the data type for PrioData    
        hid_t hPrioDataType = H5Tcreate (H5T_COMPOUND, sizeof(PrioData));
        H5Tinsert(hPrioDataType, SPOP_AT_FUNCNAME,  HOFFSET(PrioData, m_sFunction), strtype);
        H5Tinsert(hPrioDataType, SPOP_AT_FUNCPRIO,  HOFFSET(PrioData, m_iPrioVal),  H5T_NATIVE_INT);

        // create and write the attribute
        hsize_t dims =  m_mPrioInfo.size();
        hid_t  hSpace = H5Screate_simple (1, &dims, NULL);
        hid_t  hAttr = H5Acreate (hSpeciesGroup, SPOP_ATTR_PRIO_INFO, hPrioDataType, hSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT);
        status = H5Awrite (hAttr, hPrioDataType, paPD);

        // close handles
        qdf_closeDataType(hPrioDataType);
        qdf_closeDataSpace(hSpace);
        qdf_closeAttribute(hAttr);
        iResult = (status >= 0)?0:-1;
        delete[] paPD;
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// extractPrioDataAttribute
//  create an array suitable for HDF5 reading/writing
//
template<typename T>
int SPopulation<T>::extractPrioDataAttribute(hid_t hSpeciesGroup) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hSpeciesGroup, SPOP_ATTR_PRIO_INFO)) {
        hid_t hAttribute = H5Aopen_name(hSpeciesGroup, SPOP_ATTR_PRIO_INFO);
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

       
        // define the data type for PrioData    
        hid_t hPrioDataType = H5Aget_type(hAttribute);

        if (rank == 1)  {
            PrioData *paPD = new PrioData[dims[0]];
            
            status = H5Aread(hAttribute, hPrioDataType, paPD);
            m_mPrioInfo.clear();
            if (status == 0) {
                for (uint i = 0; i < dims[0]; ++i) {
                    m_mPrioInfo[paPD[i].m_sFunction] = paPD[i].m_iPrioVal;
                }
                iResult = 0;

            } else {
                printf("read priodata attribute err\n");
            } 


            delete[] paPD;
        } else {
            printf("Bad Rank (%d)\n", rank);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
        qdf_closeDataType(hPrioDataType);
    } else {
        printf("Attribute [%s] does not exist\n", SPOP_ATTR_PRIO_INFO);
    }


    return iResult;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------- dump/restore ---------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//-- dumpAgentDataQDF
//-- restoreAgentDataQDF
//-- dumpSpeciesDataQDF
//-- restoreSpeciesDataQDF
//-- dumpController
//-- restoreController
//-- dumpDeadSpaces
//-- restoreDeadSpaces
//-- 
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// dumpAgentDataQDF
//   like writeAgentDataQDF, but writes *all* layers without compacting them
//   and without removing dead spaces
//   The normal readAgentDataQDF will restore the original layer layout when
//   reading this output.
//
template<typename T>
int  SPopulation<T>::dumpAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;
    printf("[SPopulation::dumpAgentDataQDFSafe]\n");
    fflush(stdout);


    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    hsize_t offset = 0;
    hsize_t count  = dimsm;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;

    // we don't remove dead spaces
  
    herr_t status = H5P_DEFAULT;
        
    for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumLayers()); j++) {
        // write agents of layer j as hyperslab
        const T* pSlab = m_aAgents.getLayer(j);
        
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &offset, &stride, &count, &block);
            
        status = H5Dwrite(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, pSlab);
        offset += count;
    }
    
    qdf_closeDataSpace(hMemSpace); 

    printf("[SPopulation<T>::dumpAgentDataQDFSafe written %d layers of size %d\n",  m_aAgents.getNumLayers(), m_pWriteCopyController->getLayerSize());

    return (status >= 0)?iResult:-1;
}


//----------------------------------------------------------------------------
// restoreAgentDataQDF
//  restoresead agent data from a fump
//
template<typename T>
int  SPopulation<T>::restoreAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {

    int iResult = 0;
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);


    T *pSlab = new T[dims];

    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    hsize_t offset = 0;
    hsize_t count  = dimsm;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;

    // we don't remove dead spaces
  
    status = H5P_DEFAULT;
        
    for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumLayers()); j++) {
        // write agents of layer j as hyperslab
        
        
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &offset, &stride, &count, &block);
            
        status = H5Dread(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, pSlab);
        
        m_aAgents.copyLayer(j, pSlab);
        offset += count;
    }
    
    qdf_closeDataSpace(hMemSpace); 

 
    updateTotal();

    updateNumAgentsPerCell();

    // we must find highest ID
    idtype maxid = 0;

#pragma omp parallel for reduction(max:maxid)
    for (int iA = getFirstAgentIndex(); iA <=  getLastAgentIndex(); iA++) {
        if (m_aAgents[iA].m_ulID > maxid) {
            maxid = m_aAgents[iA].m_ulID;
        }
    }
    m_iMaxID = maxid;
    printf("Found mac ID: %ld\n", m_iMaxID);
    delete[] pSlab;
    iResult = (status >= 0)?iResult:-1;
    return iResult;
}


//----------------------------------------------------------------------------
// dumpSpeciesDataQDF
//  dump species data as attributes plus full restore data
//
template<typename T>
int  SPopulation<T>::dumpSpeciesDataQDF(hid_t hSpeciesGroup, int iDumpMode) {
    printf("dumping species data\n");

    int iResult = 0;

    // "normal" attributes output
    iResult = writeSpeciesDataQDF(hSpeciesGroup);
    // for full recovery
    
    // number of WELL states
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_NUM_STATES, 1, &m_iNumThreads); 
    }

    // current indexes of the states
    uint32_t *auiIndexes = new uint32_t[m_iNumThreads];
    if (iResult == 0) {
        for (int i = 0; i < m_iNumThreads; i++) {
            auiIndexes[i] = m_apWELL[i]->getIndex();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_CUR_INDEX, m_iNumThreads, auiIndexes); 
    }
    delete[] auiIndexes;

    // the WELL states
    if (iResult == 0) {
            
        uint32_t *pSuperState = new uint32_t[m_iNumThreads*STATE_SIZE];
        uint32_t *pCur = pSuperState;
        for (int i = 0; i < m_iNumThreads; i++) {
            memcpy(pCur, m_apWELL[i]->getState(), STATE_SIZE*sizeof(uint32_t));
            pCur +=  STATE_SIZE;
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_FINAL_WELL, m_iNumThreads*STATE_SIZE, pSuperState); 
        delete[] pSuperState;
    }
    
    // probably not the correct place, because IDGen is app-global (not species specific)
    if (iResult == 0) {
        idtype *pIDGBuf = new idtype[m_iNumThreads];
        for (int i = 0; i < m_iNumThreads; i++) {
            pIDGBuf[i] = m_apIDGen[i]->getCur();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_IDGEN_STATE, m_iNumThreads, pIDGBuf);
        delete[] pIDGBuf;
    }

    if (iResult == 0) {
        iResult = dumpDeadSpaces(hSpeciesGroup);
    }

    if (iResult == 0) {
        iResult = dumpController(hSpeciesGroup, iDumpMode);
    }

    printf("After dump\n");
    showStates();

    return iResult;

}



//----------------------------------------------------------------------------
// restoreSpeciesDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::restoreSpeciesDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    printf("restoring species data\n");

    iResult = readSpeciesDataQDF(hSpeciesGroup);

    if (iResult == 0) {
        int iNumStates = 0;
        

        // number of WELL states dumped
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_NUM_STATES, 1, &iNumStates); 
        uint32_t *auiIndexes = new uint32_t[iNumStates];
    
        if (iResult == 0) {
            // curernt index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_CUR_INDEX, iNumStates, auiIndexes); 
        }            

        if (iResult == 0) {
            // The WELL states themselves
            
            uint32_t *pSuperState = new uint32_t[iNumStates*STATE_SIZE];
            uint32_t *pCur = pSuperState;
            iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_FINAL_WELL, iNumStates*STATE_SIZE, pSuperState); 
            if (iResult == 0) {
                int iCopy = (iNumStates < m_iNumThreads)?iNumStates:m_iNumThreads;
                int i = 0;
                while (i < iCopy) {
                    m_apWELL[i]->seed(pCur, auiIndexes[i]);
                    pCur +=  STATE_SIZE;
                    i++;
                }
                // if there are more threads than saved states, the WELLs keep their states from the constructor
                // it doesn't matter, because continuing with a different number of threads changes everxthing
            }
            printf("WELL states after restore\n");
            showStates();

            delete[] pSuperState;
            delete[] auiIndexes;
        }
        if (iResult != 0) {
            // num states was missing, or states could not be read
            printf("[readSpeciesDataQDF] num states was missing, or states could not be read. No full recovery possible\n");
            iResult = 0;
        }
    }
    
    // this is not really the place to store IDGen state, because it is app-global,
    // and not per species.
    // it should be ok though for multiplepops, because all would write the same values
    if (iResult == 0) {
        idtype *pIDGBuf = new idtype[m_iNumThreads];
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_IDGEN_STATE, m_iNumThreads, pIDGBuf);
        for (int i = 0; i < m_iNumThreads; i++) {
            m_apIDGen[i]->setCur(pIDGBuf[i]);
        }
        delete[] pIDGBuf;
    }

    if (iResult == 0) {
        iResult = restoreDeadSpaces(hSpeciesGroup);
    }
    
    if (iResult == 0) {
        iResult = restoreController(hSpeciesGroup);
    }

    return iResult;
}


#define TEMP_SIZE 1048576
//----------------------------------------------------------------------------
// dumpController
//
template<typename T>
int  SPopulation<T>::dumpController(hid_t hSpeciesGroup, int iDumpMode) {
    printf("dumping contoller\n");
    int iResult = 0;
    hsize_t iNumWritten = 0;
    herr_t status=-1;

    m_pAgentController->calcHolyness();
    // amount of space needed to save controller serialisation
    hsize_t dims = m_pAgentController->getBufSize(iDumpMode);
    uchar *pBuf = new uchar[dims];
    m_pAgentController->serialize(pBuf);
    
    uchar *pTempBuf = pBuf;
    hsize_t iCount = TEMP_SIZE;

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, CTRL_DATASET_NAME, H5T_NATIVE_UCHAR, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
       
        if (hDataSet > 0) {

            while (iNumWritten < dims) {
                if ((dims - iNumWritten) > TEMP_SIZE) {
                    iCount = TEMP_SIZE; 
                } else {
                    iCount = dims - iNumWritten;
                    qdf_closeDataSpace(hMemSpace); 
                    hMemSpace = H5Screate_simple (1, &iCount, NULL); 
                }

                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &iNumWritten, NULL, &iCount, NULL);
                
                status = H5Dwrite(hDataSet, H5T_NATIVE_UCHAR, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pTempBuf);
                
                iNumWritten += iCount;
                pTempBuf    += iCount;
            }
            
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }
    delete[] pBuf;

    return (status >= 0)?iResult:-1;
}

//----------------------------------------------------------------------------
// restoreController
//
template<typename T>
int  SPopulation<T>::restoreController(hid_t hSpeciesGroup) {
    int iResult = -1;
    printf("[SPopulation<T>::restoreController] checking lists before\n");
    checkLists();
    
    if (qdf_exists(hSpeciesGroup, CTRL_DATASET_NAME)) {
        iResult = 0;

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, CTRL_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        printf("Dataspace extent: %lld\n", dims);

        hsize_t iCount =  TEMP_SIZE;
        hsize_t iNumRead = 0;
        uchar *pBuf = new uchar[dims];
        uchar *pTempBuf = pBuf;
        
        
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 

        while (iNumRead < dims) {
             if ((dims - iNumRead) > TEMP_SIZE) {
                 iCount = TEMP_SIZE; 
             } else {
                 iCount = dims - iNumRead;
                 qdf_closeDataSpace(hMemSpace); 
                 hMemSpace = H5Screate_simple (1, &iCount, NULL); 
             }
             status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                          &iNumRead, NULL, &iCount, NULL);
             status = H5Dread(hDataSet, H5T_NATIVE_UCHAR, hMemSpace,
                              hDataSpace, H5P_DEFAULT, pTempBuf);
             iNumRead += iCount;
             pTempBuf += iCount;
        }

        if (status >= 0) {
            iResult = m_pAgentController->deserialize(pBuf);

            if (iResult == 0) {
                printf("[SPopulation<T>::restoreController] checking lists after\n");
                checkLists();
            }

        }
        delete[] pBuf;
        
    } else {
        printf("WARNING: no dataset [%s] found\n", CTRL_DATASET_NAME);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// dumpDeadSpaces
//
template<typename T>
int  SPopulation<T>::dumpDeadSpaces(hid_t hSpeciesGroup) {
    printf("dumping dead spaces\n");
    int iResult = 0;
    uint iNumWritten = 0;
    herr_t status = 0; // status won't be changed if num desd spaces is 0

    // amount of elements we have to save
    hsize_t dims = m_iNumPrevDeaths;
    
    int    *pTempBuf = m_pPrevD;
    hsize_t iOffset = 0;
    hsize_t iCount = TEMP_SIZE;

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, DEAD_DATASET_NAME, H5T_NATIVE_INT, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
       
        if (hDataSet > 0) {

            while (iNumWritten < dims) {
                if ((dims - iNumWritten) > TEMP_SIZE) {
                    iCount = TEMP_SIZE; 
                } else {
                    iCount = dims - iNumWritten;
                    qdf_closeDataSpace(hMemSpace); 
                    hMemSpace = H5Screate_simple (1, &iCount, NULL); 
                }

                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &iOffset, NULL, &iCount, NULL);
                
                status = H5Dwrite(hDataSet, H5T_NATIVE_INT, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pTempBuf);
                
                
                iNumWritten += iCount;
                pTempBuf += iCount;
            }
            
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    return (status >= 0)?iResult:-1;
}

//----------------------------------------------------------------------------
// restoreDeadSpaces
//
template<typename T>
int  SPopulation<T>::restoreDeadSpaces(hid_t hSpeciesGroup) {
    int iResult = -1;
    printf("restoring contoller\n");

    
    if (qdf_exists(hSpeciesGroup, DEAD_DATASET_NAME)) {
        iResult = 0;

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, DEAD_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        printf("Dataspace extent: %lld\n", dims);
        m_iNumPrevDeaths = dims;
        hsize_t iCount =  TEMP_SIZE;
        hsize_t iOffset = 0;
        if (m_pPrevD != NULL) {
            delete[] m_pPrevD;
        }
        m_pPrevD = new int[dims];
        int *pTempBuf = m_pPrevD;
        uint iNumRead = 0;
        
        
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 

        while (iNumRead < dims) {
             if ((dims - iNumRead) > TEMP_SIZE) {
                 iCount = TEMP_SIZE; 
             } else {
                 iCount = dims - iNumRead;
                 qdf_closeDataSpace(hMemSpace); 
                 hMemSpace = H5Screate_simple (1, &iCount, NULL); 
             }
             status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                          &iOffset, NULL, &iCount, NULL);
             status = H5Dread(hDataSet, H5T_NATIVE_INT, hMemSpace,
                              hDataSpace, H5P_DEFAULT, pTempBuf);
             iNumRead += iCount;
             pTempBuf += iCount;
        }

        if (status >= 0) {
            iResult = 0;
        }
        
    } else {
        printf("WARNING: no dataset [%s] found\n", DEAD_DATASET_NAME);
    }
    return iResult;

}




//----------------------------------------------------------------------------
// getFirstAgentIndex
//
template<typename T>
int SPopulation<T>::getFirstAgentIndex() {

    return m_pAgentController->getFirstIndex(ACTIVE);
}


//----------------------------------------------------------------------------
// getLastAgentIndex
//
template<typename T>
int SPopulation<T>::getLastAgentIndex() {

    return m_pAgentController->getLastIndex(ACTIVE);
}



/**********************************************************************************/
/**********************************************************************************/
/**NG STUFF*DEBUGGING STUFF*DEBUGGIN STUFF*DEBUGGING STUFF*DEBUGGING STUFF*DEBUGG**/
/**********************************************************************************/
/**********************************************************************************/


//----------------------------------------------------------------------------
// agentCheck (debugging)
//
template<typename T>
void SPopulation<T>::agentCheck() {
    printf("Arrays:\n");
    m_pAgentController->displayArray(0, 0, m_pAgentController->getLayerSize());

    printf("States:\n");    
    int iFirstAgent = m_pAgentController->getFirstIndex(ACTIVE);
    if (iFirstAgent != NIL) {
        int iLastAgent  = m_pAgentController->getLastIndex(ACTIVE);
        for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
            printf(" %d:", iAgent);
            int s = m_aAgents[iAgent].m_iLifeState;
            switch (s) {
            case LIFE_STATE_ALIVE:
                printf("L");
                break;
            case LIFE_STATE_DEAD:
                printf("D");
                break;
            case LIFE_STATE_MOVING+LIFE_STATE_ALIVE:
                printf("M");
                break;
            default:
                printf("?");
                break;
            }
        }
        printf("\n");
        
    }
    
}


//----------------------------------------------------------------------------
// showAgents (debugging only)
//   display a list of all agents by calling the pure virtual function showAgent
template<typename T>
void SPopulation<T>::showAgents() {
    printf("-> %d agents\n", m_pAgentController->getNumUsed());
    int iCur = m_pAgentController->getFirstIndex(ACTIVE);
    while (iCur != NIL) {
        printf("+ ");
        showAgent(iCur);
        printf("\n");
        iCur = m_pAgentController->getNextIndex(ACTIVE, iCur);
    }
 
}


//----------------------------------------------------------------------------
// showAgent
//
template<typename T>
void SPopulation<T>::showAgent(int iAgentIndex) {
    Agent &a = m_aAgents[iAgentIndex];

    printf(" [%d] ID [%ld] LS [%d] Loc [%d] ", iAgentIndex, a.m_ulID, a.m_iLifeState, a.m_ulCellID);
}


//----------------------------------------------------------------------------
// showControllerState
//
template<typename T>
void SPopulation<T>::showControllerState(const char *pCaption) {
    printf("%s [%s]\n", pCaption, m_sSpeciesName);
    printf("  layersize: %u\n", m_pAgentController->getLayerSize());
    printf("  numlayers: %u\n", m_pAgentController->getNumLayers());
    printf("  numused:   %u\n", m_pAgentController->getNumUsed());
    uint iSum1 = 0;
    for (uint i = 0; i <  m_pAgentController->getNumLayers(); i++) {
        iSum1 += m_pAgentController->getNumUsed(i);
    }
    printf("  numused L: %d\n", iSum1);
    if (iSum1 != m_pAgentController->getNumUsed()) printf("************** numused and layercount differ\n");
}

 
#endif
