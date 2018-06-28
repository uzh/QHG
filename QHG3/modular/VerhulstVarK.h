#ifndef __VERHULSTVARK_H__
#define __VERHULSTVARK_H__

#include "Action.h"
#include "LinearBirth.h"
#include "LinearDeath.h"

#define ATTR_VERHULSTVARK_NAME "VerhulstVarK"
#define ATTR_VERHULST_B0_NAME "Verhulst_b0"
#define ATTR_VERHULST_D0_NAME "Verhulst_d0"
#define ATTR_VERHULST_TURNOVER_NAME "Verhulst_theta"

template<typename T>
class VerhulstVarK : public Action<T> { 

public:
   VerhulstVarK(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adK, int iStride, int iMateOffset/*=-1*/);
   ~VerhulstVarK();

   int initialize(float fTime);
   int operator()(int iA, float fT);
   int finalize(float fTime);

   int extractParamsQDF(hid_t hSpeciesGroup);
   int writeParamsQDF(hid_t hSpeciesGroup);
   int tryReadParamLine(char *pLine); 

   void showAttributes();

protected:
    static int NUM_VERHULSTVARK_PARAMS;
protected:
   double m_dB0;
   double m_dD0;
   double m_dTheta;
   double* m_adK;
   int m_iStride;
   int m_iMateOffset;
   int m_iNumSetParams;
   WELL512 **m_apWELL;
   LinearBirth<T> *m_pLB;
   LinearDeath<T> *m_pLD;
    
};

#endif
