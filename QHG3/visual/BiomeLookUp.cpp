#include <stdio.h>
#include <math.h>
#include "BiomeLookUp.h"
#include "types.h"

const int BASE_COLS = 6;
static double adCBasic[BASE_COLS][4] = {
    {0.00, 0.00, 0.70, 1.0}, // OCEAN 
    {0.89, 0.70, 0.31, 1.0}, // DESERT
    {0.63, 1.00, 0.28, 1.0}, // GRASSLAND 
    {0.53, 0.60, 0.06, 1.0}, // SHRUBLAND
    {0.08, 0.81, 0.63, 1.0}, // PARKLAND
    {0.08, 0.55, 0.06, 1.0}, // WOODLAND

  
};

//-----------------------------------------------------------------------------
// constructor
//
BiomeLookUp::BiomeLookUp() 
    :   LookUp(0,6.0) {
    

    m_aadCols = new double *[BASE_COLS];
    for (int i = 0; i < BASE_COLS; ++i) {
        m_aadCols[i] = new double[4];
        for (int j = 0; j < 4; ++j) {
            m_aadCols[i][j] = adCBasic[i][j];
        }
        
    }
    
}


//-----------------------------------------------------------------------------
// destructor
//
BiomeLookUp::~BiomeLookUp() {
    if (m_aadCols != NULL) {
        for (int i = 0; i < BASE_COLS; ++i) {
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
void BiomeLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {

    uchar iVal = (uchar)round(dValue);
    if (iVal == 0xff) {
        // multiple values
        dRed   = 0;
        dGreen = 0;
        dBlue  = 0;
        dAlpha = 1;
    } else if (iVal == 0xfe) {
        // no value
        dRed   = 0;
        dGreen = 0;
        dBlue  = 0;
        dAlpha = 0;
    } else if (iVal >= BASE_COLS) {
        dRed   = 1;
        dGreen = 1;
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
