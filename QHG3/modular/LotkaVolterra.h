#ifndef __LOTKAVOLTERRA_H__
#define __LOTKAVOLTERRA_H__

/*---------------------------------------------
  The Lotka-Volterra equations are:

    dx/dt = a*x - b*x*y  // prey
    dy/dt = d*x*y - c*y  //predator

  Both  equations have th form
    dx/dt = u*x - v*x*y
  if (u>0,v<0) it is the predator's equation,
  if (u<0,v>0) it is the prey's equation.

  Here i use the name "SelfRate" for the variable u,
  and "MixRate" for the variable v.
  ---------------------------------------------*/


#include "Action.h"

#define ATTR_LOTKAVOLTERRA_NAME           "LotkaVolterra"
#define ATTR_LOTKAVOLTERRA_SELFRATE_NAME  "LV_selfrate"
#define ATTR_LOTKAVOLTERRA_MIXRATE_NAME   "LV_mixrate"
#define ATTR_LOTKAVOLTERRA_OTHERPOP_NAME  "LV_otherpop"
#define ATTR_LOTKAVOLTERRA_K_NAME         "LV_K"


#define NAME_LEN 1024

class WELL512;

template<typename T>
class LotkaVolterra : public Action<T> {
    
 public:
    LotkaVolterra(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, PopFinder *pPopFinder);
    ~LotkaVolterra();

    int preLoop();
    int initialize(float fT);
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

    void showAttributes();
 protected:
    WELL512 **m_apWELL;
    PopFinder *m_pPopFinder;
    PopBase   *m_pOtherPop;
    double m_dSelfRate;
    double m_dMixRate;
    char   m_sOtherPopname[NAME_LEN];
    double m_dK;
    double *m_adB;
    double *m_adD;
    
};


#endif
