#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "DirMove.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "DirTestPop.h"


//----------------------------------------------------------------------------
// constructor
//
DirTestPop::DirTestPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<DirTestAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {
  
    m_pDM = new DirMove<DirTestAgent>(this, m_pCG);

    DirTestAgent aama;
    m_pVer = new Verhulst<DirTestAgent>(this, m_pCG, m_apWELL, (int)qoffsetof(aama, m_iMateIndex));
    m_pPair = new RandomPair<DirTestAgent>(this, m_pCG, m_apWELL);
    m_pGO = new GetOld<DirTestAgent>(this, m_pCG);


    m_prio.addAction(ATTR_DIRMOVE_NAME, m_pDM);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVer);
    m_prio.addAction(ATTR_RANDPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GETOLD_NAME, m_pGO);

}

///----------------------------------------------------------------------------
// destructor
//
DirTestPop::~DirTestPop() {

    if (m_pDM != NULL) {
        delete m_pDM;
    }
    if (m_pVer != NULL) {
        delete m_pVer;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pGO != NULL) {
        delete m_pGO;
    }
   
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int DirTestPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we record parent info into the ancestor box

    DirTestAgent* pBaby = &m_aAgents[iAgent];
    DirTestAgent* pMother = &m_aAgents[iMother];
        
    pBaby->m_fAge = 0.0;
    pBaby->m_iMateIndex = -3;
    pBaby->m_fDirection = pMother->m_fDirection;
    if (pMother->m_iLifeState == LIFE_STATE_ALIVE) {
        pBaby->m_fError = pMother->m_fOldError;
        pBaby->m_fOldError = pMother->m_fOldError;
    } else {
        pBaby->m_fError = pMother->m_fError;
        pBaby->m_fOldError = pMother->m_fError;
    }
    //    printf("Mom %7d (C:%7d, d % 1.3f, e % 1.3f LS %d) -> Baby %7d (C:%7d, d % 1.3f, e % 1.3f)\n", pMother->m_ulID, pMother->m_ulCellID, pMother->m_fDirection, pMother->m_fOldError, pMother->m_iLifeState, pBaby->m_ulID, pBaby->m_ulCellID, pBaby->m_fDirection, pBaby->m_fError);
    return 0;
}



///----------------------------------------------------------------------------
// preLoop
//  use this to set initial values for dDirection and dError
//
int DirTestPop::preLoop() {
    int iResult = 0;

    ;   if (m_pCG->m_pGeography->m_adAngles != NULL) {

        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != NIL) {
            int iLastAgent = getLastAgentIndex();
#ifdef OMP_A
#pragma omp parallel for
#endif
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                m_aAgents[iAgent].m_fDirection = M_PI/127;
                m_aAgents[iAgent].m_fError     = 0;
                m_aAgents[iAgent].m_fOldError  = 0;
            }
        }
    } else {
        printf("This Population needs geo angles. call with '--calc-geoangles'\n");
        iResult = -1;
    }
    return iResult;
}
