#ifndef __GENLANDDWELLERPOP_H__
#define __GENLANDDWELLERPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "Genetics.h"


struct GenLandDwellerAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class GenLandDwellerPop : public SPopulation<GenLandDwellerAgent> {

 public:
    GenLandDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~GenLandDwellerPop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<GenLandDwellerAgent> *m_pWM;
    MultiEvaluator<GenLandDwellerAgent> *m_pME;
    SingleEvaluator<GenLandDwellerAgent> *m_pSE;
    VerhulstVarK<GenLandDwellerAgent> *m_pVerVarK;
    RandomPair<GenLandDwellerAgent> *m_pPair;
    GetOld<GenLandDwellerAgent> *m_pGO;
    Genetics<GenLandDwellerAgent> *m_pGenetics;
    double *m_adEnvWeights;
    double *m_adMoveWeights;

    bool m_bCreateGenomes;
};


#endif
