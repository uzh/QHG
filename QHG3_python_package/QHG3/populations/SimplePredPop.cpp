#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////
#include "ArrayShare.h"

#include "WeightedMoveRand.cpp"
#include "MassManager.cpp"
//#include "SingleEvaluator.cpp"
#include "ShareEvaluator.cpp"
#include "Birther.cpp"
#include "Hunting.cpp"
#include "SimplePredPop.h"

bool g_bUpdateNeeded = true;

//----------------------------------------------------------------------------
// constructor
//
SimplePredPop::SimplePredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<SimplePredAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_pWM(NULL),
      m_pSE(NULL),
      m_pMM(NULL),
      m_pBB(NULL),
      m_pHU(NULL) {
    

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));

    m_pHU = new Hunting<SimplePredAgent>(this, m_pCG, m_apWELL, m_pPopFinder);
    m_pBB = new Birther<SimplePredAgent>(this, m_pCG, m_apWELL);
    m_pMM  = new MassManager<SimplePredAgent>(this, m_pCG);

    m_pWM = new WeightedMoveRand<SimplePredAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    m_pSE = new ShareEvaluator<SimplePredAgent>(this, m_pCG, m_adEnvWeights, "PreyMass",  true, &g_bUpdateNeeded);
    
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
   

    m_prio.addAction(WEIGHTEDMOVERAND_NAME, m_pWM);
    m_prio.addAction(SHAREEVAL_NAME, m_pSE);
    m_prio.addAction(MASSMANAGER_NAME, m_pMM);
    m_prio.addAction(BIRTHER_NAME, m_pBB);
    m_prio.addAction(HUNTING_NAME, m_pHU);
    
}


///----------------------------------------------------------------------------
// destructor
//
SimplePredPop::~SimplePredPop() {
    if (m_pBB != NULL) {
        delete m_pBB;
    }
    if (m_pHU != NULL) {
        delete m_pHU;
    }
    if (m_pSE != NULL) {
        delete m_pSE;
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
int SimplePredPop::preLoop() {
    int iResult = 0;
    // share before calling action's preLooop (in SPopulation::preLoop)

    // now we know the species name and can share the array as "<speciesname>_Masses"
    char sArrName[64];
    sprintf(sArrName, "%s_Masses", m_sSpeciesName);
    ArrayShare::getInstance()->shareArray(sArrName, m_pCG->m_iNumCells, m_afMassArray[0]);
    printf("[SimplePredPop::preLoop] Share m_afMassArray[0] (%p) as [%s]\n", m_afMassArray[0], sArrName);


    iResult = SPopulation<SimplePredAgent>::preLoop();

    return iResult;
} 


///----------------------------------------------------------------------------
// postLoop
//
int SimplePredPop::postLoop() {
    ArrayShare::freeInstance();
    return 0;
}


///----------------------------------------------------------------------------
// initializeStep
//
int SimplePredPop::initializeStep(float fTime) {
    int iResult = SPopulation<SimplePredAgent>::initializeStep(fTime);

    // later: update number of local agents array for SimplePred
    char sArrName[64];
    sprintf(sArrName, "%s_Masses", m_sSpeciesName);
    printf("[SimplePredPop::initializeStep] Updating shared m_afMassArray[0] (%p) [%s]\n", m_afMassArray[0], sArrName);
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
                SimplePredAgent* pA = &(m_aAgents[iA]); 
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
        printf("[SimplePredPop::initializeStep] Updated shared m_afMassArray[0] (%p) [%s]\n", m_afMassArray[0], sArrName);
    

    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int SimplePredPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    m_aAgents[iAgent].m_fMass = m_aAgents[iMother].m_fBabyMass;
    m_aAgents[iAgent].m_iPreyIndex = -1;
    
    return 0;
}

///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int SimplePredPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fMass);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void SimplePredPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    SimplePredAgent agent;
    H5Tinsert(*hAgentDataType, "Mass", qoffsetof(agent, m_fMass), H5T_NATIVE_FLOAT);

}

   

///----------------------------------------------------------------------------
// setMass
//
double SimplePredPop::setMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass = fMass;
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// addMass
//
double SimplePredPop::addMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass += fMass; 
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getMass
//
double SimplePredPop::getMass(int iAgentIndex) {
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getTotalMass
//
double SimplePredPop::getTotalMass(int iCellIndex) {
    return m_afMassArray[0][iCellIndex];
}

///----------------------------------------------------------------------------
// getTotalMassArray
//
double *SimplePredPop::getTotalMassArray() {
    return m_afMassArray[0];
}

///----------------------------------------------------------------------------
// setSecondardMass
//
double SimplePredPop::setSecondaryMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fBabyMass = fMass;
    return 0;
}
