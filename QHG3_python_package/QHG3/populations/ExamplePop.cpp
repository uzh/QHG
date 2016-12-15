#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"
#include "RandomMove.cpp"
#include "GetOld.cpp"

#include "ExamplePop.h"

//----------------------------------------------------------------------------
// constructor
//
ExamplePop::ExamplePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<ExampleAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState) {
    
    pRM = new RandomMove<ExampleAgent>(this, m_pCG, m_apWELL);
    pGO = new GetOld<ExampleAgent>(this, m_pCG);

    // TO DO: actually we can just make RANDOMMOVE_NAME a member of RandomMove
    // and let m_prio fetch the name of the action from the action itself...
    m_prio.addAction(RANDOMMOVE_NAME,  pRM);
    m_prio.addAction(GETOLD_NAME, pGO);
    
}

///----------------------------------------------------------------------------
// destructor
//
ExamplePop::~ExamplePop() {
    delete pRM;
    delete pGO;
}


//----------------------------------------------------------------------------
// makePopSpecificOffspring
// here we do the extra stuff needed to create a new agent for this population
//
int ExamplePop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {

    ExampleAgent *pA = static_cast<ExampleAgent*>(&m_aAgents[iAgent]);
    ExampleAgent *pM = static_cast<ExampleAgent*>(&m_aAgents[iMother]);
    ExampleAgent *pF = static_cast<ExampleAgent*>(&m_aAgents[iFather]);

    pA->m_fMass = pM->m_fMass + pF->m_fMass;
    pA->m_fAge = 0.0;

    return 0;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file
//
int ExamplePop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    
    int iResult = 0;
    
    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fMass);

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void ExamplePop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    ExampleAgent ea;
    H5Tinsert(*hAgentDataType, "Mass", qoffsetof(ea, m_fMass), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Age",  qoffsetof(ea, m_fAge), H5T_NATIVE_FLOAT);
   
}

