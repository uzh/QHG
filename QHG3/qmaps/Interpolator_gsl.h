#ifndef __INTERPOLATOR_GSL_H__
#define __INTERPOLATOR_GSL_H__

#include <gsl/gsl_spline2d.h>

#define INTERP_BILIN 0
#define INTERP_BICUB 1

class Interpolator_gsl {
public:
    Interpolator_gsl(int iType, 
                     double *pdXGrid, int iNX, 
                     double *pdYGrid, int iNY, 
                     double **ppdVals);

    virtual ~Interpolator_gsl();

    double getValue(double dX, double dY);

private:
    int      m_iNX;
    int      m_iNY;

    double m_dMinX;
    double m_dMaxX;
    double m_dMinY;
    double m_dMaxY;

    double *m_pLinearVals;
    gsl_spline2d *m_pSpline;
    gsl_interp_accel *m_pXacc;
    gsl_interp_accel *m_pYacc;

};

#endif

