/*============================================================================
| Vec3D
| 
|  A simple class for 3d vectors.
|  Note: all operations defined below DO NOT change the this vector, but
|  create new instances.
|  
|  It's the caller's responability to clean them  up.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <stdio.h>
#include <math.h>
#include "Vec3D.h"


/*
    // some constants (unit vectors in x, y, z direction)
    static Vec3D vX = new Vec3D(1,0,0);
    static Vec3D vY = new Vec3D(0,1,0);
    static Vec3D vZ = new Vec3D(0,0,1);
*/
const double EPS2 = 0.00001;
    
//------------------------------------------------------------------------
// constructor
//
Vec3D::Vec3D() {
    m_fX = 0;
    m_fY = 0;
    m_fZ = 0;
}
    
//------------------------------------------------------------------------
// constructor
//
Vec3D::Vec3D(double fX, double fY, double fZ) {
    m_fX = fX;
    m_fY = fY;
    m_fZ = fZ;
}
    
//------------------------------------------------------------------------
// constructor
//
Vec3D::Vec3D(const Vec3D *pv) {
    m_fX = pv->m_fX;
    m_fY = pv->m_fY;
    m_fZ = pv->m_fZ;
}
    

//------------------------------------------------------------------------
// add
//   return new vector which is the sum of this and v
//
void Vec3D::add(const Vec3D *pv) {
    m_fX+=pv->m_fX; 
    m_fY+=pv->m_fY;
    m_fZ+=pv->m_fZ;
}
    
//------------------------------------------------------------------------
// subtract
//   return new vector which is the difference of this and v
//
void Vec3D::subtract(const Vec3D *pv) {
    m_fX-=pv->m_fX; 
    m_fY-=pv->m_fY;
    m_fZ-=pv->m_fZ;
}
    
//------------------------------------------------------------------------
// scale
//   return a new vector which is a scaled version of this
//
void Vec3D::scale(double f) {
    m_fX *= f;
    m_fY *= f;
    m_fZ *= f;
}
    
//------------------------------------------------------------------------
// calcNorm
//   return the length of this vector
//
double Vec3D::calcNorm() const {
    return  sqrt(m_fX*m_fX+m_fY*m_fY+m_fZ*m_fZ);
}


//------------------------------------------------------------------------
// dist
//   return the distance to other vector
//
double Vec3D::dist(const Vec3D *pv) {
    Vec3D A(this);
    A.subtract(pv);
    return A.calcNorm();
}

//------------------------------------------------------------------------
// normalize
//   returns a new vector which is a normalized version of this
//
void Vec3D::normalize() {
    double f = calcNorm();
    scale(1/f);
}

//------------------------------------------------------------------------
// crossProduct
//   returns a new vector which is the result of 
//   the vector product of this and v
//
Vec3D *Vec3D::crossProduct(const Vec3D *pv) const {
        return new Vec3D(m_fY*pv->m_fZ - m_fZ*pv->m_fY,
                         m_fZ*pv->m_fX - m_fX*pv->m_fZ,
                         m_fX*pv->m_fY - m_fY*pv->m_fX);
}
    
//------------------------------------------------------------------------
// getCrossSize
//   the length of the cross product of this and v
//   useful to calculate triange and parallelogram areas
//
double Vec3D::getCrossSize(const Vec3D *pv) const {
    Vec3D v(m_fY*pv->m_fZ - m_fZ*pv->m_fY,
            m_fZ*pv->m_fX - m_fX*pv->m_fZ,
            m_fX*pv->m_fY - m_fY*pv->m_fX);
    return v.calcNorm();
}

//------------------------------------------------------------------------
// getAngle
//   returns the angle between this and v
//
double Vec3D::getAngle(const Vec3D *pv) const {
    double dAngle = 0;
    double nn = calcNorm()*pv->calcNorm();
    if (nn > 0) {
        double v = dotProduct(pv)/nn;
        if (v > 1) {
            v = 1;
        } else if (v < -1) {
            v = -1;
        }
        dAngle = acos(v); 
    }
    return dAngle;
}
    
 
bool Vec3D::operator==(const Vec3D &other) const {
    //    return (m_dX == other.m_dX) && (m_dY == other.m_dY);
    /*
      return (fabs(m_fX-other.m_fX) < EPSC) && 
           (fabs(m_fY-other.m_fY) < EPSC) &&
           (fabs(m_fZ-other.m_fZ) < EPSC);
    */
    return (fabs((float)(m_fX+EPSC)-(float)(other.m_fX+EPSC)) < EPSC) && 
           (fabs((float)(m_fY+EPSC)-(float)(other.m_fY+EPSC)) < EPSC) &&
           (fabs((float)(m_fZ+EPSC)-(float)(other.m_fZ+EPSC)) < EPSC);

}

/*
bool Vec3D::operator<(const Vec3D &other) const {
    return (m_fX < other.m_fX) || 
        (((m_fX == other.m_fX)&&(m_fY < other.m_fY)) || 
         ((m_fX == other.m_fX)&&(m_fY == other.m_fY)&&(m_fZ < other.m_fZ)));
}

*/



void Vec3D::trunc() {
    m_fX = float(m_fX);
    m_fY = float(m_fY);
    m_fZ = float(m_fZ);
}
