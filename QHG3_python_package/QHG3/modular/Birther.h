#ifndef __BIRTHER_H__
#define __BIRTHER_H__

#include "Action.h"

#define BIRTHER_NAME       "Birther"
#define BIRTHER_ADULTMASS_NAME   "Birther_adultmass"
#define BIRTHER_BIRTHMASS_NAME   "Birther_birthmass"
#define BIRTHER_UNCERTAINTY_NAME "Birther_uncertainty"

class WELL512;
class MassInterface;


template<typename T>
class Birther : public Action<T> {
public:
    Birther(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL);

    int preLoop();
    int operator()(int iA, float fT);
    
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

protected:
    WELL512 **m_apWELL;
    double m_dAdultMass;
    double m_dBirthMass;
    double m_dUncertainty;
    MassInterface *m_pMI;
};

#endif

