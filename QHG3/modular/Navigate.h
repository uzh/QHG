#ifndef __NAVIGATE_H__
#define __NAVIGATE_H__

#include <map>

#include "Observer.h"
#include "Action.h"

typedef std::pair<int, double> targdist;

typedef std::map<int, std::pair<int, targdist*> > distprobmap;

#define ATTR_NAVIGATE_NAME  "Navigate"
#define ATTR_NAVIGATE_DECAY_NAME    "NavigateDecay"
#define ATTR_NAVIGATE_DIST0_NAME    "NavigateDist0"
#define ATTR_NAVIGATE_PROB0_NAME    "NavigateProb0"
#define ATTR_NAVIGATE_MINDENS_NAME  "NavigateMinDens"


template<typename T>
class Navigate : public Action<T>, Observer {
    
 public:
    Navigate(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    ~Navigate();

    void recalculate();
    void cleanup();

    virtual int operator()(int iA, float fT);

    virtual int preLoop(); 
    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryReadParamLine(char *pLine);

    void notify(Observable *pObs, int iEvent, const void *pData);

    virtual void showAttributes();
 protected:
    WELL512 **m_apWELL;
    double    m_dDecay;
    double    m_dDist0;
    double    m_dProb0;
    double    m_dMinDens;
    double    m_dA;
    bool      m_bNeedUpdate;

    distprobmap m_mDistProbs;
};

#endif
