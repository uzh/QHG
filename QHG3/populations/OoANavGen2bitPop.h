#ifndef __OOANAVGEN2BITPOP_H__
#define __OOANAVGEN2BitPOP_H__

#include "GeneUtils.h"
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


struct OoANavGen2bitAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGen2bitPop : public SPopulation<OoANavGen2bitAgent> {
public:
    OoANavGen2bitPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGen2bitPop();

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
    WeightedMove<OoANavGen2bitAgent> *m_pWM;
    //    ConfinedMove<OoANavGen2bitAgent> *m_pCM;
    MultiEvaluator<OoANavGen2bitAgent> *m_pME;
    VerhulstVarK<OoANavGen2bitAgent> *m_pVerVarK;
    RandomPair<OoANavGen2bitAgent> *m_pPair;
    GetOld<OoANavGen2bitAgent> *m_pGO;
    OldAgeDeath<OoANavGen2bitAgent> *m_pOAD;
    Fertility<OoANavGen2bitAgent> *m_pFert;
    NPPCapacity<OoANavGen2bitAgent> *m_pNPPCap;
    Genetics<OoANavGen2bitAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGen2bitAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;

};


#endif
