#include "nrutil.h"
#include "Interpolator.h"
#include "Interpolator_nr.h"
#include "types.h"
#include "BinSearch.h"

#include <math.h>

//-----------------------------------------------------------------------------
// constructor
//   prepare vectors and matrixes, calculate some values for bicubic spline
//
Interpolator::Interpolator(double *pdXGrid, int iNX, 
                           double *pdYGrid, int iNY, 
                           double **ppdVals) 
    : m_iNX(iNX),
      m_iNY(iNY) {

 
    // create double** with pointers to ppdVals' rows
    // for use as NR-style matrix
    m_matVals  = new double *[m_iNY+1];


    for (int i = 1; i <= m_iNY; i++) {
        m_matVals[i] =
            ppdVals[i-1]-1;
    } 

    // create NR-style vectors from original vectors
    m_vecXGrid = pdXGrid-1;
    m_vecYGrid = pdYGrid-1; 
  

    // NR-style matrix for internal use
    m_matGrad  = dmatrix(1,m_iNY,1,m_iNX);


    // calculate matrix of values needed for splin2
    splie2(m_vecYGrid, m_vecXGrid, m_matVals, m_iNY, m_iNX, m_matGrad);

}

//-----------------------------------------------------------------------------
// bilin
//   calculate the bilinear interpolation for a value at coordinates dX,dY
//
double Interpolator::bilin(double dX, double dY) {
    double dT;
    double dU;
    int iXL;

    
    if (dX >= m_vecXGrid[m_iNX]) {
        iXL = m_iNX-1; // one grid point before the last
    } else if (dX < m_vecXGrid[1]) {
        iXL = 1;
    } else {
      iXL = bs(dX, m_vecXGrid+1, m_iNX)+1;
    }
    dT = (dX - m_vecXGrid[iXL])/(m_vecXGrid[iXL+1] - m_vecXGrid[iXL]);
    //if (bVerbose)    printf("[bilin] MaxX %f[%d], minX %f[1], X %f[%d] -> dT %f\n", m_vecXGrid[m_iNX], m_iNX, m_vecXGrid[1], dX, iXL, dT);

    int iYL;
    if (dY >= m_vecYGrid[m_iNY]) {
        iYL = m_iNY-1;  // one grid point before the last
    } else if (dY < m_vecYGrid[1]) {
        iYL = 1;
    } else {
        iYL = bs(dY, m_vecYGrid+1, m_iNY)+1;    
    }
    //    fprintf(stderr, "[bilin] MaxY %f[%d], minY %f[1], Y %f[%d] : ", m_vecYGrid[m_iNY], m_iNY, m_vecYGrid[1], dY, iYL);
    dU = (dY - m_vecYGrid[iYL])/(m_vecYGrid[iYL+1] - m_vecYGrid[iYL]);
    //    fprintf(stderr, " dU %f\n", dU);
    //if (bVerbose)   printf("[bilin] MaxY %f[%d], minY %f[1], Y %f[%d] -> dU %f\n", m_vecYGrid[m_iNY], m_iNY, m_vecYGrid[1], dY, iYL, dU);

    //
    int iXP = (iXL<m_iNX)?iXL+1:iXL;
    int iYP = (iYL<m_iNY)?iYL+1:iYL;
    //if (bVerbose)   printf("[bilin] iXP %d, iYP %d\n", iXP, iYP);

    /*
     if (bVerbose) {    
    printf("m_matVals[%d][%d]:%f\n", iYL,iXL, m_matVals[iYL][iXL]);
    printf("m_matVals[%d][%d]:%f\n", iYL,iXP, m_matVals[iYL][iXP]);
    printf("m_matVals[%d][%d]:%f\n", iYP,iXP, m_matVals[iYP][iXP]);
    printf("m_matVals[%d][%d]:%f\n", iYP,iXL, m_matVals[iYP][iXL]);
    printf("tot:%f\n",(1-dT)*(1-dU) * m_matVals[iYL][iXL]+
           dT    *(1-dU) * m_matVals[iYL][iXP]+
              dT *   dU  * m_matVals[iYP][iXP]+
           (1-dT)*   dU  * m_matVals[iYP][iXL]); 
     }
    */
    return (1-dT)*(1-dU) * m_matVals[iYL][iXL]+
           dT    *(1-dU) * m_matVals[iYL][iXP]+
              dT *   dU  * m_matVals[iYP][iXP]+
           (1-dT)*   dU  * m_matVals[iYP][iXL]; 


}

//-----------------------------------------------------------------------------
// bipol
//   calculate the bilinear interpolation for a value at coordinates dX,dY
//
double Interpolator::bipol(double dX, double dY) {
    double dv;
    double dd;
    polin2(m_vecYGrid, m_vecXGrid, m_matVals, m_iNY, m_iNX, dY, dX, &dv, &dd);
    return dv;
}

//-----------------------------------------------------------------------------
// bicub
//   calculate the bicubic spline interpolation for a value at coordinates dX,dY
//
double Interpolator::bicub(double dX, double dY) {
    double dv;
    splin2(m_vecYGrid, m_vecXGrid, m_matVals, m_matGrad, m_iNY, m_iNX, dY, dX, &dv);
    return dv;
}



//-----------------------------------------------------------------------------
// destructor
//   clean up allocated stuff
//
Interpolator::~Interpolator() {
    delete[] m_matVals;
    // free NR-style matrix
    free_dmatrix(m_matGrad,1,m_iNX,1,m_iNY); 
}
