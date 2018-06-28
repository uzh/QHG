#ifndef __ALTDENSGENMOVERPOP_H__
#define __ALTDENSGENMOVERPOP_H__

#include <hdf5.h>

#include "SPopulation.h"

#include "GeneUtils.h"
#include "MoveCondition.h"
#include "SimpleCondition.h"
#include "CondWeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "Genetics.h"

struct AltDensGenMoverAgent : Agent {
    int   m_iMateIndex;
};


class AltDensGenMoverPop : public SPopulation<AltDensGenMoverAgent> {

 public:
    AltDensGenMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~AltDensGenMoverPop();

    virtual int preLoop();
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int setParams(const char *pParams);

 protected:
    
    MoveCondition *m_pMC;
    CondWeightedMove<AltDensGenMoverAgent> *m_pCWM;
    MultiEvaluator<AltDensGenMoverAgent> *m_pME;
    Verhulst<AltDensGenMoverAgent> *m_pVer;
    RandomPair<AltDensGenMoverAgent> *m_pPair;
    Genetics<AltDensGenMoverAgent,GeneUtils> *m_pGenetics;

    double *m_adEnvWeights;

    bool m_bCreateGenomes;
};


#endif
