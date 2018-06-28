#include <math.h>
#include <values.h>
#include "utils.h"
#include "GeoInfo.h"
#include "LCCProjector.h"



//----------------------------------------------------------------------------
// constructor
//
LambertConformalConicalProjector:: LambertConformalConicalProjector(double dLambda0, double dPhi0, double dPhi1/*=0*/, double dPhi2/*=M_PI/6*/)
    : Projector(PR_LAMBERT_CONFORMAL_CONIC,
                GeoInfo::getName(PR_LAMBERT_CONFORMAL_CONIC), 
                dLambda0, 
                dPhi0),
      m_dPhi1(dPhi1),
      m_dPhi2(dPhi2) {
		
    calcConsts();
}

//----------------------------------------------------------------------------
// sphere2Plane
//
void LambertConformalConicalProjector::sphere2Plane(double dLambda, 
                                                    double dPhi, 
                                                    double &dX, 
                                                    double &dY) {

    double dRho    = m_dF/pow(tan(M_PI/4+dPhi/2), m_dN);
    dX      = dRho*sin(m_dN *(dLambda - m_dLambda0));
    dY      = m_dRho0 - dRho*cos(m_dN*(dLambda - m_dLambda0));
	
}

//----------------------------------------------------------------------------
// plane2Sphere
//
void LambertConformalConicalProjector::plane2Sphere(double dX,
                                                    double dY,
                                                    double &dLambda,
                                                    double &dPhi) {
                                  
    double dRho = jsignum(m_dN)*sqrt(dX*dX+(m_dRho0-dY)*(m_dRho0-dY));
    double dTheta = atan2(m_dRho0-dY, dX);
    
    dPhi = 2*atan(pow(m_dF/dRho, 1/m_dN)) - M_PI/2;
    dLambda = m_dLambda0+dTheta/m_dN;
}

//----------------------------------------------------------------------------
// calcConsts
//
void LambertConformalConicalProjector::calcConsts() {
    double dN1 = log(cos(m_dPhi1)/cos(m_dPhi2));
    double dN2 = log(tan(M_PI/4+m_dPhi2/2)/tan(M_PI/4+m_dPhi1/2));

    if (dN2 != 0) {
        m_dN = dN1/dN2;
    } else {
        // i.e. dPhi1 == dPhi2 (or dPhi1 == M_PI/2, but this is not probable)
        if (m_dPhi1 == m_dPhi2) {
            m_dN = sin(m_dPhi1);
        } else {
            m_dN = MAXDOUBLE;
        }

    }
    m_dF = pow(tan(M_PI/4+m_dPhi1/2), m_dN)*cos(m_dPhi1)/m_dN;
    m_dRho0 = m_dF/pow(tan(M_PI/4+m_dPhi0/2), m_dN);

}

//----------------------------------------------------------------------------
// setAdditional
//
void LambertConformalConicalProjector::setAdditional(int iNumAdd, const double *pdAdd) {
    if (iNumAdd >1) {
        setStandardLatitudes(pdAdd[0], pdAdd[1]);
    }
}

//----------------------------------------------------------------------------
// setStandardLatitudes
//
void LambertConformalConicalProjector::setStandardLatitudes(double dPhi1, double dPhi2) {
    m_dPhi1 = dPhi1;
    m_dPhi2 = dPhi2;

    calcConsts();
}
