#ifndef __WEIGHTEDMOVE_H__
#define __WEIGHTEDMOVE_H__

#include "Action.h"
#include "PolyLine.h"

#define ATTR_WEIGHTEDMOVE_NAME "WeightedMove"
#define ATTR_WEIGHTEDMOVE_PROB_NAME "WeightedMoveProb"

template<typename T>
class WeightedMove : public Action<T> {
    
 public:
    WeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adEnvWeights);
    ~WeightedMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void showAttributes();
 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;

};

#endif
