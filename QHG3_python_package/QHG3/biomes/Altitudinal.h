#ifndef __ALTITUDINAL_H__
#define __ALTITUDINAL_H__

#include "TrinaryFunc.h"
#include "QMapReader.h"

class DEM;

class Altitudinal : public TrinaryFunc {
public:
    Altitudinal(QMapReader<short int> *pQMR, double dMax, bool bInverseLat); 
    
    double calc(double dLon, double dLat);
protected:
    QMapReader<short int> *m_pQMR;
    double                m_dMax;
    double                m_dFactor;
};

#endif
