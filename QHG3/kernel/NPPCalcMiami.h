#ifndef __NPPCALCMIAMI_H__
#define __NPPCALCMIAMI_H__

#include "NPPCalc.h"
class WELL512;

class NPPCalcMiami : public NPPCalc {
public:
    NPPCalcMiami(WELL512 **apWell);
    virtual ~NPPCalcMiami() {};

    virtual double calcNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *adVariance=NULL);
    
    virtual double calcNPP(double dT, double dP, double dVariance=0);
        
protected:
};

#endif
