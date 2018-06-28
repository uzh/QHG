#ifndef __MASSMANAGER_H__
#define __MASSMANAGER_H__

#include "Action.h"

#define ATTR_MASSMANAGER_NAME       "MassManager"
#define ATTR_MASSMANAGER_MIN_NAME   "MM_minmass"
#define ATTR_MASSMANAGER_MAX_NAME   "MM_maxmass"
#define ATTR_MASSMANAGER_DELTA_NAME "MM_deltamass"

class MassInterface;

template<typename T>
class MassManager : public Action<T> {
public:
    MassManager(SPopulation<T> *pPop, SCellGrid *pCG);

    int preLoop();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    double getMinMass() { return m_dMinMass;};

    void showAttributes();
protected:
    double m_dMinMass;
    double m_dMaxMass;
    double m_dDelta;
    MassInterface *m_pMI;
};


#endif
