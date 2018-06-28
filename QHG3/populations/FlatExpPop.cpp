#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "BitGeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
//#include "ConfinedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "FlatExpPop.h"


//----------------------------------------------------------------------------
// constructor
//
FlatExpPop::FlatExpPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<FlatExpAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<FlatExpAgent>(this, pCG, m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    evaluatorinfos mEvalInfo;

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<FlatExpAgent> *pSEAlt = new SingleEvaluator<FlatExpAgent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<FlatExpAgent> *pSECap = new SingleEvaluator<FlatExpAgent>(this, pCG, NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("NPPWeight", pSECap));

    m_pME = new MultiEvaluator<FlatExpAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators

    FlatExpAgent agent;
    m_pVerhulst = new Verhulst<FlatExpAgent>(this, m_pCG, m_apWELL, (int)qoffsetof(agent, m_iMateIndex));


    // evaluator for movement

    m_pWM = new WeightedMove<FlatExpAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<FlatExpAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<FlatExpAgent>(this, m_pCG);

    m_pOAD = new OldAgeDeath<FlatExpAgent>(this, m_pCG, m_apWELL);

    m_pFert = new Fertility<FlatExpAgent>(this, m_pCG);

    m_pGenetics = new Genetics<FlatExpAgent,BitGeneUtils>(this, m_pCG, m_pAgentController, &m_vMergedDeadList, m_apWELL);

    //    m_pCM = new ConfinedMove<FlatExpAgent>(this, m_pCG, this->m_vMoveList);

    // adding all actions to prioritizer

    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    //    m_prio.addAction(ATTR_CONFINEDMOVE_NAME, m_pCM);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVerhulst);
    m_prio.addAction(ATTR_RANDPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GETOLD_NAME, m_pGO);
    m_prio.addAction(ATTR_OLDAGEDEATH_NAME, m_pOAD);
    m_prio.addAction(ATTR_FERTILITY_NAME, m_pFert);
    m_prio.addAction(ATTR_NPPCAPACITY_NAME, m_pNPPCap);
    m_prio.addAction(ATTR_GENETICS_NAME, m_pGenetics);

}



///----------------------------------------------------------------------------
// destructor
//
FlatExpPop::~FlatExpPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pVerhulst != NULL) {
        delete m_pVerhulst;
    }
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pOAD != NULL) {
        delete m_pOAD;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pNPPCap != NULL) {
        delete m_pNPPCap;
    }
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int FlatExpPop::setParams(const char *pParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int FlatExpPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<FlatExpAgent>::preLoop();
   
    /*
    if (m_bCreateGenomes) {
        int iN = getNumAgentsEffective();
        
        iResult = m_pGenetics->createInitialGenomes(iN);
    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }
    */
    int iN = getNumAgentsEffective();
    iResult = m_pGenetics->createInitialGenomes(iN);

    for (int i = 0; i < m_iNumThreads; ++i) {
        printf("[preLoop3]T %d:  Numrands so far: %lu; state: ", i, m_apWELL[i]->getCount());
        const uint32_t *pState = m_apWELL[i]->getState();
        for (uint j = 0; j < STATE_SIZE; ++j) {
            printf(" %08x", pState[j]);
	}
        printf("\n");
    }

    return iResult;
}


///----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : update NPP
//
int FlatExpPop::updateEvent(int iEventID, char *pData, float fT) { 
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != NIL) {
            int iLastAgent = getLastAgentIndex();
#ifdef OMP_A
            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
#pragma omp parallel for schedule(static, iChunk) 
#endif
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((this->m_pCG->m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (this->m_pCG->m_pGeography->m_abIce[iCellIndex] > 0)) {
                        registerDeath(iCellIndex, iAgent);
                    }
                }
            }
        }
        // make sure they are removed before step starts
        if (m_bRecycleDeadSpace) {
            recycleDeadSpaceNew();
        } else {
            performDeaths();
        }
        // update counts
        updateTotal();
        updateNumAgentsPerCell();
        // clear lists to avoid double deletion
	initListIdx();
    }
      
    notifyObservers(iEventID, pData);
    
    return 0;
};



///----------------------------------------------------------------------------
// flushEvents
//
void FlatExpPop::flushEvents(float fT) {
    notifyObservers(EVENT_ID_FLUSH, NULL);
}
///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int FlatExpPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    m_pGenetics->initialize(m_fCurTime);
    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    // child & death stistics
    m_aAgents[iAgent].m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int FlatExpPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
      
    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int FlatExpPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void FlatExpPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    FlatExpAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int FlatExpPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);

    /*
    if (iResult == 0) {
        // we read the genome from a QDF: no need to create it
        m_bCreateGenomes = false;
    } else {
        m_bCreateGenomes = true;
        iResult = 0;
    }
    */
    return iResult;
}
