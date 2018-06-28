#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"
#include "RandomMove1D.cpp"
#include "Verhulst.cpp"
#include "GetOld.cpp"

#include "FK1DPop.h"

//----------------------------------------------------------------------------
// constructor
//
FK1DPop::FK1DPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<FKAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {
    
    pRM = new RandomMove1D<FKAgent>(this, m_pCG, m_apWELL);
    pVer = new Verhulst<FKAgent>(this, m_pCG, m_apWELL); // without iMateOffset, this is asexual
    pGO = new GetOld<FKAgent>(this, m_pCG);

    m_prio.addAction(ATTR_RANDOMMOVE1D_NAME,  pRM);
    m_prio.addAction(ATTR_VERHULST_NAME, pVer);
    m_prio.addAction(ATTR_GETOLD_NAME, pGO);
   
}

///----------------------------------------------------------------------------
// destructor
//
FK1DPop::~FK1DPop() {
    delete pRM;
    delete pVer;
    delete pGO;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int FK1DPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void FK1DPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    FKAgent fka;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(fka, m_fAge), H5T_NATIVE_FLOAT);

}

