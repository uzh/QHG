#ifndef __NPPCALC_H__
#define __NPPCALC_H__


class WELL512;

class NPPCalc {
public:
    NPPCalc(WELL512 **apWell);
    virtual ~NPPCalc() {};
    virtual double calcNPP(double *dT, double *dP, int iNum, int iStride, double *dOut, double *dVariance=0)=0;
    
    virtual double calcNPP(double dT, double dP, double dVariance=0)=0;
    // max value for total = 1800+1800+1600 = 5200
    
protected:
    double calcVariance(double dVariance);

    WELL512 **m_apWELL;
};

#endif
