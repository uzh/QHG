
#include "GeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "SimpleCondition.h"
#include "CondWeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "AltDensGenMoverPop.h"



//----------------------------------------------------------------------------
// constructor
//
AltDensGenMoverPop::AltDensGenMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<AltDensGenMoverAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_bCreateGenomes(true) {
  
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    
    /* old
    std::map<char*, double*> mEvalInfo;
    
    // add altitude evaluation
    char* eval1 = new char[128];
    strcpy(eval1,"AltPreference,AltWeight");
    mEvalInfo.insert(std::pair<char*, double*>(eval1, (double*)m_pCG->m_pGeography->m_adAltitude));
  
    // add density evaluation
    char* eval2 = new char[128];
    strcpy(eval2,"DensPreference,DensWeight");
    mEvalInfo.insert(std::pair<char*, double*>(eval2, m_dNumAgentsPerCell));
    */

    evaluatorinfos mEvalInfo;

    SingleEvaluator<AltDensGenMoverAgent> *pSEAlt = new SingleEvaluator<AltDensGenMoverAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    SingleEvaluator<AltDensGenMoverAgent> *pSEDens = new SingleEvaluator<AltDensGenMoverAgent>(this, m_pCG, NULL, m_dNumAgentsPerCell, "DensPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("DensWeight", pSEDens));

    int iMode = MODE_MUL_SIMPLE;
    //    bool bUpdate = true;
    m_pME = new MultiEvaluator<AltDensGenMoverAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, iMode, NULL,  &(m_pCG->m_pGeography->m_bUpdated));
    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    
    m_pMC = new SimpleCondition(m_dNumAgentsPerCell, ALLOW_ALWAYS);
    m_pCWM = new CondWeightedMove<AltDensGenMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights, m_pMC);
    m_prio.addAction(ATTR_CONDWEIGHTEDMOVE_NAME, m_pCWM);
    /*
    m_pWM = new WeightedMove<AltDensGenMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);
    m_prio.addAction(WEIGHTEDMOVE_NAME, m_pWM);
    */
    m_pGenetics = new Genetics<AltDensGenMoverAgent,GeneUtils>(this, m_pCG, m_pAgentController, &m_vMergedDeadList, m_apWELL);

    AltDensGenMoverAgent aama;
    m_pVer = new Verhulst<AltDensGenMoverAgent>(this, m_pCG, m_apWELL, (int)qoffsetof(aama, m_iMateIndex));

    m_pPair = new RandomPair<AltDensGenMoverAgent>(this, m_pCG, m_apWELL);

    
    m_prio.addAction(ATTR_CONDWEIGHTEDMOVE_NAME, m_pCWM);
    /*
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    */
    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVer);
    m_prio.addAction(ATTR_RANDPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GENETICS_NAME, m_pGenetics);
    
  
}

///----------------------------------------------------------------------------
// destructor
//
AltDensGenMoverPop::~AltDensGenMoverPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    
    if (m_pMC != NULL) {
        delete m_pMC;
    }
    if (m_pCWM != NULL) {
        delete m_pCWM;
    }
    /*
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    */
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pVer != NULL) {
        delete m_pVer;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }
}


///----------------------------------------------------------------------------
// setParams
//
int AltDensGenMoverPop::setParams(const char *pParams) {
    int iResult = 0;
  
    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int AltDensGenMoverPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = 0;
   
    if (m_bCreateGenomes) {
        // we assume the agents start at position 0
        
        int iN = getNumAgentsTotal();
        
        iResult = m_pGenetics->createInitialGenomes(iN);
    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int AltDensGenMoverPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we call the Genetics method to create a genome for the baby

    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int AltDensGenMoverPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    printf("writewriteqwrite start\n");fflush(stdout);
    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);
}



///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int AltDensGenMoverPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    // we read the genome from a QDF: no need to create it
    m_bCreateGenomes = false;
    return m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);
}


