#ifndef __WEIGHTEDMOVERAND_H__
#define __WEIGHTEDMOVERAND_H__

#include "Action.h"
#include "PolyLine.h"

#define ATTR_WEIGHTEDMOVERAND_NAME      "WeightedMoveRand"
#define ATTR_WEIGHTEDMOVERAND_PROB_NAME "WeightedMoveRandProb"

template<typename T>
class WeightedMoveRand : public Action<T> {
    
 public:
    WeightedMoveRand(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adEnvWeights);
    ~WeightedMoveRand();
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
