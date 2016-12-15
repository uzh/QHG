#ifndef __COASTDWELLERPOP_H__
#define __COASTDWELLERPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "AncestorBoxR.h"


struct CoastDwellerAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class CoastDwellerPop : public SPopulation<CoastDwellerAgent> {

 public:
    CoastDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~CoastDwellerPop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<CoastDwellerAgent> *m_pWM;
    MultiEvaluator<CoastDwellerAgent> *m_pME;
    SingleEvaluator<CoastDwellerAgent> *m_pSE;
    VerhulstVarK<CoastDwellerAgent> *m_pVerVarK;
    RandomPair<CoastDwellerAgent> *m_pPair;
    GetOld<CoastDwellerAgent> *m_pGO;
    double *m_adEnvWeights;
    double *m_adMoveWeights;
    AncestorBoxR *m_pAncBox;

    char *m_pOutDir;
    char *m_pPrefix;
};


#endif
