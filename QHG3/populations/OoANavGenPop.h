#ifndef __OOANAVGENPOP_H__
#define __OOANAVGENPOP_H__

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


struct OoANavGenAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenPop : public SPopulation<OoANavGenAgent> {
public:
    OoANavGenPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenPop();

    virtual int preLoop();

    virtual int setParams(const char *pParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<OoANavGenAgent> *m_pWM;
    //    ConfinedMove<OoANavGenAgent> *m_pCM;
    MultiEvaluator<OoANavGenAgent> *m_pME;
    VerhulstVarK<OoANavGenAgent> *m_pVerVarK;
    RandomPair<OoANavGenAgent> *m_pPair;
    GetOld<OoANavGenAgent> *m_pGO;
    OldAgeDeath<OoANavGenAgent> *m_pOAD;
    Fertility<OoANavGenAgent> *m_pFert;
    NPPCapacity<OoANavGenAgent> *m_pNPPCap;
    Genetics<OoANavGenAgent,BitGeneUtils> *m_pGenetics;
    Navigate<OoANavGenAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

};


#endif
