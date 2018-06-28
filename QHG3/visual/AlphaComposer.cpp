#include <stdio.h>
#include <string.h>

#include "types.h"
#include "LookUp.h"
#include "PNGImage.h"

#include "AlphaComposer.h"

//----------------------------------------------------------------------------
// createInstance
//
AlphaComposer *AlphaComposer::createInstance(int iW, int iH) {
    AlphaComposer *pAC = new AlphaComposer();
    int iResult = pAC->init(iW, iH);
    if (iResult != 0) {
        delete pAC;
        pAC = NULL;
    }
    return pAC;
}

//----------------------------------------------------------------------------
// constructor
//
AlphaComposer::AlphaComposer() 
    : m_iW(0),
      m_iH(0),
      m_ppPNGData(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
AlphaComposer::~AlphaComposer() {
    if (m_ppPNGData != NULL) {
        if (m_ppPNGData != NULL) {
            for (int i = 0; i < m_iH; i++) {
                delete[] m_ppPNGData[i];
            }
            delete[] m_ppPNGData;
        }
    }
}


//----------------------------------------------------------------------------
// init
//
int AlphaComposer::init(int iW, int iH) {
    int iResult = 0;

    m_iW = iW;
    m_iH = iH;

    // allocate
    m_ppPNGData = new uchar*[m_iH];
    for (int i = 0; i < m_iH; i++) {
        m_ppPNGData[i] = new uchar[4*m_iW];
        memset(m_ppPNGData[i], 0, 4*m_iW*sizeof(uchar));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// clear
//
void AlphaComposer::clear() {
    for (int i = 0; i < m_iH; i++) {
        memset(m_ppPNGData[i], 0, 4*m_iW*sizeof(uchar));
    }
}



//----------------------------------------------------------------------------
// addMatrix
//
uchar **AlphaComposer::addMatrix(double **ppdData, LookUp *pLU) {

    for (int i = 0; i < m_iH; i++) {
        uchar *pData = m_ppPNGData[i];
        for (int j = 0; j < m_iW; j++) {

            double v = ppdData[i][j];
            double dR;
            double dG;
            double dB;
            double dA;

            pLU->getColor(v, dR, dG, dB, dA);

            uchar *p0 = pData;
            // pData contains uchar colors, but for the calculation we need doubles
            double  dR0 = (*p0++)/255.0;
            double  dG0 = (*p0++)/255.0;
            double  dB0 = (*p0++)/255.0;
            double  dA0 = (*p0++)/255.0;
             
            double dA1 = dA + dA0*(1-dA);
            dR = (dR*dA + dR0*dA0*(1-dA))/dA1;
            dG = (dG*dA + dG0*dA0*(1-dA))/dA1;
            dB = (dB*dA + dB0*dA0*(1-dA))/dA1;
            dA = dA1;

            *pData++ = (uchar)(255*dR);
            *pData++ = (uchar)(255*dG);
            *pData++ = (uchar)(255*dB);
            *pData++ = (uchar)(255*dA);

        }
    }
    return m_ppPNGData;
}

 
//----------------------------------------------------------------------------
// addPNGData
//
uchar **AlphaComposer::addPNGData(uchar **ppdData) {
    for (int i = 0; i < m_iH; i++) {
        uchar *pData = m_ppPNGData[i];
        uchar *p1    = ppdData[i];

        for (int j = 0; j < m_iW; j++) {

            uchar *p0 = pData;
            // pData contains uchar colors, but for the calculation we need doubles
            double  dR0 = (*p0++)/255.0;
            double  dG0 = (*p0++)/255.0;
            double  dB0 = (*p0++)/255.0;
            double  dA0 = (*p0++)/255.0;

            double  dR1 = (*p1++)/255.0;
            double  dG1 = (*p1++)/255.0;
            double  dB1 = (*p1++)/255.0;
            double  dA1 = (*p1++)/255.0;

            double dAT = dA1 + dA0*(1-dA1);
            dR1 = (dR1*dA1 + dR0*dA0*(1-dA1))/dAT;
            dG1 = (dG1*dA1 + dG0*dA0*(1-dA1))/dAT;
            dB1 = (dB1*dA1 + dB0*dA0*(1-dA1))/dAT;
            dA1 = dAT;
 
            *pData++ = (uchar)(255*dR1);
            *pData++ = (uchar)(255*dG1);
            *pData++ = (uchar)(255*dB1);
            *pData++ = (uchar)(255*dA1);
            
        }
    }
    return m_ppPNGData;
}


