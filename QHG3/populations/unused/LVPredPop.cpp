#include <string.h>
#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "RandomMove.cpp"
#include "LotkaVolterra.cpp"
#include "LVPredPop.h"


//----------------------------------------------------------------------------
// constructor
//
LVPredPop::LVPredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<LVPredAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {


    m_pRM = new RandomMove<LVPredAgent>(this, m_pCG, m_apWELL);


    m_pLV = new LotkaVolterra<LVPredAgent>(this, m_pCG, m_apWELL, pPopFinder);



    // adding all actions to prioritizer

    m_prio.addAction(ATTR_RANDOMMOVE_NAME, m_pRM);
    m_prio.addAction(ATTR_LOTKAVOLTERRA_NAME, m_pLV);

}


//----------------------------------------------------------------------------
// destructor
//
LVPredPop::~LVPredPop() {

    if (m_pRM != NULL) {
        delete m_pRM;
    }
    if (m_pLV != NULL) {
        delete m_pLV;
    }
}

///----------------------------------------------------------------------------
// preLoop
//
int LVPredPop::preLoop() {


    int iResult = 0;

    iResult = m_pLV->preLoop();

    return iResult;
} 
