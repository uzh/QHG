#include <stdio.h>
#include <math.h>


#include "LookUp.h"
#include "SnapColorizer.h"


SnapColorizer::SnapColorizer()
    : IQColorizer() {

}

void SnapColorizer::getCol(double dVal, float fCol[4]) {
    if (m_bUseColor && (m_pLookUp != NULL)) {
        /*
        double dVal = dNaN; 
        nodelist::const_iterator it = m_pPI->getNodeList().find(lNode);
        if (it !=  m_pPI->getNodeList().end()) {
            dVal = it->second;
        }
        
        if (!isnan(dVal) && bUseAlt) {
                *pfScale = 1+m_fAltFactor*(dVal - m_pLookUp->m_dMinLevel)/(m_pLookUp->m_dMaxLevel-m_pLookUp->m_dMinLevel);
        } else {
            *pfScale =1;
        }
        */

        double dCol[4];

        //        printf("m_pLookUp is %p\n", m_pLookUp);
        m_pLookUp->getColor(dVal, dCol[0],dCol[1],dCol[2],dCol[3]);
        //         printf("-> %lld-> %f (%f,%f,%f)\n", lNode, dVal, dCol[0], dCol[1], dCol[2]);
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
