#include <math.h>
#include "LatAltitudinal.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "utils.h"

LatAltitudinal::LatAltitudinal(QMapReader<short int> *pQMR, double dMax, bool bInverseLat) 
    :    m_pQMR(pQMR),
         m_dMax(dMax),
         m_dFactor(bInverseLat?-1:1) {

}

double LatAltitudinal::calc(double dLon, double dLat) {
    double dV = 0;
    double dAlt = m_pQMR->getDValue(dLon, m_dFactor*dLat);
    if (dAlt < m_dMax) {
        dV = 1  -  dAlt/m_dMax;
    } 
    return dV*cos(M_PI*dLat/180);
}
