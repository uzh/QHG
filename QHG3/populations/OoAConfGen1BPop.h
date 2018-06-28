#ifndef __OOACONFGEN1BPOP_H__
#define __OOACONFGEN1BPOP_H__

#include "BitGeneUtils.h"
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


struct OoAConfGen1BAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAConfGen1BPop : public SPopulation<OoAConfGen1BAgent> {
public:
    OoAConfGen1BPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoAConfGen1BPop();

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
    WeightedMove<OoAConfGen1BAgent> *m_pWM;
    ConfinedMove<OoAConfGen1BAgent> *m_pCM;
    MultiEvaluator<OoAConfGen1BAgent> *m_pME;
    VerhulstVarK<OoAConfGen1BAgent> *m_pVerVarK;
    RandomPair<OoAConfGen1BAgent> *m_pPair;
    GetOld<OoAConfGen1BAgent> *m_pGO;
    OldAgeDeath<OoAConfGen1BAgent> *m_pOAD;
    Fertility<OoAConfGen1BAgent> *m_pFert;
    NPPCapacity<OoAConfGen1BAgent> *m_pNPPCap;
    Genetics<OoAConfGen1BAgent,BitGeneUtils> *m_pGenetics;
    Navigate<OoAConfGen1BAgent> *m_pNavigate;

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
