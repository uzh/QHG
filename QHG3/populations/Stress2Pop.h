#ifndef __STRESS2POP_H__
#define __STRESS2POP_H__

#include "SPopulation.h"
#include "RandomMove.h"
#include "Verhulst.h"



struct Stress2Agent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
};

class Stress2Pop : public SPopulation<Stress2Agent> {
public:
    Stress2Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *   aulState, uint *aiSeeds);
    ~Stress2Pop();

  
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    int setParams(const char *pParams) { return 0;};

 protected:
    RandomMove<Stress2Agent> *m_pRM;
    Verhulst<Stress2Agent> *m_pVerhulst;
   

};


#endif
