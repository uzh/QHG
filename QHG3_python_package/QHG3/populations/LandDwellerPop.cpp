#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "LandDwellerPop.h"


//----------------------------------------------------------------------------
// constructor
//
LandDwellerPop::LandDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<LandDwellerAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    m_adMoveWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity

    /* old
    std::map<char*, double*> mEvalInfo;
    
    char *altprefname = new char[64];
    strcpy(altprefname, "AltCapPref,AltWeight");
    mEvalInfo.insert(std::pair<char*, double*>(altprefname, (double*)m_pCG->m_pGeography->m_adAltitude));
    
    char *tempprefname = new char[64];
    strcpy(tempprefname, "TempCapPref,TempWeight");
    mEvalInfo.insert(std::pair<char*, double*>(tempprefname, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp));
    */    
    evaluatorinfos mEvalInfo;
    
    // add altitude evaluation
    SingleEvaluator<LandDwellerAgent> *pSEAlt = new SingleEvaluator<LandDwellerAgent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    // add altitude evaluation
    SingleEvaluator<LandDwellerAgent> *pSETemp = new SingleEvaluator<LandDwellerAgent>(this, pCG, NULL, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp, "TempCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("TempWeight", pSETemp));



    m_pME = new MultiEvaluator<LandDwellerAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, false, &(m_pCG->m_pGeography->m_bUpdated));

    /*old 
    delete tempprefname;
    delete altprefname;
    */
    LandDwellerAgent agent;
    m_pVerVarK = new VerhulstVarK<LandDwellerAgent>(this, m_pCG, m_apWELL, m_adEnvWeights, (int)qoffsetof(agent, m_iMateIndex));


    // evaluator for movement

    char *moveprefname = new char[64];
    strcpy(moveprefname, "AltMovePref");
    m_pSE = new SingleEvaluator<LandDwellerAgent>(this, m_pCG, m_adMoveWeights, (double*)m_pCG->m_pGeography->m_adAltitude,
                                                  moveprefname, true, &(m_pCG->m_pGeography->m_bUpdated));
    delete moveprefname;
    
    m_pWM = new WeightedMove<LandDwellerAgent>(this, m_pCG, m_apWELL, m_adMoveWeights);


    m_pPair = new RandomPair<LandDwellerAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<LandDwellerAgent>(this, m_pCG);


    // adding all actions to prioritizer

    m_prio.addAction(MULTIEVAL_NAME, m_pME);
    m_prio.addAction(SINGLEEVAL_NAME, m_pSE);
    m_prio.addAction(WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(VERHULSTVARK_NAME, m_pVerVarK);
    m_prio.addAction(RANDPAIR_NAME, m_pPair);
    m_prio.addAction(GETOLD_NAME, m_pGO);

}

///----------------------------------------------------------------------------
// destructor
//
LandDwellerPop::~LandDwellerPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pSE != NULL) {
        delete m_pSE;
    }
    if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
}



///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int LandDwellerPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    m_aAgents[iAgent].m_fAge = 0.0;

    return 0;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int LandDwellerPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void LandDwellerPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    LandDwellerAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);

}

