#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "Verhulst.cpp"

#include "AltMoverFKPop.h"

//------------------------------------------------------------------------------
// constructor
//
AltMoverFKPop::AltMoverFKPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
	: SPopulation<AltMoverFKAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState){
	
	m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

	char* parname = new char[64];
	strcpy(parname,"AltMovePref");
    
    m_pSE = new SingleEvaluator<AltMoverFKAgent>(this, m_pCG, m_adEnvWeights, (double*)m_pCG->m_pGeography->m_adAltitude, parname, true, &(m_pCG->m_pGeography->m_bUpdated));
    m_pWM = new WeightedMove<AltMoverFKAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);
    m_pVer = new Verhulst<AltMoverFKAgent>(this, m_pCG, m_apWELL); // without iMateOffset, this is asexual

    // set priorities
    m_prio.addAction(SINGLEEVAL_NAME, m_pSE);
    m_prio.addAction(WEIGHTEDMOVE_NAME, m_pWM);	  
    m_prio.addAction(VERHULST_NAME, m_pVer);  
}

//-------------------------------------------------------------------------------
// destructor
//
AltMoverFKPop::~AltMoverFKPop() {
	
	if (m_adEnvWeights != NULL) {
    	delete[] m_adEnvWeights;
    }
    if (m_pSE != NULL) {
		delete m_pSE;
    } 
    if (m_pWM != NULL) {
 		delete m_pWM;
    }
    if (m_pVer != NULL) {
    	delete m_pVer;
    }
}





