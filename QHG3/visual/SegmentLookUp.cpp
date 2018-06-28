#include <stdio.h>
#include <math.h>
#include "SegmentLookUp.h"
#include "types.h"


//-----------------------------------------------------------------------------
// constructor
//
SegmentLookUp::SegmentLookUp(double dMin, double dMax, double dWidth, double dSubDiv) 
    :   LookUp(dMin, dMax),
        m_dWidth(dWidth),
        m_iSubDiv((int)dSubDiv),
        m_dSubDiv(0),
        m_iNumCols((int)ceil(dMax/dWidth)) {
    
    
    m_aadCols = new double *[m_iNumCols];
    for (int i = 0; i < m_iNumCols; ++i) {
        m_aadCols[i] = new double[4];
    }
    
    double dRed;
    double dGreen;
    double dBlue;
    double dAlpha;
    for (int i = 0; i < m_iNumCols; i++) {
        double dV = (i*1.0)/m_iNumCols;

        if (4*dV < 1) {
            dRed   = 0.0; 
            dGreen = 4*dV;
            dBlue  = 1.0;

        } else if (4*dV < 2) {
            dRed   = 0.0; 
            dGreen = 1.0;
            dBlue  = 1.75 - 3*dV;
            
        } else if (4*dV < 3) {
            dRed   = 3*dV - 1.25; 
            dGreen = 1.0;
            dBlue  = 0.0;

        } else {
            dRed   = 1.0;
            dGreen = 4.0-4*dV;
            dBlue  = 0.0;
        }
        
        dAlpha = 1.0; 
        m_aadCols[i][0] = dRed;
        m_aadCols[i][1] = dGreen;
        m_aadCols[i][2] = dBlue;
        m_aadCols[i][3] = dAlpha;
    }
    
    if (m_iSubDiv > 0) {
        m_dSubDiv = m_dWidth/m_iSubDiv;
    }
}


//-----------------------------------------------------------------------------
// destructor
//
SegmentLookUp::~SegmentLookUp() {
    
    for (int i = 0; i < m_iNumCols; ++i) {
        delete[] m_aadCols[i];
    }
    delete[] m_aadCols;

}

//-----------------------------------------------------------------------------
// getColor
//
void SegmentLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    if (isnan(dValue)) {
        dRed   = 1.0;
        dGreen = 1.0;
        dBlue  = 1.0;
        dAlpha = 0.0;
    } else if (isinf(dValue)) {
        dRed   = 0.0;
        dGreen = 0.0;
        dBlue  = 0.0;
        dAlpha = 0.0;
    } else {
        
        dValue -= m_dMinLevel;
        if (dValue < 0) {
            dValue = 0;
        }
        double dValue1 = floor(dValue/m_dWidth);
            
        double dMax = ceil((m_dMaxLevel - m_dMinLevel)/m_dWidth);

        int iIndex = (int)floor(m_iNumCols*dValue1/dMax);
        if (iIndex >= m_iNumCols) {
            iIndex = m_iNumCols-1;
        }
        dRed   = m_aadCols[iIndex][0];
        dGreen = m_aadCols[iIndex][1];
        dBlue  = m_aadCols[iIndex][2];
        dAlpha = m_aadCols[iIndex][3];

        if (m_iSubDiv > 0) {
            if (fmod(dValue, 2*m_dSubDiv) > m_dSubDiv) {
                dRed   *= 0.75;
                dGreen *= 0.75;
                dBlue  *= 0.75;
            }
        }
    }
}
