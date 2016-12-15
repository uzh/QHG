#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"
#include "RandomMove.cpp"
#include "Verhulst.cpp"
#include "GetOld.cpp"

#include "FKPop.h"

//----------------------------------------------------------------------------
// constructor
//
FKPop::FKPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<FKAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {
    
    pRM = new RandomMove<FKAgent>(this, m_pCG, m_apWELL);
    pVer = new Verhulst<FKAgent>(this, m_pCG, m_apWELL); // without iMateOffset, this is asexual
    pGO = new GetOld<FKAgent>(this, m_pCG);

    m_prio.addAction(RANDOMMOVE_NAME,  pRM);
    m_prio.addAction(VERHULST_NAME, pVer);
    m_prio.addAction(GETOLD_NAME, pGO);
   
}

///----------------------------------------------------------------------------
// destructor
//
FKPop::~FKPop() {
    delete pRM;
    delete pVer;
    delete pGO;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int FKPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void FKPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    FKAgent fka;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(fka, m_fAge), H5T_NATIVE_FLOAT);

}

