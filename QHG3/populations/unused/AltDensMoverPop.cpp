
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "AltDensMoverPop.h"



//----------------------------------------------------------------------------
// constructor
//
AltDensMoverPop::AltDensMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<AltDensMoverAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {
  
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    /*old
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

    SingleEvaluator<AltDensMoverAgent> *pSEAlt = new SingleEvaluator<AltDensMoverAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    SingleEvaluator<AltDensMoverAgent> *pSEDens = new SingleEvaluator<AltDensMoverAgent>(this, m_pCG, NULL, m_dNumAgentsPerCell, "DensPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("DensWeight", pSEDens));

    m_pME = new MultiEvaluator<AltDensMoverAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo);
    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);

    m_pWM = new WeightedMove<AltDensMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    
}

///----------------------------------------------------------------------------
// destructor
//
AltDensMoverPop::~AltDensMoverPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
}


