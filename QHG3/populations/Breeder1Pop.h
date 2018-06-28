#ifndef __BREEDER1POP_H__
#define __BREEDER1POP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"


struct Breeder1Agent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;

};

class Breeder1Pop : public SPopulation<Breeder1Agent> {
public:
    Breeder1Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~Breeder1Pop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<Breeder1Agent> *m_pWM;
    ConfinedMove<Breeder1Agent> *m_pCM;
    MultiEvaluator<Breeder1Agent> *m_pME;
    Verhulst<Breeder1Agent> *m_pVerhulst;
    RandomPair<Breeder1Agent> *m_pPair;
    GetOld<Breeder1Agent> *m_pGO;
    OldAgeDeath<Breeder1Agent> *m_pOAD;
    Fertility<Breeder1Agent> *m_pFert;
    NPPCapacity<Breeder1Agent> *m_pNPPCap;
    Genetics<Breeder1Agent,GeneUtils> *m_pGenetics;
    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
};


#endif
