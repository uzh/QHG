#ifndef __LINBIRTH_H__
#define __LINBIRTH_H__

#include "Action.h"

#define ATTR_LINBIRTH_NAME "LinearBirth"
#define ATTR_LINBIRTH_B0_NAME "LinearBirth_B0"
#define ATTR_LINBIRTH_TURNOVER_NAME "LinearBirth_theta"
#define ATTR_LINBIRTH_CAPACITY_NAME "LinearBirth_K"

class WELL512;

template<typename T>
class LinearBirth : public Action<T> {
    
 public:
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, int iMateOffset=-1);
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dB0, double dTheta, double dK, int iMateOffset=-1);
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double dB0, double dTheta, double* adK, int iStride, int iMateOffset=-1);
    ~LinearBirth();

    int initialize(float fT);
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void showAttributes();
 protected:
    WELL512 **m_apWELL;
    double *m_adB;
    double m_dB0;
    double m_dTheta;
    double m_dK;
    double* m_adK;
    int m_iStride;
    int m_iMateOffset;  // leave as -1 if reproduction is not in couples (father=mother)
};

#endif
