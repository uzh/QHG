#ifndef __OOAPOP_H__
#define __OOAPOP_H__

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


struct OoAAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAPop : public SPopulation<OoAAgent> {
public:
    OoAPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoAPop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, float fT);

 protected:
    WeightedMove<OoAAgent> *m_pWM;
    //    ConfinedMove<OoAAgent> *m_pCM;
    MultiEvaluator<OoAAgent> *m_pME;
    VerhulstVarK<OoAAgent> *m_pVerVarK;
    RandomPair<OoAAgent> *m_pPair;
    GetOld<OoAAgent> *m_pGO;
    OldAgeDeath<OoAAgent> *m_pOAD;
    Fertility<OoAAgent> *m_pFert;
    NPPCapacity<OoAAgent> *m_pNPPCap;
    Genetics<OoAAgent,BitGeneUtils> *m_pGenetics;
    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;
};


#endif
