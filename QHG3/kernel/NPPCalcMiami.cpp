#include "math.h"

#include <omp.h>

#include "utils.h"
#include "WELL512.h"
#include "NPPCalc.h"
#include "NPPCalcMiami.h"

// DM to C between 0.5 for wood and 0.45 for foliage etc
//  https://daac.ornl.gov/NPP/guides/NPP_GPPDI.html
#define GDM_TO_KGC  0.000475


//----------------------------------------------------------------------------
// constructor
//
NPPCalcMiami::NPPCalcMiami(WELL512 **apWELL) 
    : NPPCalc(apWELL) {
}

//----------------------------------------------------------------------------
// calcNPP
//   npp in kgC/ym2
//
double NPPCalcMiami::calcNPP(double *dT, double *dP, int iNumCells, int iStride, double *dOut, double *adVariance) {
    double dMaxVal = dNegInf;
#ifdef OMP_A
#pragma omp parallel for reduction(max : dMaxVal)
#endif
    for (int i = 0; i < iNumCells; i++) {
        double nppT = 3000.0 / (1 + exp(1.315 - 0.119*dT[i]));
        double nppP = 3000.0 * (1 - exp(-0.000664*dP[i]));
        double npp  = GDM_TO_KGC*((nppT < nppP)?nppT:nppP);
        dOut[i*iStride] = npp;
        if (npp > dMaxVal) {
            dMaxVal = npp;
        }
    }  
    return dMaxVal;
}

    
//----------------------------------------------------------------------------
// calcNPP
//  npp in gC/ym2
//
double NPPCalcMiami::calcNPP(double dT, double dP, double dVariance) {
    double nppT = 3000.0 / (1 + exp(1.315 - 0.119*dT));
    double nppP = 3000.0 / (1 - exp(0.000664*dP));
    return GDM_TO_KGC*((nppT < nppP)?nppT:nppP);
}
