#ifndef __CONDWEIGHTEDMOVE_H__
#define __CONDWEIGHTEDMOVE_H__

#include "Action.h"
#include "PolyLine.h"
#include "MoveCondition.h"

#define CONDWEIGHTEDMOVE_NAME "CondWeightedMove"
#define CONDWEIGHTEDMOVE_PROB_NAME "CondWeightedMoveProb"



template<typename T>
class CondWeightedMove : public Action<T> {
    
 public:
    CondWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adEnvWeights, MoveCondition *pMC);
    ~CondWeightedMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;
    MoveCondition *m_pMC;
};

#endif
