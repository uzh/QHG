#ifndef __OOANAVPOP_H__
#define __OOANAVPOP_H__

#include "BitGeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"


struct OoANavAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavPop : public SPopulation<OoANavAgent> {
public:
    OoANavPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavPop();

    virtual int setParams(const char *pParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);
 
 protected:
    WeightedMove<OoANavAgent> *m_pWM;
    MultiEvaluator<OoANavAgent> *m_pME;
    VerhulstVarK<OoANavAgent> *m_pVerVarK;
    RandomPair<OoANavAgent> *m_pPair;
    GetOld<OoANavAgent> *m_pGO;
    OldAgeDeath<OoANavAgent> *m_pOAD;
    Fertility<OoANavAgent> *m_pFert;
    NPPCapacity<OoANavAgent> *m_pNPPCap;
    Navigate<OoANavAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

};


#endif
