#ifndef __SINGLEEVALUATOR_H__
#define __SINGLEEVALUATOR_H__

#include <string>

#include "types.h"
#include "Action.h"
#include "Evaluator.h"

#define ATTR_SINGLEEVAL_NAME "SingleEvaluator"

class WELL512;
class PolyLine;

template<typename T>
class SingleEvaluator : public Action<T>, public Evaluator {
    
 public:
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, intset &sTriggerIDs);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, int iTriggerID);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, intset &sTriggerIDs);
    ~SingleEvaluator();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    void showAttributes();
 protected:
    char *m_sPLParName; // name of PolyLine parameter
    PolyLine *m_pPL;           // how to go from env values to weights
    double *m_adOutputWeights; // scaled probabilities
    double *m_adInputData;     // array of values from which to compute weights
    int m_iMaxNeighbors;
    bool m_bCumulate;
    intset m_sTriggerIDs;
    bool m_bAlwaysUpdate;
    void calcValues();
    void exchangeAndCumulate();

    char *m_pInputArrayName;
    bool m_bFirst;
};

#endif
