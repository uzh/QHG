#ifndef __OOANAVGEN2BITASSHAMPOP_H__
#define __OOANAVGEN2BitAssHAMPOP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "AssortativePairHam.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"


struct OoANavGen2bitAssHamAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGen2bitAssHamPop : public SPopulation<OoANavGen2bitAssHamAgent> {
public:
    OoANavGen2bitAssHamPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGen2bitAssHamPop();

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
    WeightedMove<OoANavGen2bitAssHamAgent> *m_pWM;
    //    ConfinedMove<OoANavGen2bitAssHamAgent> *m_pCM;
    MultiEvaluator<OoANavGen2bitAssHamAgent> *m_pME;
    VerhulstVarK<OoANavGen2bitAssHamAgent> *m_pVerVarK;
    AssortativePairHam<OoANavGen2bitAssHamAgent,GeneUtils> *m_pPair;
    GetOld<OoANavGen2bitAssHamAgent> *m_pGO;
    OldAgeDeath<OoANavGen2bitAssHamAgent> *m_pOAD;
    Fertility<OoANavGen2bitAssHamAgent> *m_pFert;
    NPPCapacity<OoANavGen2bitAssHamAgent> *m_pNPPCap;
    Genetics<OoANavGen2bitAssHamAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGen2bitAssHamAgent> *m_pNavigate;

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
