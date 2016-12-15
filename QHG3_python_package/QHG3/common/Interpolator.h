#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__


class Interpolator {
public:
    Interpolator(double *pdXGrid, int iNX, 
                 double *pdYGrid, int iNY, 
                 double **ppdVals);

    virtual ~Interpolator();

    double bilin(double dX, double dY);
    double bipol(double dX, double dY);
    double bicub(double dX, double dY);

private:
    int      m_iNX;
    int      m_iNY;

    double **m_matVals;
    double **m_matGrad;
    double  *m_vecXGrid;
    double  *m_vecYGrid;
};

#endif

