#ifndef __MULTIEVALUATOR_H__
#define __MULTIEVALUATOR_H__

#include <string>
#include <map>

#include "Observable.h"
#include "Observer.h"

#include "Action.h"


#define ATTR_MULTIEVAL_NAME "MultiEvaluator"

#define MODE_ADD_SIMPLE  0
#define MODE_ADD_BLOCK   1
#define MODE_MUL_SIMPLE  2
#define MODE_MAX_SIMPLE  3
#define MODE_MAX_BLOCK   4
#define MODE_MIN_SIMPLE  5


class Evaluator;

typedef std::vector<std::pair<std::string, Evaluator*> > evaluatorinfos;

template<typename T>
class MultiEvaluator : public Action<T>, public Evaluator, public Observable{

 public:
    MultiEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, evaluatorinfos &mEvaluators, int iMode, bool bDeleteEvaluators);
    ~MultiEvaluator();
    int initialize(float fT);
    int finalize(float fT);
    int preLoop();
    int postLoop();
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    void showAttributes();
 protected:
    bool  m_bDeleteEvaluators;
    double *m_adOutputWeights;    // scaled probabilities
    int m_iMaxNeighbors;
    int m_iNumEvals;
    int m_iMode;
    bool m_bFirst;

    // here the array of Evaluators
    Evaluator **m_aEvaluators;

    // here the array with the parameter names for the weights
    char **m_asCombinationWeightNames;

    // here the array with the weights to combine the evaluators
    double *m_adCombinationWeights;

    // this array will be used by each SingleEvaluator in turn to save space
    double *m_adSingleEvalWeights;

    // called by the different constructors
    void init(evaluatorinfos &mEvalInfo, int iMode);

    int addSingleWeights(float fT);
  
    int addSingleWeightsBlock(float fT);
  
    int multiplySingleWeights(float fT);

    int maxSingleWeights(float fT);

    int maxSingleWeightsBlock(float fT);

    int minSingleWeights(float fT);

    int findBlockings(float fT);
    uchar *m_acAllowed;

};

#endif
