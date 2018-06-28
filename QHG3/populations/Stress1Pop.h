#ifndef __STRESS1POP_H__
#define __STRESS1POP_H__

#include "SPopulation.h"
#include "Verhulst.h"



struct Stress1Agent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
};

class Stress1Pop : public SPopulation<Stress1Agent> {
public:
    Stress1Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *   aulState, uint *aiSeeds);
    ~Stress1Pop();

  
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    int setParams(const char *pParams) { return 0;};

 protected:
    Verhulst<Stress1Agent> *m_pVerhulst;
   
};


#endif
