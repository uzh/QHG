#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "GeneUtils.h"
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
#include "VerhulstVarK.cpp"
#include "SelPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "OoASelPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoASelPop::OoASelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoASelAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<OoASelAgent>(this, pCG, m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    evaluatorinfos mEvalInfo;

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoASelAgent> *pSEAlt = new SingleEvaluator<OoASelAgent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoASelAgent> *pSECap = new SingleEvaluator<OoASelAgent>(this, pCG, NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("NPPWeight", pSECap));

    m_pME = new MultiEvaluator<OoASelAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators

    OoASelAgent agent;
    m_pVerVarK = new VerhulstVarK<OoASelAgent>(this, m_pCG, m_apWELL, m_adCapacities, iCapacityStride, (int)qoffsetof(agent, m_iMateIndex));


    // evaluator for movement

    m_pWM = new WeightedMove<OoASelAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    m_pPair = new SelPair<OoASelAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<OoASelAgent>(this, m_pCG);

    m_pOAD = new OldAgeDeath<OoASelAgent>(this, m_pCG, m_apWELL);

    m_pFert = new Fertility<OoASelAgent>(this, m_pCG);

    m_pGenetics = new Genetics<OoASelAgent,GeneUtils>(this, m_pCG, m_pAgentController, &m_vMergedDeadList, m_aiSeeds[1]);

    //    m_pCM = new ConfinedMove<OoASelAgent>(this, m_pCG, this->m_vMoveList);

    // adding all actions to prioritizer

    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    //    m_prio.addAction(ATTR_CONFINEDMOVE_NAME, m_pCM);
    m_prio.addAction(ATTR_VERHULSTVARK_NAME, m_pVerVarK);
    m_prio.addAction(ATTR_SELPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GETOLD_NAME, m_pGO);
    m_prio.addAction(ATTR_OLDAGEDEATH_NAME, m_pOAD);
    m_prio.addAction(ATTR_FERTILITY_NAME, m_pFert);
    m_prio.addAction(ATTR_NPPCAPACITY_NAME, m_pNPPCap);
    m_prio.addAction(ATTR_GENETICS_NAME, m_pGenetics);

}



///----------------------------------------------------------------------------
// destructor
//
OoASelPop::~OoASelPop() {

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
     if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
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
int OoASelPop::setParams(const char *pParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoASelPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoASelAgent>::preLoop();
   
    if (m_bCreateGenomes) {
        // we assume the agents start at position 0
        //        int iNumMutations =  3; //-1: free reco
        int iN = getNumAgentsTotal();
        
        iResult = m_pGenetics->createInitialGenomes(iN);

    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }

    if (iResult == 0) {
        m_pPair->setGenome(m_pGenetics->getGenome(), m_pGenetics->getNumBlocks());
    }
    return iResult;
}


///----------------------------------------------------------------------------
// postLoop
//
int OoASelPop::postLoop() {
    int iResult = SPopulation<OoASelAgent>::postLoop();
    return iResult;
}


///----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : update NPP
//
int OoASelPop::updateEvent(int iEventID, char *pData, float fT) { 
    if (iEventID == EVENT_ID_GEO) {
        // drown
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
                    if (this->m_pCG->m_pGeography->m_adAltitude[iCellIndex] < 0) {
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
void OoASelPop::flushEvents(float fT) {
    notifyObservers(EVENT_ID_FLUSH, NULL);
}

///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoASelPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int OoASelPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoASelPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void OoASelPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoASelAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int OoASelPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);

    if (iResult == 0) {
        // we read the genome from a QDF: no need to create it
        m_bCreateGenomes = false;
    } else {
        m_bCreateGenomes = true;
        iResult = 0;
    }

    return iResult;
}
