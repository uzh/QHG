#ifndef __VEGDWELLERPOP_H__
#define __VEGDWELLERPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "AncestorBoxR.h"


struct VegDwellerAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class VegDwellerPop : public SPopulation<VegDwellerAgent> {

 public:
    VegDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~VegDwellerPop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<VegDwellerAgent> *m_pWM;
    MultiEvaluator<VegDwellerAgent> *m_pME;
    SingleEvaluator<VegDwellerAgent> *m_pSE;
    VerhulstVarK<VegDwellerAgent> *m_pVerVarK;
    RandomPair<VegDwellerAgent> *m_pPair;
    GetOld<VegDwellerAgent> *m_pGO;
    double *m_adEnvWeights;
    double *m_adMoveWeights;
    AncestorBoxR *m_pAncBox;

    char *m_pOutDir;
    char *m_pPrefix;
};


#endif
