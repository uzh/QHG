#ifndef __SHAREEVALUATOR_H__
#define __SHAREEVALUATOR_H__

#include <string>

#include "Action.h"
#include "Evaluator.h"

#define SHAREEVAL_NAME "ShareEvaluator"
#define SHAREEVAL_ARRAYNAME "ShareEvaluator_%s_arrayname"
#define SHAREEVAL_POLYNAME  "ShareEvaluator_%s_polyname"

#define SHARE_NAME_LEN 512

class WELL512;
class PolyLine;


template<typename T>
class ShareEvaluator : public Action<T>, public Evaluator {

public:
    ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, const char *sID, bool bCumulate = true, bool* pbUpdateNeeded = NULL);
    ~ShareEvaluator();
    int initialize(float fT);
    int preLoop();
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

protected:
    PolyLine *m_pPL;           // how to go from env values to weights
    double *m_adOutputWeights;    // scaled probabilities
    double *m_adInputData;                // array of values from which to compute weights
    int m_iMaxNeighbors;
    bool m_bCumulate;
    bool* m_pbUpdateNeeded;

    void calcValues();
    void exchangeAndCumulate();

    char *m_pID;
    char m_sArrayName[SHARE_NAME_LEN];
    char m_sPolyName[SHARE_NAME_LEN];
};


#endif
