#ifndef __NPPCALCNCEAS_H__
#define __NPPCALCNCEAS_H__

#include "NPPCalc.h"
class WELL512;

class NPPCalcNCEAS : public NPPCalc {
public:
    NPPCalcNCEAS(WELL512 **apWell, double *pdWeights);
    virtual ~NPPCalcNCEAS();

    virtual double calcNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *dVariance=0);
    
    virtual double calcNPP(double dT, double dP, double dVariance=0);
        

    double calcGrassNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *adVariance=0);
    double calcBushNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *adVariance=0);
    double calcTreeNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *adVariance=0);
    
      
protected:
    
    double calcGrassNPP(double dT, double dP, double dVariance=0);
    double calcBushNPP(double dT, double dP, double dVariance=0);
    double calcTreeNPP(double dT, double dP, double dVariance=0);

    double *m_pdWeights;
    bool m_bDeleteWeights;
};

#endif
