#ifndef __MINIOOAPOP_H__
#define __MINIOOAPOP_H__

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"


struct MiniOoAAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
};

class MiniOoAPop : public SPopulation<MiniOoAAgent> {
public:
    MiniOoAPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~MiniOoAPop();

  
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<MiniOoAAgent> *m_pWM;
    MultiEvaluator<MiniOoAAgent> *m_pME;
    VerhulstVarK<MiniOoAAgent> *m_pVerVarK;
    RandomPair<MiniOoAAgent> *m_pPair;
    GetOld<MiniOoAAgent> *m_pGO;
    OldAgeDeath<MiniOoAAgent> *m_pOAD;
    Fertility<MiniOoAAgent> *m_pFert;
    NPPCapacity<MiniOoAAgent> *m_pNPPCap;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;

};


#endif
