#ifndef __ANC4ALTMOVERPOP_H__
#define __ANC4ALTMOVERPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "AncestorBox4.h"

struct Anc4AltMoverAgent : Agent {

    float m_fAge;
    int m_iMateIndex;

};


class Anc4AltMoverPop : public SPopulation<Anc4AltMoverAgent> {

 public:
    Anc4AltMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~Anc4AltMoverPop();

    int preLoop();
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    virtual int setParams(const char *pParams);

 protected:
    WeightedMove<Anc4AltMoverAgent> *m_pWM;
    SingleEvaluator<Anc4AltMoverAgent> *m_pAE;
    Verhulst<Anc4AltMoverAgent> *m_pVer;
    RandomPair<Anc4AltMoverAgent> *m_pPair;
    GetOld<Anc4AltMoverAgent> *m_pGO;
    double *m_adEnvWeights;
    AncestorBox4 *m_pAncBox;

    char *m_pAltPrefName;
    char *m_pOutDir;
    char *m_pPrefix;
};


#endif
