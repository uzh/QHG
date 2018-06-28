#ifndef __RANDOMMOVE_H__
#define __RANDOMMOVE_H__

#include "Action.h"

#define ATTR_RANDOMMOVE_NAME "RandMove"
#define ATTR_RANDOMMOVE_PROB_NAME "RandMoveProb"

class WELL512;

template<typename T>
class RandomMove : public Action<T> {
    
 public:
    RandomMove(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    ~RandomMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void showAttributes();
 protected:
    WELL512 **m_apWELL;
    double m_dMoveProb;
    bool m_bAbsorbing;
};

#endif
