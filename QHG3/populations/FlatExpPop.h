#ifndef __FLATEXPPOP_H__
#define __FLATEXPPOP_H__

#include "BitGeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "Genetics.h"


struct FlatExpAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class FlatExpPop : public SPopulation<FlatExpAgent> {
public:
    FlatExpPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~FlatExpPop();

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
    WeightedMove<FlatExpAgent> *m_pWM;
    //    ConfinedMove<FlatExpAgent> *m_pCM;
    MultiEvaluator<FlatExpAgent> *m_pME;
    Verhulst<FlatExpAgent> *m_pVerhulst;
    RandomPair<FlatExpAgent> *m_pPair;
    GetOld<FlatExpAgent> *m_pGO;
    OldAgeDeath<FlatExpAgent> *m_pOAD;
    Fertility<FlatExpAgent> *m_pFert;
    NPPCapacity<FlatExpAgent> *m_pNPPCap;
    Genetics<FlatExpAgent,BitGeneUtils> *m_pGenetics;
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
