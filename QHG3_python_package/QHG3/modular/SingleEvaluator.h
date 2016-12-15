#ifndef __SINGLEEVALUATOR_H__
#define __SINGLEEVALUATOR_H__

#include <string>

#include "Action.h"
#include "Evaluator.h"

#define SINGLEEVAL_NAME "SingleEvaluator"

class WELL512;
class PolyLine;

template<typename T>
class SingleEvaluator : public Action<T>, public Evaluator {
    
 public:
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate = true, bool* pbUpdateNeeded = NULL);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate = true, bool* pbUpdateNeeded = NULL);
    ~SingleEvaluator();
    int initialize(float fT);
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };
 protected:
    char *m_sPLParName; // name of PolyLine parameter
    PolyLine *m_pPL;           // how to go from env values to weights
    double *m_adOutputWeights; // scaled probabilities
    double *m_adInputData;     // array of values from which to compute weights
    int m_iMaxNeighbors;
    bool m_bCumulate;
    bool* m_pbUpdateNeeded;

    void calcValues();
    void exchangeAndCumulate();

    char *m_pInputArrayName;
};

#endif
