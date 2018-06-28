#ifndef __GENETICMOVERPOP_H__
#define __GENETICMOVERPOP_H__

#include <hdf5.h>

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "Genetics.h"

struct GeneticMoverAgent : Agent {
    int   m_iMateIndex;
    float m_fAge;
};


class GeneticMoverPop : public SPopulation<GeneticMoverAgent> {

 public:
    GeneticMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~GeneticMoverPop();

    virtual int preLoop();
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    virtual int setParams(const char *pParams);

 protected:
    WeightedMove<GeneticMoverAgent> *m_pWM;
    SingleEvaluator<GeneticMoverAgent> *m_pAE;
    Verhulst<GeneticMoverAgent> *m_pVer;
    RandomPair<GeneticMoverAgent> *m_pPair;
    GetOld<GeneticMoverAgent> *m_pGO;
    double *m_adEnvWeights;
    Genetics<GeneticMoverAgent,GeneUtils> *m_pGenetics;

    char *m_pAltPrefName;
    
    bool m_bCreateGenomes;
};


#endif
