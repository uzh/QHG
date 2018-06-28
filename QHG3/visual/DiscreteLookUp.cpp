#include <stdio.h>
#include <math.h>
#include "DiscreteLookUp.h"
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
DiscreteLookUp::DiscreteLookUp(int iNumCols) 
    :   LookUp(0,1.0*iNumCols),
        m_iNumCols(iNumCols+1) {
    
    
    m_aadCols = new double*[m_iNumCols];
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
DiscreteLookUp::~DiscreteLookUp() {
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
void DiscreteLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    int iVal = (int)round(dValue);
    //  printf("dVal %f, iVal:%d\n", dValue, iVal);
    if (iVal < 0 ) {
        iVal = 0;
    } else if (iVal >= m_iNumCols) {
        iVal = m_iNumCols-1;
    }

    dRed   = m_aadCols[iVal][0];
    dGreen = m_aadCols[iVal][1];
    dBlue  = m_aadCols[iVal][2];
    dAlpha = m_aadCols[iVal][3];
}
