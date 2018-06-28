#ifndef __VERHULST_H__
#define __VERHULST_H__

#include "Action.h"
#include "LinearBirth.h"
#include "LinearDeath.h"

#define ATTR_VERHULST_NAME "Verhulst"
#define ATTR_VERHULST_B0_NAME "Verhulst_b0"
#define ATTR_VERHULST_D0_NAME "Verhulst_d0"
#define ATTR_VERHULST_TURNOVER_NAME "Verhulst_theta"
#define ATTR_VERHULST_CAPACITY_NAME "Verhulst_K"

template<typename T>
class Verhulst : public Action<T> { 

public:
    Verhulst(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, int iMateOffset=-1);
    ~Verhulst();
    
    int initialize(float fTime);
    int operator()(int iA, float fT);
    int finalize(float fTime);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine); 
    
    void showAttributes();

protected:
    static int NUM_VERHULST_PARAMS;
protected:
    double m_dB0;
    double m_dD0;
    double m_dTheta;
    double m_dK;
    int m_iMateOffset;
    int m_iNumSetParams;
    WELL512 **m_apWELL;
    LinearBirth<T> *m_pLB;
    LinearDeath<T> *m_pLD;
   
};

#endif
