#ifndef __FERTILITY_H__
#define __FERTILITY_H__

#include "Action.h"

#define ATTR_FERTILITY_NAME            "Fertility"
#define ATTR_FERTILITY_MIN_AGE_NAME    "Fertility_min_age"
#define ATTR_FERTILITY_MAX_AGE_NAME    "Fertility_max_age"
#define ATTR_FERTILITY_INTERBIRTH_NAME "Fertility_interbirth"

class WELL512;

template<typename T>
class Fertility : public Action<T> {
    
public:
    Fertility(SPopulation<T> *pPop, SCellGrid *pCG);
    ~Fertility();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine); 
    
    float getMinAge() {return m_fFertilityMinAge;};
    float getMaxAge() {return m_fFertilityMaxAge;};
    float getInterBirth() { return m_fInterbirth;};

    void showAttributes();
protected:
    float m_fFertilityMinAge;
    float m_fFertilityMaxAge;
    float m_fInterbirth;
};

#endif
