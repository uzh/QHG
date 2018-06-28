#ifndef __OOASELPOP_H__
#define __OOASELPOP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "SelPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"


struct OoASelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;

};

class OoASelPop : public SPopulation<OoASelAgent> {
public:
    OoASelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoASelPop();

    int preLoop();
    int postLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<OoASelAgent> *m_pWM;
    ConfinedMove<OoASelAgent> *m_pCM;
    MultiEvaluator<OoASelAgent> *m_pME;
    VerhulstVarK<OoASelAgent> *m_pVerVarK;
    SelPair<OoASelAgent> *m_pPair;
    GetOld<OoASelAgent> *m_pGO;
    OldAgeDeath<OoASelAgent> *m_pOAD;
    Fertility<OoASelAgent> *m_pFert;
    NPPCapacity<OoASelAgent> *m_pNPPCap;
    Genetics<OoASelAgent,GeneUtils> *m_pGenetics;
    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
};


#endif
