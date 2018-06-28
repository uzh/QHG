#ifndef __OOANAVGEN2BITASSPOP_H__
#define __OOANAVGEN2BitAssPOP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "AssortativePair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"


struct OoANavGen2bitAssAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGen2bitAssPop : public SPopulation<OoANavGen2bitAssAgent>  {
public:
    OoANavGen2bitAssPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGen2bitAssPop();

    int preLoop();
  
  int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<OoANavGen2bitAssAgent> *m_pWM;
    //    ConfinedMove<OoANavGen2bitAssAgent> *m_pCM;
    MultiEvaluator<OoANavGen2bitAssAgent> *m_pME;
    VerhulstVarK<OoANavGen2bitAssAgent> *m_pVerVarK;
    AssortativePair<OoANavGen2bitAssAgent,GeneUtils> *m_pPair;
    GetOld<OoANavGen2bitAssAgent> *m_pGO;
    OldAgeDeath<OoANavGen2bitAssAgent> *m_pOAD;
    Fertility<OoANavGen2bitAssAgent> *m_pFert;
    NPPCapacity<OoANavGen2bitAssAgent> *m_pNPPCap;
    Genetics<OoANavGen2bitAssAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGen2bitAssAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;

};


#endif
