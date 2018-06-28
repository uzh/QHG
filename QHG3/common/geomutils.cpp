/*============================================================================
| geomutils
| 
|  Some geometric utilities. Amongst others,
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include "utils.h"
#include "math.h"
#include "geomutils.h"
#include "types.h"
#include "Vec3D.h"
#include "Quat.h"

const float EPS = 0.00001f;



//----------------------------------------------------------------------------
//  isPointInPoly
//  returns value
//   false : point (X0,Y0) is outside of poly
//   true  : point inside poly
// 
bool isPointInPoly(double fX0, double fY0, 
                   int iN, 
                   double *afX, double *afY) {

    int iWindingNumber = 0;
    double fXPrev = afX[iN-1];
    double fYPrev = afY[iN-1];

    for (int i = 0; i < iN; ++i) {
        double fXCur = afX[i];
        double fYCur = afY[i];
        //printf("Doing (%f,%f)-(%f,%f)\n", fXPrev, fYPrev, fXCur, fYCur);
        if ((fXPrev >= fX0) || (fXCur >= fX0)) {
            // upwards
            if (fYPrev <= fY0)  {
                if (fYCur > fY0) {
                    // point on left : increase winding number
                    printf("U WHichside(%lf,%lf) (%lf,%lf)-(%lf,%lf) : %lf\n", fX0, fY0,fXPrev, fYPrev, fXCur, fYCur, whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur)); 
                    if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) > -EPS) {
                        ++iWindingNumber;
                    }
                }
            } else {
                // prev is below
                if (fYCur <= fY0) {
                    printf("D WHichside(%lf,%lf) (%lf,%lf)-(%lf,%lf) : %lf\n", fX0, fY0,fXPrev, fYPrev, fXCur, fYCur, whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur)); 
                    if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) < EPS) {
                        --iWindingNumber;
                    }
                }
            }
        }
        fXPrev = fXCur;
        fYPrev = fYCur;
    }

    printf("WInding : %d\n", iWindingNumber);
    return (iWindingNumber > 0);
}


//----------------------------------------------------------------------------
//  isPolyBounds
// 
void isPolyBounds(int iN, double *afX, double *afY,
                  double *pfXMin, double *pfYMin,
                  double *pfXMax, double *pfYMax) {

    *pfXMin = dPosInf;
    *pfYMin = dPosInf;
    *pfXMax = dNegInf;
    *pfYMax = dNegInf;
    double *pfX = afX;
    double *pfY = afY;
    for (int i = 0; i < iN; ++i) {
        // extreme X
        if (*pfX > *pfXMax) {
            *pfXMax = *pfX;
        } else if (*pfX < *pfXMin) {
            *pfXMin = *pfX;
        }
        // extreme Y
        if (*pfY > *pfYMax) {
            *pfYMax = *pfY;
        } else if (*pfY < *pfYMin) {
            *pfYMin = *pfY;
        }

        ++pfX;
        ++pfY;
    }
    
}

//----------------------------------------------------------------------------
//  isPointInPoly2
//  returns value
//    0 : point (X0,Y0) is outside of poly
//   >0 : point inside poly or on poly
//  (http://www.geometryalgorithms.com/Archive/algorithm_0103/algorithm_0103.htm)
//
int isPointInPoly2(double fX0, double fY0, 
                   int iN, 
                   double *afX, double *afY) {

    int iResult = 0;
    double fXPrev = afX[iN-1];
    double fYPrev = afY[iN-1];

    for (int i = 0; i < iN; ++i) {
        double fXCur = afX[i];
        double fYCur = afY[i];
        // upwards
        if (fYPrev <= fY0)  {
            if (fYCur > fY0) {
                // point on left : increase winding number
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) >= 0) {
                    ++iResult;
                }
            }
        } else {
            // prev is below
            if (fYCur <= fY0) {
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) <= 0) {
                    --iResult;
                }
            }
        } 
        fXPrev = fXCur;
        fYPrev = fYCur;
    }

    // check if point is sitting on a polygon side
    if (iResult == 0) {
        fXPrev = afX[iN-1]; 
        fYPrev = afY[iN-1];
        for (int i = 0; (iResult == 0) && (i < iN); ++i) {
            double fXCur = afX[i];
            double fYCur = afY[i];
            if ((fX0>=jmin(fXPrev, fXCur)) && (jmax(fXPrev,fXCur)>=fX0) &&
                (fY0>=jmin(fYPrev, fYCur)) && (jmax(fYPrev,fYCur)>=fY0)) {
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) == 0) {
                    ++iResult;
                }
            }
        
            fXPrev = fXCur;
            fYPrev = fYCur;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
//  isPointInPoly2
//  returns value
//    0 : point (X0,Y0) is outside of poly
//   >0 : point inside poly or on poly
//  (http://www.geometryalgorithms.com/Archive/algorithm_0103/algorithm_0103.htm)
// 
int isPointInPoly2(double fX0, double fY0, VEC_POINTS vp) {

    int iResult = 0;
    DPOINT *p0 = vp.at(vp.size()-1);
    double fXPrev = p0->first;
    double fYPrev = p0->second;

    for (unsigned int i = 0; i < vp.size(); ++i) {
        DPOINT *p = vp.at(i);
        double fXCur = p->first;
        double fYCur = p->second;
        // upwards
        if (fYPrev <= fY0)  {
            if (fYCur > fY0) {
                // point on left : increase winding number
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) >= 0) {
                    ++iResult;
                }
            }
        } else  {
            // prev is below
            if (fYCur <= fY0) {
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) <= 0) {
                    --iResult;
                }
            }
        }
        fXPrev = fXCur;
        fYPrev = fYCur;
    }
    

    // check if point is sitting on a polygon side
    if (iResult == 0) {
        fXPrev = p0->first;
        fYPrev = p0->second;
        for (unsigned int i = 0; (iResult == 0) && (i < vp.size()); ++i) {
            DPOINT *p = vp.at(i);
            double fXCur = p->first;
            double fYCur = p->second;
            if ((fX0>=jmin(fXPrev, fXCur)) && (jmax(fXPrev,fXCur)>=fX0) &&
                (fY0>=jmin(fYPrev, fYCur)) && (jmax(fYPrev,fYCur)>=fY0)) {
                if (whichSide(fX0, fY0, fXPrev, fYPrev, fXCur, fYCur) == 0) {
                    ++iResult;
                }
            }
            fXPrev = fXCur;
            fYPrev = fYCur;
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
//  spherInterpol
//    creates a point lying a great circle from p1 to p2 at a fraction of dT
//    along the angle
//    p1 and p2 are given in deg long and deg lat
//
DPOINT *spherInterpol(DPOINT *dP1, DPOINT *dP2, double dT) {
    // convert polar to cart
    Vec3D *v1 = polarD2Cart(dP1);
    Vec3D *v2 = polarD2Cart(dP2);

    Vec3D  *vRes = spherInterpol(v1, v2, dT);

    DPOINT *pRes = cart2PolarD(vRes);


    delete v1;
    delete v2;
    delete vRes;

    return pRes;
}

//----------------------------------------------------------------------------
//  spherInterpol
//    creates a point lying a great circle from p1 to p2 at a fraction of dT
//    along the angle
//    p1 and p2 and result are given in cartesian coordinates
//
Vec3D *spherInterpol(Vec3D *p1, Vec3D *p2, double dT) {
    double dAngle = p1->getAngle(p2);
    Vec3D *vAxis  = p1->crossProduct(p2);
    vAxis->normalize();

    Quat *qAxis = Quat::makeRotation(dT*dAngle, vAxis);
    
    Vec3D *vRes = qAxis->apply(p1);

    delete vAxis;
    delete qAxis;

    return vRes;
}

const double RAD = M_PI/180;

//----------------------------------------------------------------------------
//  cart2PolarD
//    cartesian -> polar (unit sphere)
//    angles in degrees
//
DPOINT *cart2PolarD(Vec3D *v) {
    double dP = asin(v->m_fZ);
    double dL = atan2(v->m_fY,v->m_fX);
    return new DPOINT(dL/RAD, dP/RAD);
}

//----------------------------------------------------------------------------
//  polarD2Cart
//    polar (unit sphere) -> cartesian
//    angles in degrees
//
Vec3D *polarD2Cart(DPOINT *p) {
    double dL = RAD*p->first;
    double dP = RAD*p->second;
    double dX = cos(dL)*cos(dP);    
    double dY = sin(dL)*cos(dP);    
    double dZ = sin(dP);    
    return new Vec3D(dX, dY, dZ);
}

//----------------------------------------------------------------------------
//  cart2Polar
//    cartesian -> polar (unit sphere)
//    angles in radians
//
DPOINT *cart2Polar(Vec3D *v) {
    double dP = asin(v->m_fZ);
    double dL = atan2(v->m_fY,v->m_fX);
    return new DPOINT(dL, dP);
}

//----------------------------------------------------------------------------
//  polar2Cart
//    polar (unit sphere) -> cartesian
//    angles in radians
//
Vec3D *polar2Cart(DPOINT *p) {
    double dL = p->first;
    double dP = p->second;
    double dX = cos(dL)*cos(dP);    
    double dY = sin(dL)*cos(dP);    
    double dZ = sin(dP);    
    return new Vec3D(dX, dY, dZ);
}


//----------------------------------------------------------------------------
//  spherdist
//
double spherdist(double dLon1, double  dLat1, double dLon2, double dLat2, double dR) {
    double dX1 = cos(dLon1)*cos(dLat1);    
    double dY1 = sin(dLon1)*cos(dLat1);    
    double dZ1 = sin(dLat1);    
    double dX2 = cos(dLon2)*cos(dLat2);    
    double dY2 = sin(dLon2)*cos(dLat2);    
    double dZ2 = sin(dLat2);    
    double dProd = dX1*dX2+dY1*dY2+dZ1*dZ2;
    if (dProd > 1) {
        dProd = 1;
    } else if (dProd < -1) {
        dProd = -1;
    }
    return dR * acos(dProd);
}


//----------------------------------------------------------------------------
//  spherdistDeg
//
double spherdistDeg(double dLon1, double  dLat1, double dLon2, double dLat2, double dR) {
    return spherdist(dLon1*M_PI/180, dLat1*M_PI/180, dLon2*M_PI/180, dLat2*M_PI/180, dR);
}


//----------------------------------------------------------------------------
//  spherdist
//
double spherdist(Vec3D *v1, Vec3D *v2, double dR) {
    double dProd = v1->dotProduct(v2);
    if (dProd > 1) {
        dProd = 1;
    } else if (dProd < -1) {
        dProd = -1;
    }
    return dR * acos(dProd);
}

//----------------------------------------------------------------------------
//  spherdist
//
double spherdist(double dX0, double dY0, double dZ0, double dX1, double dY1, double dZ1, double dR) {
    double dProd = dX0*dX1 + dY0*dY1 + dZ0*dZ1;
    if (dProd > 1) {
        dProd = 1;
    } else if (dProd < -1) {
        dProd = -1;
    }
    return dR * acos(dProd);
}


//----------------------------------------------------------------------------
//  cartdist
//
double cartdist(double dX1, double  dY1, double dX2, double dY2, double dScale) {
    double d = (dX2-dX1)*(dX2-dX1)+(dY2-dY1)*(dY2-dY1);
    return dScale*sqrt(d);
}
