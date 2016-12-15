#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////
#include "ArrayShare.h"

#include "RandomMove.cpp"
#include "MassManager.cpp"
#include "Verhulst.cpp"
#include "SimplePreyPop.h"



//----------------------------------------------------------------------------
// constructor
//
SimplePreyPop::SimplePreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<SimplePreyAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_afMassArray(NULL),
      m_pRM(NULL),
      m_pMM(NULL),
      m_pVer(NULL) {
    
    m_pVer = new Verhulst<SimplePreyAgent>(this, m_pCG, m_apWELL, -1);
    m_pRM  = new RandomMove<SimplePreyAgent>(this, m_pCG, m_apWELL);
    m_pMM  = new MassManager<SimplePreyAgent>(this, m_pCG);

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

    ArrayShare::getInstance()->shareArray("PreyMasses", m_pCG->m_iNumCells, m_afMassArray[0]);
    printf("[SimplePreyPop] Share m_afMassArray[0] (%p) as [PreyMasses]\n", m_afMassArray[0]);
    m_prio.addAction(VERHULST_NAME, m_pVer);
    m_prio.addAction(RANDOMMOVE_NAME, m_pRM);
    m_prio.addAction(MASSMANAGER_NAME, m_pMM);
  
}


///----------------------------------------------------------------------------
// destructor
//
SimplePreyPop::~SimplePreyPop() {
    if (m_pVer != NULL) {
        delete m_pVer;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
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
}


///----------------------------------------------------------------------------
// preLoop
//
int SimplePreyPop::preLoop() {
    int iResult = SPopulation<SimplePreyAgent>::preLoop();

    return iResult;
} 


///----------------------------------------------------------------------------
// postLoop
//
int SimplePreyPop::postLoop() {
    ArrayShare::freeInstance();
    return 0;
}


///----------------------------------------------------------------------------
// initializeStep
//
int SimplePreyPop::initializeStep(float fT) {
    int iResult = SPopulation<SimplePreyAgent>::initializeStep(fT);
    if (iResult == 0) {

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
                SimplePreyAgent* pA = &(m_aAgents[iA]); 
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
int SimplePreyPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {

    int iThread = omp_get_thread_num();

    float fM = this->m_apWELL[iThread]->wrandr(0.8,1.25)*m_pMM->getMinMass();
    
    m_aAgents[iAgent].m_fMass = fM;
    m_aAgents[iMother].m_fMass -= fM;
    
    
    return 0;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int SimplePreyPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fMass);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void SimplePreyPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    SimplePreyAgent agent;
    H5Tinsert(*hAgentDataType, "Mass", qoffsetof(agent, m_fMass), H5T_NATIVE_FLOAT);

}


///----------------------------------------------------------------------------
// setMass
//
double SimplePreyPop::setMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass = fMass;
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// addMass
//
double SimplePreyPop::addMass(int iAgentIndex, double fMass) {
    m_aAgents[iAgentIndex].m_fMass += fMass; 
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getMass
//
double SimplePreyPop::getMass(int iAgentIndex) {
    return m_aAgents[iAgentIndex].m_fMass;
}


///----------------------------------------------------------------------------
// getTotalMass
//
double SimplePreyPop::getTotalMass(int iCellIndex) {
    return m_afMassArray[0][iCellIndex];
}

///----------------------------------------------------------------------------
// getTotalMassArray
//
double *SimplePreyPop::getTotalMassArray() {
    return m_afMassArray[0];
}

///----------------------------------------------------------------------------
// asetSecondaryMass
//
double SimplePreyPop::setSecondaryMass(int iAgentIndex, double fMass) {
    return dNaN;
}
