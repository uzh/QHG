#ifndef __ALTMOVERPOP_H__
#define __ALTMOVERPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"

struct AltMoverAgent : Agent {

};


class AltMoverPop : public SPopulation<AltMoverAgent> {

 public:
    AltMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~AltMoverPop();

 protected:
    WeightedMove<AltMoverAgent> *m_pWM;
    SingleEvaluator<AltMoverAgent> *m_pAE;
    double *m_adEnvWeights;

};


#endif
