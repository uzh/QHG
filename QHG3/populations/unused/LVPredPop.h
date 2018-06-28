#ifndef __LVPREDPOP_H__
#define __LVPREDPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "LotkaVolterra.h"

struct LVPredAgent : Agent {
    int m_iDummy;
};

class LVPredPop : public SPopulation<LVPredAgent> {
 public:
    LVPredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~LVPredPop();

    int preLoop();
  

 protected:
    RandomMove<LVPredAgent> *m_pRM;
    LotkaVolterra<LVPredAgent> *m_pLV;
};




#endif
