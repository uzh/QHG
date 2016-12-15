#ifndef __ALTDENSMOVERPOP_H__
#define __ALTDENSMOVERPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"

struct AltDensMoverAgent : Agent {

};


class AltDensMoverPop : public SPopulation<AltDensMoverAgent> {

 public:
    AltDensMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~AltDensMoverPop();

 protected:
    WeightedMove<AltDensMoverAgent> *m_pWM;
    MultiEvaluator<AltDensMoverAgent> *m_pME;
    double *m_adEnvWeights;

};


#endif
