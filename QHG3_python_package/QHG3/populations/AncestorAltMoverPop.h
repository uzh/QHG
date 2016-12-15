#ifndef __ANCALTMOVERPOP_H__
#define __ANCALTMOVERPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "AncestorBoxR.h"

struct AncestorAltMoverAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class AncestorAltMoverPop : public SPopulation<AncestorAltMoverAgent> {

 public:
    AncestorAltMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~AncestorAltMoverPop();

    int preLoop();
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    virtual int setParams(const char *pParams);

 protected:
    WeightedMove<AncestorAltMoverAgent> *m_pWM;
    SingleEvaluator<AncestorAltMoverAgent> *m_pAE;
    Verhulst<AncestorAltMoverAgent> *m_pVer;
    RandomPair<AncestorAltMoverAgent> *m_pPair;
    GetOld<AncestorAltMoverAgent> *m_pGO;
    double *m_adEnvWeights;
    AncestorBoxR *m_pAncBox;

    char *m_pAltPrefName;
    char *m_pOutDir;
    char *m_pPrefix;
};


#endif
