#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "AltMoverPop.h"



//----------------------------------------------------------------------------
// constructor
//
AltMoverPop::AltMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<AltMoverAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {
  
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    char* parname = new char[64];
    strcpy(parname,"AltPreference");

    m_pWM = new WeightedMove<AltMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    m_pAE = new SingleEvaluator<AltMoverAgent>(this, m_pCG, m_adEnvWeights, (double*)m_pCG->m_pGeography->m_adAltitude, parname);

    // TO DO: actually we can just make RANDOMMOVE_NAME a member of RandomMove
    // and let m_prio fetch the name of the action from the action itself...
    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(ATTR_SINGLEEVAL_NAME, m_pAE);
    
}

///----------------------------------------------------------------------------
// destructor
//
AltMoverPop::~AltMoverPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pAE != NULL) {
        delete m_pAE;
    }
}


