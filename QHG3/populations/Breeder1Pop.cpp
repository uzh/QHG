#include <omp.h>
#include <hdf5.h>

#include "GeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "ConfinedMove.cpp"
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
#include "Breeder1Pop.h"


//----------------------------------------------------------------------------
// constructor
//
Breeder1Pop::Breeder1Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<Breeder1Agent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<Breeder1Agent>(this, pCG, m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    evaluatorinfos mEvalInfo;
    
    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<Breeder1Agent> *pSEAlt = new SingleEvaluator<Breeder1Agent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<Breeder1Agent> *pSECap = new SingleEvaluator<Breeder1Agent>(this, pCG, NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("NPPWeight", pSECap));

    m_pME = new MultiEvaluator<Breeder1Agent>(this, m_pCG, m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators

    Breeder1Agent agent;
    m_pVerhulst = new Verhulst<Breeder1Agent>(this, m_pCG, m_apWELL, (int)qoffsetof(agent, m_iMateIndex));


    // evaluator for movement

    m_pWM = new WeightedMove<Breeder1Agent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<Breeder1Agent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<Breeder1Agent>(this, m_pCG);

    m_pOAD = new OldAgeDeath<Breeder1Agent>(this, m_pCG, m_apWELL);

    m_pFert = new Fertility<Breeder1Agent>(this, m_pCG);

    m_pGenetics = new Genetics<Breeder1Agent,GeneUtils>(this, m_pCG, m_pAgentController, &m_vMergedDeadList, m_apWELL);

    m_pCM = new ConfinedMove<Breeder1Agent>(this, m_pCG, this->m_vMoveList);

    // adding all actions to prioritizer

    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(ATTR_CONFINEDMOVE_NAME, m_pCM);
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
Breeder1Pop::~Breeder1Pop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pCM != NULL) {
        delete m_pCM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pVerhulst != NULL) {
        delete m_pVerhulst;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
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
int Breeder1Pop::setParams(const char *pParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int Breeder1Pop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<Breeder1Agent>::preLoop();
   
    if (m_bCreateGenomes) {
        // we assume the agents start at position 0
        //        int iNumMutations = 0;
        int iN = getNumAgentsTotal();
        
        iResult = m_pGenetics->createInitialGenomes(iN);
    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }

    /*
    //DEBUG
    if (this->m_pCG->m_pVegetation == NULL) {
        this->m_pCG->m_pVegetation = new Vegetation(this->m_pCG->m_iNumCells, 2,  this->m_pCG->m_pGeography,  this->m_pCG->m_pClimate);
    }
    int iStep = 1;
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        this->m_pCG->m_pVegetation->m_adANPP[0][i] = m_adCapacities[i*iStep];
    }
    // to be sure it has not been overwritten
    for (uint i = 0; i < 5; i++) {
        this->m_pCG->m_pVegetation->m_adANPP[0][i] = 23;
    }
    // DEBUG end
    */

    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int Breeder1Pop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int Breeder1Pop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int Breeder1Pop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void Breeder1Pop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    Breeder1Agent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int Breeder1Pop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

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
