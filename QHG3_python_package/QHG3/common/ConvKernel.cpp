#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "ConvKernel.h"

//-------------------------------------------------------------------------
// constructor
//
ConvKernel::ConvKernel(size iW, size iH) 
    : m_iW(iW),
      m_iH(iH),
      m_aadValues(NULL) {
    
    m_aadValues = new double*[m_iH];
    for (int i = 0; i < m_iH; ++i) {
        m_aadValues[i] = new double[m_iW];
    }
    m_bDeleteData = true;
}


//-------------------------------------------------------------------------
// constructor
//
ConvKernel::ConvKernel(size iW, size iH, double **aadValues, bool bDeleteOnExit)
    : m_iW(iW),
      m_iH(iH),
      m_aadValues(aadValues) {
    
    normalize();
    m_bDeleteData = bDeleteOnExit;
}

//-------------------------------------------------------------------------
// destructor
//
ConvKernel::~ConvKernel() {
    deleteArray();
}

//-------------------------------------------------------------------------
// set
//
void ConvKernel::set(size iW, size iH, double **aadValues, bool bDeleteOnExit) {
    deleteArray();
    m_iW = iW;
    m_iH = iH;
    m_aadValues = aadValues;
    normalize();
    m_bDeleteData = bDeleteOnExit;
    
}


//-------------------------------------------------------------------------
// clear
//
void ConvKernel::clear() {
    for (int i = 0; i < m_iH; ++i) {
        if (m_aadValues[i] != NULL) {
            memset(m_aadValues[i], 0, m_iW*sizeof(double));   
        }
    }
}

//-------------------------------------------------------------------------
// setElement
//
void ConvKernel::setElement(coord iX, coord iY, double dValue) {
    if ((iX < m_iW) && (iY < m_iH) && (m_aadValues != NULL)) {
        m_aadValues[iY][iX] = dValue;
    }
}


//-------------------------------------------------------------------------
// uniformRect
//
ConvKernel *ConvKernel::uniformRect(size iW, size iH) {
    double **aadValues = new double*[iH];
    for (int i = 0; i < iH; ++i) {
        aadValues[i] = new double[iW];
        for (int j = 0; j < iW; ++j) {
            aadValues[i][j] = 1.0/(iW*iH);
        }
    }    
    ConvKernel *pK = new ConvKernel(iW, iH, aadValues, true);
    return pK;
}    
    


//-------------------------------------------------------------------------
// uniformCirc
//
ConvKernel *ConvKernel::uniformCirc(size iW, size iH) {
    double dSum = 0;
    double dRX = iW/2;
    double dRY = iH/2;
    double **aadValues = new double*[iH];
    for (int j = 0; j < iH; ++j) {
        aadValues[j] = new double[iW];
        for (int i = 0; i < iW; ++i) {
            double d = sqrt(((dRX-i)*(dRX-i))/(dRX*dRX)+((dRY-j)*(dRY-j))/(dRY*dRY));
            if (d <= 1) {
                aadValues[j][i] = 1;
                dSum += aadValues[j][i];
            }
        }
    }    
    ConvKernel *pK = new ConvKernel(iW, iH, aadValues, true);
    if (dSum != 0) {
        pK->scale(1/dSum);
    }
    return pK;
}

//-------------------------------------------------------------------------
// pyramidal
//
ConvKernel *ConvKernel::pyramidal(size iW, size iH, double dAlt) {
    double dW2 = iW/2.0-0.5;
    double dH2 = iH/2.0-0.5;

    double dX = dAlt/(floor(dW2)+1);
    double dY = dAlt/(floor(dH2)+1);
    double d = (dX>dY)?dX:dY;
    //    printf("Delta : %f\n", d);
    double dSum = 0;
    double **aadValues = new double*[iH];
    for (int j = 0; j < iH; ++j) {
        aadValues[j] = new double[iW];
        double dM = (1+dH2-fabs(dH2-j))*d;
        //        printf("M:%f      ", dM);
        for (int i = 0; i < iW; ++i) {
            double dK1 = (dW2 - fabs(dW2-i) + 1)*d;
            double dK = (dK1<=dM)?dK1:dM;
            aadValues[j][i] = dK;
            dSum += aadValues[j][i];
            //            printf("%f ", dK);
        }
        //        printf("\n");
    }    
    ConvKernel *pK = new ConvKernel(iW, iH, aadValues, true);
    if (dSum != 0) {
        pK->scale(1/dSum);
    }
    return pK;
}

