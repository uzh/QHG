#ifndef __FK1DPOP_H__
#define __FK1DPOP_H__

#include <hdf5.h>

#include "FKPop.h"
#include "SPopulation.h"
#include "RandomMove1D.h"
#include "Verhulst.h"
#include "GetOld.h"


class FK1DPop : public SPopulation<FKAgent> {

 public:
    FK1DPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~FK1DPop();
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);


 protected:
    RandomMove1D<FKAgent> *pRM;
    Verhulst<FKAgent> *pVer;
    GetOld<FKAgent> *pGO;
};

#endif
