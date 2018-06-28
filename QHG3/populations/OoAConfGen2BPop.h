#ifndef __OOACONFGEN2BPOP_H__
#define __OOACONFGEN2BPOP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
#include "ConfinedMove.h"
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


struct OoAConfGen2BAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAConfGen2BPop : public SPopulation<OoAConfGen2BAgent> {
public:
    OoAConfGen2BPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulStat, uint *aiSeedse);
    ~OoAConfGen2BPop();

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
    WeightedMove<OoAConfGen2BAgent> *m_pWM;
    ConfinedMove<OoAConfGen2BAgent> *m_pCM;
    MultiEvaluator<OoAConfGen2BAgent> *m_pME;
    VerhulstVarK<OoAConfGen2BAgent> *m_pVerVarK;
    RandomPair<OoAConfGen2BAgent> *m_pPair;
    GetOld<OoAConfGen2BAgent> *m_pGO;
    OldAgeDeath<OoAConfGen2BAgent> *m_pOAD;
    Fertility<OoAConfGen2BAgent> *m_pFert;
    NPPCapacity<OoAConfGen2BAgent> *m_pNPPCap;
    Genetics<OoAConfGen2BAgent,GeneUtils> *m_pGenetics;
    Navigate<OoAConfGen2BAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;

    // child & death stistics
    float *m_afChildrenPerYear;
    int   *m_aiChildrenPerYearCount;
    float *m_afDeathAge;
    int   *m_aiDeathAgeCount;
};


#endif