//-------------------------------------------------------------------------
// conical
//
ConvKernel *ConvKernel::conical(size iW, size iH, double dAlt) {
    double dRX = (iW)/2.0+0.5;
    double dRY = (iH)/2.0+0.5;

    double dSum = 0;
    double **aadValues = new double*[iH];
    for (int j = 1; j <= iH; ++j) {
        aadValues[j-1] = new double[iW];
        double dY = (j - dRY)/dRY;
        for (int i = 1; i <= iW; ++i) {
            double dX = (i-dRX)/dRX;
            double d = sqrt(dX*dX+dY*dY);
            double dV = dAlt*(1-d);
            if (dV < 0) {
                dV = 0;
            }
            aadValues[j-1][i-1] = dV;
            dSum += aadValues[j-1][i-1];
            //            printf("%f ", dV);
            
        }
        //printf("\n");
    }    
    ConvKernel *pK = new ConvKernel(iW, iH, aadValues, true);
    if (dSum != 0) {
        pK->scale(1/dSum);
    }
    return pK;
}

//-------------------------------------------------------------------------
// gaussian
//
ConvKernel *ConvKernel::gaussian(size iW, size iH, double dSigma) {
    double dRX = (iW)/2.0+0.5;
    double dRY = (iH)/2.0+0.5;

    double dSum = 0;
    double **aadValues = new double*[iH];
    for (int j = 1; j <= iH; ++j) {
        aadValues[j-1] = new double[iW];
        double dY = (j - dRY)/dRY;
        for (int i = 1; i <= iW; ++i) {
            double dX = (i-dRX)/dRX;
            double d = sqrt(dX*dX+dY*dY);

            double dP = d/dSigma;
            double dV = exp(-dP*dP/2)/sqrt(2*M_PI*dSigma);

            if (dV < 0) {
                dV = 0;
            }
            aadValues[j-1][i-1] = dV;
            dSum += aadValues[j-1][i-1];
            //            printf("%f ", dV);
            
        }
        //printf("\n");
    }    
    ConvKernel *pK = new ConvKernel(iW, iH, aadValues, true);
    if (dSum != 0) {
        pK->scale(1/dSum);
    }
    return pK;

}

//-------------------------------------------------------------------------
// createKernel
//
ConvKernel *ConvKernel::createKernel(int iType,  size iW, size iH, double dParam) {
    ConvKernel *pCK = NULL;
    switch (iType) {
    case KERNEL_URECT:
        pCK = uniformRect(iW, iH);
        break;
    case KERNEL_UCIRC:
        pCK = uniformCirc(iW, iH);
        break;
    case KERNEL_PYRA:
        pCK = pyramidal(iW, iH, dParam);
        break;
    case KERNEL_CONIC:
        pCK = conical(iW, iH, dParam);
        break;
    case KERNEL_GAUSS:
        pCK = gaussian(iW, iH, dParam);
        break;
    }
    return pCK;
}



//-------------------------------------------------------------------------
// normalize
//
void ConvKernel::normalize() {
    double dV = sum();
    if (dV != 0) {
        scale(1/dV);
    }
}

//-------------------------------------------------------------------------
// sum
//
double ConvKernel::sum() {
    double dSum = 0;
    for (int j = 0; j < m_iH; ++j) {
        for (int i = 0; i < m_iW; ++i) {
            dSum += m_aadValues[j][i];
        }
    }
    return dSum;
}    
    
//-------------------------------------------------------------------------
// scale
//
void ConvKernel::scale(double dS) {
    for (int j = 0; j < m_iH; ++j) {
        for (int i = 0; i < m_iW; ++i) {
            m_aadValues[j][i] *= dS;
        }
    }
}

//-------------------------------------------------------------------------
// deleteArray
//
void ConvKernel::deleteArray() {
    if (m_bDeleteData) {
        if (m_aadValues != NULL) {
            for (int i = 0; i < m_iH; ++i) {
                if (m_aadValues[i] != NULL) {
                    delete[] m_aadValues[i];
                }
            }
            delete[] m_aadValues;
            m_aadValues = NULL;
        }
    }
}


//-------------------------------------------------------------------------
// display
//
void ConvKernel::display() {
    for (int j = 0; j < m_iH; ++j) {
        for (int i = 0; i < m_iW; ++i) {
            printf("%f  ", m_aadValues[j][i]);
        }
        printf("\n");
    }
}
