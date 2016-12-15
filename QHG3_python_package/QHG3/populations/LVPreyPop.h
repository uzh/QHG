#ifndef __LVPREYPOP_H__
#define __LVPREYPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "LotkaVolterra.h"

struct LVPreyAgent : Agent {
};

class LVPreyPop : public SPopulation<LVPreyAgent> {
 public:
    LVPreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~LVPreyPop();

    int preLoop();
  

 protected:
    RandomMove<LVPreyAgent> *m_pRM;
    LotkaVolterra<LVPreyAgent> *m_pLV;
};




#endif
