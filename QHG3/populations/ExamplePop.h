#ifndef __EXAMPLEPOP_H__
#define __EXAMPLEPOP_H__

#include <hdf5.h>

#include "SPopulation.h"
#include "RandomMove.h"
#include "GetOld.h"

struct ExampleAgent : Agent {
    float m_fMass;
    float m_fAge;
};


class ExamplePop : public SPopulation<ExampleAgent> {

 public:
    ExamplePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~ExamplePop();

 protected:
    RandomMove<ExampleAgent> *pRM;
    GetOld<ExampleAgent> *pGO;

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);

    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

};


#endif
