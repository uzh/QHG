#include <string.h>
#include "ConvKernel.h"
#include "BinConvolver.h"
#include "utils.h"
#include <map>

//-------------------------------------------------------------------------
// constructor
//
BinConvolver::BinConvolver(ConvKernel *pKernel) 
    : m_iW(0),
      m_iH(0),
      m_pOrigKernel(pKernel),
      m_pMaskKernel(NULL),
      m_pRowBuf(NULL),
      m_pRows(NULL),
      m_iIns(0),
      m_dBackground(dNaN),
      m_pData(NULL),
      m_pResult(NULL)
{

    m_iKH = m_pOrigKernel->m_iH;
    m_iKW = m_pOrigKernel->m_iW;

    m_pMaskKernel = new ConvKernel(m_iKW, m_iKH);
    
}


//-------------------------------------------------------------------------
// destructor
//
BinConvolver::~BinConvolver() {
    if (m_bDeleteData && (m_pData != NULL)) {
        for (int i = 0; i < m_iH; ++i) {
            delete[] m_pData[i];
        }
        delete[] m_pData;
    }

    if  (m_bDeleteResult &&(m_pResult != NULL)) {
        for (int i = 0; i < m_iH; ++i) {
            delete[] m_pResult[i];
        }
        delete[] m_pResult;
    }
    
    if (m_pRowBuf != NULL) {
        for (int i = 0; i < m_iKH; i++) {
            delete[] m_pRowBuf[i];
        }
        delete[] m_pRowBuf;
    }

    if (m_pRows != NULL) {
        delete[] m_pRows;
    }
    
    
    if (m_pMaskKernel != NULL) {
        delete m_pMaskKernel;
    }
}

//-------------------------------------------------------------------------
// convolve
//
double** BinConvolver::convolve(double **pData, int iW, int iH, int iReSamp, 
                                double dBackground,  bool bDeleteData, bool bMaj, bool bAllowNaN,
                                double **pResultBuffer) {
    
    m_bDeleteData = bDeleteData;
    m_pData = pData;

    m_iW = iW;
    m_iH = iH;
    
    m_iReSamp = iReSamp;
    m_iWR = m_iW * m_iReSamp;
    m_iHR = m_iH * m_iReSamp;

    m_dBackground = dBackground;

    if (pResultBuffer != NULL) {
        m_pResult = pResultBuffer;
        m_bDeleteResult = false;
    } else {
        // current row of convolved elements
        m_pResult = new double*[m_iHR];
        for (int j = 0; j < m_iHR; ++j) {
            m_pResult[j] = new double[m_iWR];
            memset(m_pResult[j], 0, m_iWR*sizeof(double));
        }
        m_bDeleteResult = true;
    }

    // ringbuffer of currently used rows
    m_pRowBuf = new double*[m_iKH];
    for (int j = 0; j < m_iKH; ++j) {
        m_pRowBuf[j] = new double[m_iWR];
    }

    // array of current input rows (order determined by ring buffer m_pRowBuf)
    m_pRows = new double*[m_iKH];
    for (int j = 0; j < m_iKH; ++j) {
        m_pRows[j] = NULL;
    }
    
    // to convolve the first line, only half of the kernle can be active
    for (int k = 0; k < m_iKH/2; ++k) {
        loadNextDataRow(k);
    }
    
    if (bMaj) {
        // Loop: load next input row, then convolve all elements in current row
        for (int j = 0; j < m_iHR; ++j) {
            loadNextDataRow(j+m_iKH/2);
            for (int i = 0; i < m_iWR; ++i) {
                majorityElement(j,i);
            }
        }
    } else if (bAllowNaN) {
        // Loop: load next input row, then convolve all elements in current row
        for (int j = 0; j < m_iHR; ++j) {
            loadNextDataRow(j+m_iKH/2);
            for (int i = 0; i < m_iWR; ++i) {
                convolveElementAllowNaN(j,i);
            }
        }
    } else {
        // Loop: load next input row, then convolve all elements in current row
        for (int j = 0; j < m_iHR; ++j) {
            loadNextDataRow(j+m_iKH/2);
            for (int i = 0; i < m_iWR; ++i) {
                convolveElement(j,i);
            }
        }
    }
    return m_pResult;
}

//-------------------------------------------------------------------------
// convolveElement
//
void BinConvolver::convolveElement(int iY, int iX) {
   
    int iOffsX = iX-m_iKW/2;
    //    int iOffsY = iY-m_iKH/2;
    // create kernel mask:

    prepareMask(iY, iX);
    
    //    printf("(%d,%d)  -> Offs (%d, %d)\n", iX, iY, iOffsX, iOffsY);

    double dv = 0;
    if (isnan(m_pData[iY/m_iReSamp][iX/m_iReSamp]) && isnan(m_dBackground)) {
        dv = dNaN;
    } else {
        for (int j = 0; j < m_iKH; ++j) {
            double *pRow = m_pRows[j];

            for (int i = 0; i < m_iKW; ++i) {
                double dMask = m_pMaskKernel->m_aadValues[j][i];
                if (dMask > 0) {
                    double dCur = pRow[iOffsX+i];
                    if (isnan(dCur)) {
                        if (!isnan(m_dBackground)){
                            dCur = m_dBackground;
                        } else {
                            printf("have isnan\n");
                        }
                    }
                    dv += dMask * dCur;
                }
            }
        }
    }
        
    m_pResult[iY][iX] = dv;

  
}

