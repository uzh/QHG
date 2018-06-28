#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////
#include "ArrayShare.h"

#include "IndexCollector.cpp"
#include "WeightedMoveRand.cpp"
#include "MassManager.cpp"
#include "SingleEvaluator.cpp"
#include "ShareEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "Birther.cpp"
#include "PDHunting.cpp"
#include "PDAltPredPop.h"

bool g_bUpdateNeededPDAlt = true;

//----------------------------------------------------------------------------
// constructor
//
PDAltPredPop::PDAltPredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<PDAltPredAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_pIC(NULL),
      m_pWM(NULL),
      m_pME(NULL),
      m_pMM(NULL),
      m_pBB(NULL),
      m_pPDHU(NULL) {
    

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));

    double *pAlt = NULL;
    if (m_pCG->m_pGeography != NULL) {
        pAlt = m_pCG->m_pGeography->m_adAltitude;
    }

    evaluatorinfos mEvalInfo;
     
    // add altitude evaluation
    ShareEvaluator<PDAltPredAgent> *pNSH0 = new ShareEvaluator<PDAltPredAgent>(this, pCG, NULL, "Prey0Mass", true, EVENT_ID_NONE);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("ShareWeight0", pNSH0));

    // add altitude evaluation
    ShareEvaluator<PDAltPredAgent> *pNSH1 = new ShareEvaluator<PDAltPredAgent>(this, pCG, NULL, "Prey1Mass", true, EVENT_ID_NONE);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("ShareWeight1", pNSH1));

    // add altitude evaluation
    SingleEvaluator<PDAltPredAgent> *pNSEGeo = new SingleEvaluator<PDAltPredAgent>(this, pCG, NULL, pAlt, "AltPreference", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pNSEGeo));

    m_pME   = new MultiEvaluator<PDAltPredAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators

    m_pIC   = new IndexCollector<PDAltPredAgent>(this, m_pCG, NULL);
    m_pPDHU = new PDHunting<PDAltPredAgent>(this, m_pCG, m_apWELL, m_pPopFinder);
    m_pBB   = new Birther<PDAltPredAgent>(this, m_pCG, m_apWELL);
    m_pMM   = new MassManager<PDAltPredAgent>(this, m_pCG);

    m_pWM   = new WeightedMoveRand<PDAltPredAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    
    // create mass array and fill with '41' (to detect undefined elements)
    m_afMassArray = new double*[m_iNumThreads];
#ifdef OMP_A
#pragma omp parallel 
    {
#else
#endif
        int iT = omp_get_thread_num();
        m_afMassArray[iT] = new double[m_pCG->m_iNumCells];
        memset(m_afMassArray[iT], 41, m_pCG->m_iNumCells*sizeof(double));
    
#ifdef OMP_A
    }
#endif
    // NOTE: m_afMassArray is shared in preLoop() because
    // we don't know the species name here
   

    m_prio.addAction(ATTR_INDEXCOLLECTOR_NAME, m_pIC);
    m_prio.addAction(ATTR_WEIGHTEDMOVERAND_NAME, m_pWM);
    m_prio.addAction(ATTR_MULTIEVAL_NAME, m_pME);
    m_prio.addAction(ATTR_MASSMANAGER_NAME, m_pMM);
    m_prio.addAction(ATTR_BIRTHER_NAME, m_pBB);
    m_prio.addAction(ATTR_PDHUNTING_NAME, m_pPDHU);
    
}


///----------------------------------------------------------------------------
// destructor
//
PDAltPredPop::~PDAltPredPop() {
    if (m_pIC != NULL) {
        delete m_pIC;
    }
    if (m_pBB != NULL) {
        delete m_pBB;
    }
    if (m_pPDHU != NULL) {
        delete m_pPDHU;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pMM != NULL) {
        delete m_pMM;
    }

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }

    if (m_afMassArray != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            delete[] m_afMassArray[i];
        }
        delete[] m_afMassArray;
    }
}


