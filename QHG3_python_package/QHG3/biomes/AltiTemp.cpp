#include <stdio.h>
#ifdef OMP
#include <omp.h>
#endif

#include "QMapUtils.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapHeader.h"

#include "TopoTemp.h"
#include "AltiTemp.h"

//const double dTempRate = -6.379e-3; // [°C/m], temperature decrease per altitude ("lapse rate", 3.5°F/1000ft; 1 ft = 0.3048m, 1 °F = 5/9 °C)
const double dTempRate = -1.631e-3; // [°C/m], (sixth the original rate)

//------------------------------------------------------------------------------
// constructor
//
AltiTemp::AltiTemp(char *pAltFile, char *pTempFile) 
    : TopoTemp(pAltFile, pTempFile) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
AltiTemp::AltiTemp(ValReader *pVRAltitude, char *pTempFile)
    : TopoTemp(pVRAltitude, pTempFile) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
AltiTemp::AltiTemp(char *pAltFile, ValReader *pVRTemperature)
    : TopoTemp(pAltFile, pVRTemperature) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
AltiTemp::AltiTemp(ValReader *pVRAltitude, ValReader *pVRTemperature)
    : TopoTemp(pVRAltitude, pVRTemperature) {

}
//------------------------------------------------------------------------------
// destructor
//
AltiTemp::~AltiTemp() {
}

//------------------------------------------------------------------------------
// adjustTemperature
//
int AltiTemp::adjustTemperature(double fParam, bool bAccumulateTemp) {
    int iResult = -1;
    printf("Doing Alt\n");
    if (m_bReady) {
        iResult = 0;

#ifdef  OMP
#pragma omp parallel for
#endif
        for (unsigned int iY = 0; iY < m_iH; iY++) {
            double dPhi = m_pQMRAltitude->Y2Lat(iY);
            for (unsigned int iX = 0; iX < m_iW; iX++) {
                double dTheta = m_pQMRAltitude->X2Lon(iX);
                double dAltitude = m_pQMRAltitude->getDValue(iX, iY);
                if (dAltitude < fParam) {  // used to be <20
                    dAltitude = dNaN;
                }
                double dAdditional = dTempRate*dAltitude;
                    
                double dTemp0 = m_pQMRTemp->getDValue(dTheta, dPhi);
               
                if (!isfinite(dTemp0)) {
                    
                    //   printf("BadVal (%f) at %d,%d %f\n", dTemp0, iX, iY, dAltitude);
                    dTemp0=dNaN;
                } else {
                    dTemp0 += dAdditional;
                }
                m_ppData[iY][iX] = (float)(bAccumulateTemp?dTemp0:dAdditional);
            }
        }
    }

    return iResult;
}
