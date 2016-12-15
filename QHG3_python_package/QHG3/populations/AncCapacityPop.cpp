#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "RandomMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "AncCapacityPop.h"


//----------------------------------------------------------------------------
// constructor
//
AncCapacityPop::AncCapacityPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<AncCapacityAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
    m_pAncBox(NULL) {
  

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity

    /* old
    std::map<char*, double*> mEvalInfo;
    
    char *altprefname = new char[64];
    strcpy(altprefname, "AltPreference,AltWeight");
    mEvalInfo.insert(std::pair<char*, double*>(altprefname, (double*)m_pCG->m_pGeography->m_adAltitude));
    
    char *tempprefname = new char[64];
    strcpy(tempprefname, "TempPreference,TempWeight");
    mEvalInfo.insert(std::pair<char*, double*>(tempprefname, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp));
    */      

    evaluatorinfos mEvalInfo;

    SingleEvaluator<AncCapacityAgent> *pSEAlt = new SingleEvaluator<AncCapacityAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    SingleEvaluator<AncCapacityAgent> *pSETemp = new SingleEvaluator<AncCapacityAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp, "TempPreference");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("TempWeight", pSETemp));

    m_pME = new MultiEvaluator<AncCapacityAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, false);


    // done MultiEvaluator

    m_pRM = new RandomMove<AncCapacityAgent>(this, m_pCG, m_apWELL);


    AncCapacityAgent agent;
    m_pVerVarK = new VerhulstVarK<AncCapacityAgent>(this, m_pCG, m_apWELL, m_adEnvWeights, (int)qoffsetof(agent, m_iMateIndex));

    m_pPair = new RandomPair<AncCapacityAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<AncCapacityAgent>(this, m_pCG);


    // adding all actions to prioritizer

    m_prio.addAction(MULTIEVAL_NAME, m_pME);
    m_prio.addAction(RANDOMMOVE_NAME, m_pRM);
    m_prio.addAction(VERHULSTVARK_NAME, m_pVerVarK);
    m_prio.addAction(RANDPAIR_NAME, m_pPair);
    m_prio.addAction(GETOLD_NAME, m_pGO);

}

///----------------------------------------------------------------------------
// destructor
//
AncCapacityPop::~AncCapacityPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
    if (m_pAncBox != NULL) {
        delete m_pAncBox;
    }
}


///----------------------------------------------------------------------------
// preLoop
//
int AncCapacityPop::preLoop() {

    // here we add the initial agents    
    // to the ancestor box

    // sprintf(m_sAncOutFileName,"pop_%d.anc",m_iSpeciesID);
    m_pAncBox = AncestorBoxR::createInstance(m_aAgents.getLayerSize());

    int iFirstAgent = getFirstAgentIndex();
    int iLastAgent = getLastAgentIndex();

    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        m_pAncBox->addBaby(m_aAgents[iA].m_ulID, 0, 0);
    }

    return 0;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int AncCapacityPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we record parent info into the ancestor box

    m_pAncBox->addBaby(m_aAgents[iAgent].m_ulID, 
                       m_aAgents[iMother].m_ulID, 
                       m_aAgents[iFather].m_ulID);

    m_aAgents[iAgent].m_fAge = 0.0;

    return 0;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int AncCapacityPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    // here we use a trick:
    // we are not writing to the QDF but to a separate file
    // but it is convenient to do it here because 
    // this function is called just at the right moment 

    char *ancfilename = new char[MAX_NAME+16];
    sprintf(ancfilename, "%s_%06d.anc", m_sSpeciesName, (int)(m_fCurTime+1));
    m_pAncBox->setOutputFile(ancfilename,false);
    delete[] ancfilename;
    return m_pAncBox->writeData();
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int AncCapacityPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void AncCapacityPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    AncCapacityAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);

}

