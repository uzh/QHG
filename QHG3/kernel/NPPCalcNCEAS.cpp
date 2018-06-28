#include "math.h"

#include <omp.h>

#include "utils.h"
#include "WELL512.h"
#include "NPPCalc.h"
#include "NPPCalcNCEAS.h"

#define SQR3 1.73205


//----------------------------------------------------------------------------
// constructor
//
NPPCalcNCEAS::NPPCalcNCEAS(WELL512 **apWELL, double *adWeights) 
    : NPPCalc(apWELL),
      m_pdWeights(NULL),
      m_bDeleteWeights(false) {

    if (adWeights != NULL) {
        m_pdWeights = adWeights;
    } else {
        m_pdWeights = new double[3];
        m_pdWeights[0] = 1;
        m_pdWeights[1] = 1;
        m_pdWeights[2] = 1;
        m_bDeleteWeights = true;
    }
}

//----------------------------------------------------------------------------
// destructor
//
NPPCalcNCEAS::~NPPCalcNCEAS() {
    if (m_bDeleteWeights) {
        delete[] m_pdWeights;
    }
}


//----------------------------------------------------------------------------
// calcNPP
//
double NPPCalcNCEAS::calcNPP(double *dT, double *dP, int iNumCells, int iStride, double *dOut, double *adVariance) {
    double dMaxVal = 0;
    dMaxVal += m_pdWeights[0]*calcGrassNPP(dT, dP, iNumCells, iStride,  dOut, adVariance);
    dMaxVal += m_pdWeights[1]*calcBushNPP(dT, dP, iNumCells, iStride,  dOut, adVariance);
    dMaxVal += m_pdWeights[2]*calcTreeNPP(dT, dP, iNumCells, iStride,  dOut, adVariance);
 
    return dMaxVal;
}

    
//----------------------------------------------------------------------------
// calcNPP
//
double NPPCalcNCEAS::calcNPP(double dT, double dP, double dVariance) {
    double dVal = 0;
    dVal += m_pdWeights[0]*calcGrassNPP(dT, dP, dVariance);
    dVal += m_pdWeights[1]*calcBushNPP(dT, dP, dVariance);
    dVal += m_pdWeights[2]*calcTreeNPP(dT, dP, dVariance);
 
    return dVal;
}


//----------------------------------------------------------------------------
// calcTempCorr
//  Simone's temperature modulation:
//  Zeros at 0 and 40, peak at 26.66, max value 1
//
double calcTempCorr(double dT) {
    double dC = 0;
    if ((dT > 0) && (dT <= 40)) {
        dC = dT*dT*(40.0-dT)/9500;
    }
    return dC;
}


//----------------------------------------------------------------------------
// calcGrassNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1800
//
double NPPCalcNCEAS::calcGrassNPP(double dT, double dP, double dVariance) {
    double dNPP = 10000 * (1 - exp(-4.77e-5 * dP));
    if (dVariance > 0) {
        dNPP *= calcVariance(dVariance);
    }
    if (dNPP < 0.0) {
        dNPP = 0.0;
    } else {
        dNPP *= calcTempCorr(dT);
    }
    return dNPP;
}


//----------------------------------------------------------------------------
// calcBushNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1800
//
double NPPCalcNCEAS::calcBushNPP(double dT, double dP, double dVariance) {
    double dNPP = 10000 * (1 - exp(-4.77e-5 * dP));
    if (dVariance > 0) {
        dNPP *= calcVariance(dVariance);
    }
    if (dNPP < 0.0) {
        dNPP = 0.0;
    } else {
        dNPP *= calcTempCorr(dT);
    }
    return dNPP;
}


//----------------------------------------------------------------------------
// calcTreeNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1600
//
double NPPCalcNCEAS::calcTreeNPP(double dT, double dP, double dVariance) {
    double dNPP = 0.0;
    if (dP > 0) {
        dNPP = 0.41625*exp(log(dP)*1.185)/exp(0.000414*dP);
    }
    double dNPPT = 7847.5 / (1 + exp(2.2 - 0.0307 * dT));
    if (dNPPT < dNPP) {
        dNPP = dNPPT;
    }

    if (dVariance > 0) {
        dNPP *= calcVariance(dVariance);
    }

    if (dNPP < 0.0) {
        dNPP = 0.0;
    }
    return dNPP;
}


//----------------------------------------------------------------------------
// calcGrassNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1800
//
double NPPCalcNCEAS::calcGrassNPP(double *dT, double *dP, int iNumCells, int iStride, double *dOut, double *adVariance) {
    double dMaxVal = dNegInf;
#ifdef OMP_A
#pragma omp parallel for reduction(max : dMaxVal)
#endif
    for (int i = 0; i < iNumCells; i++) {
        if (dT[i] > 0) {
            double dNPP = 10000 * (1 - exp(-4.77e-5 * dP[i]));
            if (adVariance != NULL) {
                dNPP *= calcVariance(adVariance[i]);
            }
            if (dNPP < 0.0) {
                dNPP = 0.0;
            } else {
                dNPP *= calcTempCorr(dT[i]);
            }
               
            dOut[i*iStride] += m_pdWeights[0]*dNPP;
        }
    }
    return dMaxVal;
}


//----------------------------------------------------------------------------
// calcBushNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1800
//
double NPPCalcNCEAS::calcBushNPP(double *dT, double *dP, int iNumCells, int iStride, double *dOut, double *adVariance) {
    double dMaxVal = dNegInf;
#ifdef OMP_A
#pragma omp parallel for reduction(max : dMaxVal)
#endif
    for (int i = 0; i < iNumCells; i++) {
        if (dT[i] > 0) {
            double dNPP = 10000 * (1 - exp(-4.77e-5 * dP[i]));
            if (adVariance != NULL) {
                dNPP *= calcVariance(adVariance[i]);
            }
            if (dNPP < 0.0) {
                dNPP = 0.0;
            } else {
                dNPP *= calcTempCorr(dT[i]);
            }
            dOut[i*iStride] += m_pdWeights[1]*dNPP;
        }
    }
    return dMaxVal;
}


//----------------------------------------------------------------------------
// calcTreeNPP
//  according to DelGross 2008
//  max value for t in [-10:50] and p in [0:5000]: 1600
//
double NPPCalcNCEAS::calcTreeNPP(double *dT, double *dP, int iNumCells, int iStride, double *dOut, double *adVariance) {
    double dMaxVal = dNegInf;
#ifdef OMP_A
#pragma omp parallel for reduction(max : dMaxVal)
#endif
    for (int i = 0; i < iNumCells; i++) {
        double dNPP = 0.0;
        if (dP[i] > 0) {
            dNPP = 0.41625*exp(log(dP[i])*1.185)/exp(0.000414*dP[i]);
        }
        double dNPPT = 7847.5 / (1 + exp(2.2 - 0.0307 * dT[i]));
        if (dNPPT < dNPP) {
            dNPP = dNPPT;
        }
        
        if (adVariance != NULL) {
            dNPP *= calcVariance(adVariance[i]);
        }
        
        if (dNPP < 0.0) {
            dNPP = 0.0;
        }
        dOut[i*iStride] += m_pdWeights[2]*dNPP;
    }
    return dMaxVal;
}
