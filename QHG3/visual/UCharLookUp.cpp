#include <stdio.h>
#include <math.h>
#include "UCharLookUp.h"
#include "types.h"

const int BASE_COLS = 12;
static double adCBasic[BASE_COLS][4] = {
    {1.0, 0.0, 0.0, 1.0}, 
    {0.0, 1.0, 0.0, 1.0}, 
    {0.0, 0.0, 1.0, 1.0}, 
    {0.0, 1.0, 1.0, 1.0}, 
    {1.0, 0.0, 1.0, 1.0}, 
    {1.0, 1.0, 0.0, 1.0}, 

    {1.0, 0.7, 0.0, 1.0}, 
    {1.0, 0.7, 0.7, 1.0},
    {0.7, 1.0, 0.0, 1.0},
    {0.5, 0.0, 1.0, 1.0},
    {0.7, 0.7, 0.7, 1.0},
    {0.5, 1.0, 0.7, 1.0},
};

//-----------------------------------------------------------------------------
// constructor
//
UCharLookUp::UCharLookUp(int iNumCols) 
    :   LookUp(0,1.0*iNumCols),
        m_iNumCols(iNumCols+1) {
    

    m_aadCols = new double *[m_iNumCols];
    for (int i = 0; i < m_iNumCols; ++i) {
        m_aadCols[i] = new double[4];
    }

    if (m_iNumCols > 0) {
        m_aadCols[0][0] = 0.0;
        m_aadCols[0][1] = 0.0;
        m_aadCols[0][2] = 0.0;
        m_aadCols[0][3] = 1.0;

        int iNShades = 1+(m_iNumCols-1)/BASE_COLS;
        float fFraction = 1.0f/iNShades;

        bool bCont = true;
        for (int k = 0; k < iNShades; ++k) {
            float fAtt = 1-k*fFraction;
            int iB = k * BASE_COLS;
            for (int i = 0; bCont && (i < BASE_COLS); ++i) {
                int iX = iB+i+1;
                if (iX < m_iNumCols) {
                    for (int j = 0; j < 3; ++j) {
                        m_aadCols[iX][j] = adCBasic[i][j]*fAtt;
                    }
                    m_aadCols[iX][3] = 1.0;
                } else {
                    bCont = false;
                } 
            }
        }

    }
}


//-----------------------------------------------------------------------------
// destructor
//
UCharLookUp::~UCharLookUp() {
    if (m_aadCols != NULL) {
        for (int i = 0; i < m_iNumCols; ++i) {
            if (m_aadCols[i] != NULL) {
                delete[] m_aadCols[i];
            }
        }
        delete[] m_aadCols;
    }
}

//-----------------------------------------------------------------------------
// getColor
//
void UCharLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    uchar iVal = (uchar)round(dValue);
    if (iVal == 0xff) {
        // multiple values
        dRed   = 1;
        dGreen = 1;
        dBlue  = 1;
        dAlpha = 1;
    } else if (iVal == 0xfe) {
        // no value
        dRed   = 0;
        dGreen = 0;
        dBlue  = 0;
        dAlpha = 0;
    } else if (iVal >= m_iNumCols) {
        dRed   = 0.8;
        dGreen = 0.8;
        dBlue  = 1;
        dAlpha = 1;
    } else {

        dRed   = m_aadCols[iVal][0];
        dGreen = m_aadCols[iVal][1];
        dBlue  = m_aadCols[iVal][2];
        dAlpha = m_aadCols[iVal][3];
    }
    // printf("Val %d, col %f %f %f %f\n", iVal, dRed, dGreen, dBlue, dAlpha);
}
