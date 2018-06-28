#include <math.h>
#include "Altitudinal.h"
#include "QMapReader.h"
#include "QMapReader.cpp"

Altitudinal::Altitudinal(QMapReader<short int> *pQMR, double dMax, bool bInverseLat) 
    :    m_pQMR(pQMR),
         m_dMax(dMax),
         m_dFactor(bInverseLat?-1:1) {

}

double Altitudinal::calc(double dLon, double dLat) {
    double dV = 0;
    double dAlt = m_pQMR->getDValue(dLon, m_dFactor*dLat);
    if (dAlt < m_dMax) {
        dV = 1  -  dAlt/m_dMax;
    }
 
    return dV;
}
