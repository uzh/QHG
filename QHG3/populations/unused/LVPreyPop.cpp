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
#include "LVPreyPop.h"


//----------------------------------------------------------------------------
// constructor
//
LVPreyPop::LVPreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<LVPreyAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {


    m_pRM = new RandomMove<LVPreyAgent>(this, m_pCG, m_apWELL);


    m_pLV = new LotkaVolterra<LVPreyAgent>(this, m_pCG, m_apWELL, pPopFinder);



    // adding all actions to prioritizer

    m_prio.addAction(ATTR_RANDOMMOVE_NAME, m_pRM);
    m_prio.addAction(ATTR_LOTKAVOLTERRA_NAME, m_pLV);

}


//----------------------------------------------------------------------------
// destructor
//
LVPreyPop::~LVPreyPop() {

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
int LVPreyPop::preLoop() {


    int iResult = 0;

    iResult = m_pLV->preLoop();

    return iResult;
} 
