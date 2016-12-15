#ifndef __RANDOMMOVE1D_H__
#define __RANDOMMOVE1D_H__

#include "Action.h"

#define RANDOMMOVE1D_NAME "RandMove1D"
#define RANDOMMOVE1D_PROB_NAME "RandMove1DProb"

class WELL512;

template<typename T>
class RandomMove1D : public Action<T> {
    
 public:
    RandomMove1D(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    ~RandomMove1D();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

 protected:
    WELL512 **m_apWELL;
    double m_dMoveProb;
    bool m_bAbsorbing;
};

#endif
