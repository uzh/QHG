#include "math.h"

#include <omp.h>

#include "utils.h"
#include "WELL512.h"
#include "NPPCalc.h"

#define SQR3 1.73205


//----------------------------------------------------------------------------
// constructor
//
NPPCalc::NPPCalc(WELL512 **apWELL) 
    : m_apWELL(apWELL) {

}


//----------------------------------------------------------------------------
// calcVariance
//  calculate a variance (see Vegetation)
//
double NPPCalc::calcVariance(double dVariance) {
    double dC = 0;
    int iThread = omp_get_thread_num();

    if ((dVariance != 0) && (m_apWELL[iThread] != NULL)) {

        double dU = 1;
        double dV = 1;
        double dS = dU * dU + dV * dV;
        while ( dS >= 1 ) {			
            dU = 1. - 2. * m_apWELL[iThread]->wrandd();
            dV = 1. - 2. * m_apWELL[iThread]->wrandd();
            dS = dU * dU + dV * dV;
        }
        double dX = dU * sqrt(-2.*log(dS)/dS);
        
        // m_adANPP[iSpecies][i] += dX * 90; 
        
        // see variance of uniform distribution
        dC =  (1.0 + SQR3 * dVariance * dX);
    }
    return dC;
}


