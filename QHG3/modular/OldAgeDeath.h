#ifndef __OLDAGEDEATH_H__
#define __OLDAGEDEATH_H__

#include "Action.h"

#define ATTR_OLDAGEDEATH_NAME "OldAgeDeath"
#define ATTR_OLDAGEDEATH_MAXAGE_NAME      "OAD_max_age"
#define ATTR_OLDAGEDEATH_UNCERTAINTY_NAME "OAD_uncertainty"

class WELL512;

template<typename T>
class OldAgeDeath : public Action<T> {
    
 public:
    OldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);
    ~OldAgeDeath();

    int operator()(int iA, float fT);
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void showAttributes();
protected:
    WELL512 **m_apWELL;
    double m_dMaxAge;
    double m_dUncertainty;
};

#endif
