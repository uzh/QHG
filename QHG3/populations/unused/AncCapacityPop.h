#ifndef __ANCCAPACITYPOP_H__
#define __ANCCAPACITYPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "AncestorBoxR.h"


struct AncCapacityAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class AncCapacityPop : public SPopulation<AncCapacityAgent> {

 public:
    AncCapacityPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~AncCapacityPop();

    int preLoop();
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

 protected:
    RandomMove<AncCapacityAgent> *m_pRM;
    MultiEvaluator<AncCapacityAgent> *m_pME;
    VerhulstVarK<AncCapacityAgent> *m_pVerVarK;
    RandomPair<AncCapacityAgent> *m_pPair;
    GetOld<AncCapacityAgent> *m_pGO;
    double *m_adEnvWeights;
    AncestorBoxR *m_pAncBox;
};


#endif
