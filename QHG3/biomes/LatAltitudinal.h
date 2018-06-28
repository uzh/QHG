#ifndef __LATALTITUDINAL_H__
#define __LATALTITUDINAL_H__

#include "TrinaryFunc.h"
#include "QMapReader.h"

class DEM;

class LatAltitudinal : public TrinaryFunc {
public:
    LatAltitudinal(QMapReader<short int> *pQMR, double dMax, bool bInverseLat); 

    double calc(double dLon, double dLat);
protected:
    QMapReader<short int> *m_pQMR;
    double                m_dMax;
    double                m_dFactor;
};

#endif
