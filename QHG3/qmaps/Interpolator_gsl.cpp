#include <string.h>
#include <stdio.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>

#include "Interpolator_gsl.h"

#define EPS 0.0001

//-----------------------------------------------------------------------------
// constructor
//   prepare vectors and matrixes, calculate some values for bicubic spline
//
Interpolator_gsl::Interpolator_gsl(int iType,
                                   double *pdXGrid, int iNX, 
                                   double *pdYGrid, int iNY, 
                                   double **ppdVals) 
    : m_iNX(iNX),
      m_iNY(iNY) {

    m_dMinX =pdXGrid[0]+EPS;
    m_dMaxX =pdXGrid[iNX-1]-EPS;
    m_dMinY =pdYGrid[0]+EPS;
    m_dMaxY =pdYGrid[iNY-1]-EPS;


    printf("NX,NY: %d,%d\n",m_iNX, m_iNY);
    m_pLinearVals  = new double[m_iNX*m_iNY];
    for (int i = 0; i < m_iNY; i++) {
        memcpy(m_pLinearVals+i*m_iNX, ppdVals[i], m_iNX*sizeof(double));
    }

    const gsl_interp2d_type *T = (iType==INTERP_BICUB)?gsl_interp2d_bicubic:gsl_interp2d_bilinear;
    m_pSpline = gsl_spline2d_alloc(T, m_iNX, m_iNY);
    m_pXacc   = gsl_interp_accel_alloc();
    m_pYacc   = gsl_interp_accel_alloc();
    
    gsl_spline2d_init(m_pSpline, pdXGrid, pdYGrid, m_pLinearVals, m_iNX, m_iNY);

}

//-----------------------------------------------------------------------------
// destructor
//   clean up allocated stuff
//
Interpolator_gsl::~Interpolator_gsl() {
    delete[] m_pLinearVals;

    gsl_spline2d_free(m_pSpline);
    gsl_interp_accel_free(m_pXacc);
    gsl_interp_accel_free(m_pYacc);
}

//-----------------------------------------------------------------------------
// bilin
//   calculate the bilinear interpolation for a value at coordinates dX,dY
//
double Interpolator_gsl::getValue(double dX, double dY) {
    if (dX <= m_dMinX) {
        dX = m_dMinX;
    } else if (dX >= m_dMaxX) {
        dX = m_dMaxX;
    }
    if (dY < m_dMinY) {
        dY = m_dMinY;
    } else if (dY >= m_dMaxY) {
        dY = m_dMaxY;
    }
    double dZ = gsl_spline2d_eval(m_pSpline, dX, dY, m_pXacc, m_pYacc);
    return dZ;
}
