
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "Verhulst.cpp"
#include "Stress1Pop.h"

//----------------------------------------------------------------------------
// constructor
//
Stress1Pop::Stress1Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds)
    : SPopulation<Stress1Agent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds)
{

  m_pVerhulst = new Verhulst<Stress1Agent>(this, m_pCG, m_apWELL, -1);

  // adding all actions to prioritizer

  m_prio.addAction(ATTR_VERHULST_NAME, m_pVerhulst);
 }

///----------------------------------------------------------------------------
// destructor
//
Stress1Pop::~Stress1Pop() {

  if (m_pVerhulst != NULL) {
    delete m_pVerhulst;
  }
  
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int Stress1Pop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
  int iResult = 0;

  m_aAgents[iAgent].m_fAge = 0.0;
  m_aAgents[iAgent].m_fLastBirth = 0.0;
  m_aAgents[iAgent].m_iMateIndex = -3;

  return iResult;
}

///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int Stress1Pop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
  int iResult = 0;
  if (iResult == 0)  {
    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
  }
  if (iResult == 0)   {
    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
  }
  return iResult;
}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void Stress1Pop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

  Stress1Agent agent;
  H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
  H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
}
