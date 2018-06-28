#include <math.h>

#include "LookUp.h"
#include "ValReader.h"
#include "VRColorizer.h"


VRColorizer::VRColorizer(ValReader *pVR)
    : IcoColorizer(),
      m_pVR(pVR) {

}

void VRColorizer::getCol(double dLon, double dLat, float fCol[4]) {
    if (m_bUseColor && (m_pVR != NULL)) {
        double dVal = m_pVR->getDValue(dLon*180/M_PI, dLat*180/M_PI);

        double dCol[4];
        m_pLookUp->getColor(dVal, dCol[0],dCol[1],dCol[2],dCol[3]);
        // printf("-> %f, %f (%f, %f)-> %f (%f,%f,%f)\n", theta, phi, theta*180/M_PI, phi*180/M_PI, dVal, dCol[0], dCol[1], dCol[2]);
        fCol[0] = dCol[0];
        fCol[1] = dCol[1];
        fCol[2] = dCol[2];
        fCol[3] = dCol[3];
       
    } else {
        fCol[0] = 0;
        fCol[1] = 0;
        fCol[2] = 0;
        fCol[3] = 1;
    }


}