///----------------------------------------------------------------------------
// preLoop
//
int PDAltPredPop::preLoop() {
    int iResult = 0;

    if (m_pCG->m_pGeography == NULL) {
        printf("[PDPredPop::preLoop] No Geography in grid\n");
        iResult = -1;
    }
    
    if (iResult == 0) {
        // the action's preLoop is called in SPopulation::preLoop()
        // so let's do the sharing before calling it
        
        char s[256];
        sprintf(s, ATTR_PD_TEMPLATE_INDEXES, m_sSpeciesName);
        m_pIC->setShareName(s);
        
        // at  this time we know the species name and can share the array as "<speciesname>_Masses"
        char sArrName[64];
        sprintf(sArrName, "%s_Masses", m_sSpeciesName);
        ArrayShare::getInstance()->shareArray(sArrName, m_pCG->m_iNumCells, m_afMassArray[0]);
        printf("[PDPredPop::preLoop] Share m_afMassArray[0] (%p) as [%s]\n", m_afMassArray[0], sArrName);
        
        
        iResult = SPopulation<PDAltPredAgent>::preLoop();
    }
 
    return iResult;
} 


///----------------------------------------------------------------------------
// postLoop
//
int PDAltPredPop::postLoop() {

    ArrayShare::freeInstance();

    return 0;
}

///----------------------------------------------------------------------------
// initializeStep
//
int PDAltPredPop::initializeStep(float fTime) {
    int iResult = SPopulation<PDAltPredAgent>::initializeStep(fTime);

    // later: update number of local agents array for PDPred
    char sArrName[64];
    sprintf(sArrName, "%s_Masses", m_sSpeciesName);
    //    printf("[PDPredPop::initializeStep] Updating shared m_afMassArray[0] (%p) [%s]\n", m_afMassArray[0], sArrName);
    // update mass array
#ifdef OMP_A
#pragma omp parallel 
    {
#endif
        memset(m_afMassArray[omp_get_thread_num()], 0, m_pCG->m_iNumCells*sizeof(double));
#ifdef OMP_A
    }
#endif
        // add masses of agents in thread-specific arrays
        int iFirstAgent = getFirstAgentIndex();
        int iLastAgent  = getLastAgentIndex();
        if (iFirstAgent >= 0) {
#ifdef OMP_A
#pragma omp parallel for 
#endif
            for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
                int iT = omp_get_thread_num();
                PDAltPredAgent* pA = &(m_aAgents[iA]); 
                int iC = pA->m_iCellIndex;
                
                m_afMassArray[iT][iC] += pA->m_fMass;
            }
        }
        // accumulate all masses into array 0

#ifdef OMP_A
#pragma omp parallel for 
#endif
        for (uint iC = 0; iC < m_pCG->m_iNumCells; iC++) {
            for (int iT = 1; iT < m_iNumThreads; iT++) {
                m_afMassArray[0][iC] += m_afMassArray[iT][iC];
            }
        }
        //        printf("[PDPredPop::initializeStep] Updated shared m_afMassArray[0] (%p) [%s]\n", m_afMassArray[0], sArrName);
    

    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int PDAltPredPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    m_aAgents[iAgent].m_fMass = m_aAgents[iMother].m_fBabyMass;
    m_aAgents[iAgent].m_iPreyIndex = -1;
    
    return 0;
}

///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int PDAltPredPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fMass);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void PDAltPredPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    PDAltPredAgent agent;
    H5Tinsert(*hAgentDataType, "Mass", qoffsetof(agent, m_fMass), H5T_NATIVE_FLOAT);

}

   

///----------------------------------------------------------------------------
// setMass
//
double PDAltPredPop::setMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass = fMass;
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// addMass
//
double PDAltPredPop::addMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass += fMass; 
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getMass
//
double PDAltPredPop::getMass(int iAgentIndex) {
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getTotalMass
//
double PDAltPredPop::getTotalMass(int iCellIndex) {
    return m_afMassArray[0][iCellIndex];
}

///----------------------------------------------------------------------------
// getTotalMassArray
//
double *PDAltPredPop::getTotalMassArray() {
    return m_afMassArray[0];
}

///----------------------------------------------------------------------------
// setSecondardMass
//
double PDAltPredPop::setSecondaryMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fBabyMass = fMass;
    return 0;
}
