#ifndef __LANDDWELLERPOP_H__
#define __LANDDWELLERPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"


struct LandDwellerAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class LandDwellerPop : public SPopulation<LandDwellerAgent> {

 public:
    LandDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~LandDwellerPop();

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<LandDwellerAgent> *m_pWM;
    MultiEvaluator<LandDwellerAgent> *m_pME;
    SingleEvaluator<LandDwellerAgent> *m_pSE;
    VerhulstVarK<LandDwellerAgent> *m_pVerVarK;
    RandomPair<LandDwellerAgent> *m_pPair;
    GetOld<LandDwellerAgent> *m_pGO;
    double *m_adEnvWeights;
    double *m_adMoveWeights;
};


#endif