//-------------------------------------------------------------------------
// convolveElementAllowNaN
//
void BinConvolver::convolveElementAllowNaN(int iY, int iX) {
   
    int iOffsX = iX-m_iKW/2;
    //    int iOffsY = iY-m_iKH/2;
    // create kernel mask:

    prepareMask(iY, iX);
    
    //    printf("(%d,%d)  -> Offs (%d, %d)\n", iX, iY, iOffsX, iOffsY);

    double dv = 0;
    int iC = 0;
    for (int j = 0; j < m_iKH; ++j) {
        double *pRow = m_pRows[j];

        for (int i = 0; i < m_iKW; ++i) {
            double dMask = m_pMaskKernel->m_aadValues[j][i];
            if (dMask > 0) {
                double dCur = pRow[iOffsX+i];
                if (isnan(dCur)) {
                    if (!isnan(m_dBackground)){
                        dCur = m_dBackground;
                    } else {
                        printf("have isnan\n");
                        //dCur = 0;
                    }
                }
                if (!isnan(dCur)) {
                    dv += dMask * dCur;
                    iC++;
                }
            }
        }
    }
        
    m_pResult[iY][iX] = (iC > 0)?dv:dNaN;
}



//-------------------------------------------------------------------------
// majorityElement
//
void BinConvolver::majorityElement(int iY, int iX) {
   
    int iOffsX = iX-m_iKW/2;
    //    int iOffsY = iY-m_iKH/2;
    // create kernel mask:

    prepareMask(iY, iX);
    
    //    printf("(%d,%d)  -> Offs (%d, %d)\n", iX, iY, iOffsX, iOffsY);

    double dv = 0;
    
    if (isnan(m_pData[iY/m_iReSamp][iX/m_iReSamp]) && isnan(m_dBackground)) {
        dv = dNaN;
    } else {
        std::map<double, int> mapC;
        for (int j = 0; j < m_iKH; ++j) {
            double *pRow = m_pRows[j];

            for (int i = 0; i < m_iKW; ++i) {
                double dMask = m_pMaskKernel->m_aadValues[j][i];
                if (dMask > 0) {
                    double dCur = pRow[iOffsX+i];
                    if (isnan(dCur)) {
                        if (!isnan(m_dBackground)){
                            dCur = m_dBackground;
                        } else {
                            printf("have isnan\n");
                        }
                    }
                    mapC[dCur] += 1;
                }
            }
        }
        int iMax = 0;
        for (std::map<double, int>::iterator it = mapC.begin(); it!= mapC.end(); it++) {
            if (it->second == 3) {
                dv = 3;
                break;
            } else if (it->second >= iMax) {
                iMax = it->second;
                dv = it->first;
            }
        }
        
    }
    m_pResult[iY][iX] = dv;
 
}

//-------------------------------------------------------------------------
// prepareMask
//
void BinConvolver::prepareMask(int iY, int iX) {
    
    int iOffsX = iX-m_iKW/2;
    int iOffsY = iY-m_iKH/2;
    // create kernel mask:
    double dSum = 0;

    
    //    printf("(%d,%d)  -> Offs (%d, %d)\n", iX, iY, iOffsX, iOffsY);
    // same size as kernel, with 1 at legal positions, 0 at invalid positions
        
    m_pMaskKernel->clear();
    for (int j = 0; j < m_iKH; ++j) {
        if ((iOffsY+j >= 0) && (iOffsY+j < m_iHR)) {
            for (int i = 0; i < m_iKW; ++i) {
                if ((iOffsX+i >= 0) && (iOffsX+i < m_iWR)) {
                    // element is inside: check if we have null elements (NaN etc)
                    //                    if (!isnan(m_pData[(iOffsY+j)/m_iReSamp][(iOffsX+i)/m_iReSamp])) {
                    if (!isnan(m_pRows[j][(iOffsX+i)])) {
                        m_pMaskKernel->setElement(i, j, m_pOrigKernel->m_aadValues[j][i]);
                        dSum += m_pOrigKernel->m_aadValues[j][i];

                    } else {
                        if (isnan(m_dBackground)) {
                            m_pMaskKernel->setElement(i, j, 0);
                        } else {
                            //printf("Adding background at (%d,%d)\n", i, j);
                            m_pMaskKernel->setElement(i, j, m_dBackground);
                            dSum += m_dBackground;
                        }
                    }
                    
                }
            }
        }
    }
    if (dSum != 0) {
        m_pMaskKernel->scale(1/dSum);
    }
      

    
}


//-------------------------------------------------------------------------
// loadNextDataRow
//
void BinConvolver::loadNextDataRow(int iRow) {
    
    // shift all rows
    for (int i = 0; i < m_iKH-1; ++i) {
        m_pRows[i] = m_pRows[i+1];
    }
    

    iRow /= m_iReSamp;
    if ((iRow >= 0) && (iRow < m_iH)) {
        for (int k = 0; k < m_iWR; ++k) {
            m_pRowBuf[m_iIns][k] = m_pData[iRow][k/m_iReSamp];
        }
        m_pRows[m_iKH-1] = m_pRowBuf[m_iIns];
    } else {
        m_pRows[m_iKH-1] = NULL;
    }
    m_iIns++;
    if (m_iIns >= m_iKH) {
        m_iIns = 0;
    }
    
}    
