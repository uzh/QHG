#ifndef __FKPOP_H__
#define __FKPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "RandomMove.h"
#include "Verhulst.h"
#include "GetOld.h"

struct FKAgent : Agent {
    float m_fAge;
};


class FKPop : public SPopulation<FKAgent> {

 public:
    FKPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~FKPop();
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);


 protected:
    RandomMove<FKAgent> *pRM;
    Verhulst<FKAgent> *pVer;
    GetOld<FKAgent> *pGO;
};

#endif
