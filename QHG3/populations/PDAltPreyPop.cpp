#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////
#include "ArrayShare.h"

#include "IndexCollector.cpp"
#include "SingleEvaluator.cpp"
#include "WeightedMove.cpp"
#include "MassManager.cpp"
#include "Verhulst.cpp"
#include "PreyDistributor.h"
#include "PDAltPreyPop.h"


//----------------------------------------------------------------------------
// constructor
//
PDAltPreyPop::PDAltPreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<PDAltPreyAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_afMassArray(NULL),
      m_pWM(NULL),
      m_pMM(NULL),
      m_pVer(NULL) {
    

    int iNumCells = m_pCG->m_iNumCells;
    m_adEnvWeights = new double[iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, iNumCells * (m_pCG->m_iConnectivity + 1) *sizeof(double));

    double *pAlt = NULL;
    if (m_pCG->m_pGeography != NULL) {
        pAlt = m_pCG->m_pGeography->m_adAltitude;
    }

    char* parname = new char[64];
    strcpy(parname,"AltPreference");

    m_pWM = new WeightedMove<PDAltPreyAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);
    m_pSE = new SingleEvaluator<PDAltPreyAgent>(this, m_pCG, m_adEnvWeights, pAlt, parname, true, EVENT_ID_GEO);

    m_pIC = new IndexCollector<PDAltPreyAgent>(this, m_pCG, NULL);
    m_pVer = new Verhulst<PDAltPreyAgent>(this, m_pCG, m_apWELL, -1);
    m_pMM  = new MassManager<PDAltPreyAgent>(this, m_pCG);

    m_afMassArray = new double*[m_iNumThreads];
#ifdef OMP_A
#pragma omp parallel 
    {
#else
#endif
        int iT = omp_get_thread_num();
        m_afMassArray[iT] = new double[iNumCells];
        memset(m_afMassArray[iT], 41, iNumCells*sizeof(double));
#ifdef OMP_A
    }
#endif



    m_prio.addAction(ATTR_INDEXCOLLECTOR_NAME, m_pIC);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVer);
    m_prio.addAction(ATTR_SINGLEEVAL_NAME, m_pSE);
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(ATTR_MASSMANAGER_NAME, m_pMM);
  
}


///----------------------------------------------------------------------------
// destructor
//
PDAltPreyPop::~PDAltPreyPop() {
    if (m_pIC != NULL) {
        delete m_pIC;
    }
    if (m_pVer != NULL) {
        delete m_pVer;
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
    if (m_afMassArray != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            delete[] m_afMassArray[i];
        }
        delete[] m_afMassArray;
    }
    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
}



///----------------------------------------------------------------------------
// preLoop
//
int PDAltPreyPop::preLoop() {
    int iResult = 0;

    if (m_pCG->m_pGeography == NULL) {
        printf("[PDPredPop::preLoop] No Geography in grid\n");
        iResult = -1;
    }
    
    if (iResult == 0) {
        // since the action's preLoop is called in SPopulation::preLoop()
        // we do the sharing beforehand
        char s1[256];
        sprintf(s1, ATTR_PD_TEMPLATE_INDEXES, m_sSpeciesName);
        m_pIC->setShareName(s1);
        
        char s2[256];
        sprintf(s2, "%s_Masses", m_sSpeciesName);
        ArrayShare::getInstance()->shareArray(s2, m_pCG->m_iNumCells, m_afMassArray);
        printf("[PDPreyPop] Share m_afMassArray[0] (%p) as [%s]\n", m_afMassArray[0],s2);


        iResult = SPopulation<PDAltPreyAgent>::preLoop();
    }
    return iResult;
} 


///----------------------------------------------------------------------------
// postLoop
//
int PDAltPreyPop::postLoop() {
    ArrayShare::freeInstance();
    return 0;
}


///----------------------------------------------------------------------------
// initializeStep
//
int PDAltPreyPop::initializeStep(float fT) {
    int iResult = 0;

    // now we can do the other actions
    iResult = SPopulation<PDAltPreyAgent>::initializeStep(fT);
    
    if (iResult == 0) {

        // clear the mass array
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
                PDAltPreyAgent* pA = &(m_aAgents[iA]); 
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

    }
    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int PDAltPreyPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {

    int iThread = omp_get_thread_num();

    float fM = this->m_apWELL[iThread]->wrandr(0.8,1.25)*m_pMM->getMinMass();
    
    m_aAgents[iAgent].m_fMass = fM;
    m_aAgents[iMother].m_fMass -= fM;
    
    
    return 0;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int PDAltPreyPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fMass);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void PDAltPreyPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    PDAltPreyAgent agent;
    H5Tinsert(*hAgentDataType, "Mass", qoffsetof(agent, m_fMass), H5T_NATIVE_FLOAT);

}


///----------------------------------------------------------------------------
// setMass
//
double PDAltPreyPop::setMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass = fMass;
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// addMass
//
double PDAltPreyPop::addMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass += fMass; 
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getMass
//
double PDAltPreyPop::getMass(int iAgentIndex) {
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getTotalMass
//
double PDAltPreyPop::getTotalMass(int iCellIndex) {
    return m_afMassArray[0][iCellIndex];
}

///----------------------------------------------------------------------------
// getTotalMassArray
//
double *PDAltPreyPop::getTotalMassArray() {
    return m_afMassArray[0];
}

///----------------------------------------------------------------------------
// asetSecondaryMass
//
double PDAltPreyPop::setSecondaryMass(int iAgentIndex, double fMass) {
    return dNaN;
}
