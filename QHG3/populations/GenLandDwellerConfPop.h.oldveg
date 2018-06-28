#ifndef __GENLANDDWELLERCONFPOP_H__
#define __GENLANDDWELLERCONFPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "Genetics.h"
#include "ConfinedMove.h"


struct GenLandDwellerConfAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class GenLandDwellerConfPop : public SPopulation<GenLandDwellerConfAgent> {

 public:
    GenLandDwellerConfPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~GenLandDwellerConfPop();

    int preLoop();
    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    WeightedMove<GenLandDwellerConfAgent> *m_pWM;
    MultiEvaluator<GenLandDwellerConfAgent> *m_pME;
    SingleEvaluator<GenLandDwellerConfAgent> *m_pSE;
    VerhulstVarK<GenLandDwellerConfAgent> *m_pVerVarK;
    RandomPair<GenLandDwellerConfAgent> *m_pPair;
    GetOld<GenLandDwellerConfAgent> *m_pGO;
    Genetics<GenLandDwellerConfAgent> *m_pGenetics;
    ConfinedMove<GenLandDwellerConfAgent> *m_pConfMove;
    double *m_adEnvWeights;
    double *m_adMoveWeights;

    bool m_bCreateGenomes;
};


#endif
