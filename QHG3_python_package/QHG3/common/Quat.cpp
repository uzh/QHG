#include <math.h>
#include "Vec3D.h"
#include "Quat.h"

//------------------------------------------------------------------------
// constructor
//   definition of all components
//
Quat::Quat(double fR, double fI, double fJ, double fK) {
    m_fR = fR;      
    m_fI = fI;      
    m_fJ = fJ;      
    m_fK = fK;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a pure quaternion by components
//
Quat::Quat(double fI, double fJ, double fK) {
    m_fR = 0;       
    m_fI = fI;      
    m_fJ = fJ;      
    m_fK = fK;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a pure quaternion by a vector
//
Quat::Quat(Vec3D *v) {
    m_fR = 0;       
    m_fI = v->m_fX;      
    m_fJ = v->m_fY;      
    m_fK = v->m_fZ;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a quaternion having only a real part
//
Quat::Quat(double fR) {
    m_fR = fR;      
    m_fI = 0;       
    m_fJ = 0;       
    m_fK = 0;       
}
    
//------------------------------------------------------------------------
// constructor
//   definition of the unit quaternion
//
Quat::Quat() {
    m_fR = 1;       
    m_fI = 0;       
    m_fJ = 0;       
    m_fK = 0;       
}
    
//------------------------------------------------------------------------
// constructor
//   copy constructor
//
Quat::Quat(Quat *q){
    m_fR = q->m_fR;
    m_fI = q->m_fI;
    m_fJ = q->m_fJ;
    m_fK = q->m_fK;
}
    
//------------------------------------------------------------------------
// makeRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat *Quat::makeRotation(double fAngle, double fX, double fY, double fZ){
    double fS = sin(fAngle/2);
    double fC = cos(fAngle/2);
    return new Quat(fC, fX*fS, fY*fS, fZ*fS);
}

//------------------------------------------------------------------------
// makeRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat *Quat::makeRotation(double fAngle, const Vec3D &v){
    return makeRotation(fAngle, v.m_fX, v.m_fY, v.m_fZ);
}

//------------------------------------------------------------------------
// makeRotation
//   create a quaternion which rotates pvFrom to pvTo
//
Quat *Quat::makeRotation(const  Vec3D *pvFrom, const Vec3D *pvTo) {
    // calculate axis
    Vec3D *pvAxis = pvFrom->crossProduct(pvTo);
    // get angle
    double dAngle = pvFrom->getAngle(pvTo);
    // create rotation
    pvAxis->normalize();
    Quat *rot1 = Quat::makeRotation(dAngle, pvAxis);

    // clean up
    delete pvAxis;

    return rot1;
}

    
//------------------------------------------------------------------------
// add
//   add q  to this quaternion
//
void Quat::add(Quat *q) {
    m_fR += q->m_fR;
    m_fI += q->m_fI;
    m_fJ += q->m_fJ;   
    m_fK += q->m_fK;
}

//------------------------------------------------------------------------
// sub
//   subtract q from this quaternion
//
void Quat::sub(Quat *q) {
    m_fR -= q->m_fR;
    m_fI -= q->m_fI;
    m_fJ -= q->m_fJ;
    m_fK -= q->m_fK;
}

//------------------------------------------------------------------------
// conjugate 
//   conjugates this quaternion
//
void Quat::conjugate() {
    m_fI *= -1;
    m_fJ *= -1;   
    m_fK *= -1;
}
    
//------------------------------------------------------------------------
// mult
//   multiplies this quaternion with q 
//   result: first q, then this
//
void Quat::mult(Quat *q) {
    double dR = m_fR*q->m_fR - m_fI*q->m_fI - m_fJ*q->m_fJ - m_fK*q->m_fK;
    double dI = m_fR*q->m_fI + m_fI*q->m_fR + m_fJ*q->m_fK - m_fK*q->m_fJ;
    double dJ = m_fR*q->m_fJ + m_fJ*q->m_fR + m_fK*q->m_fI - m_fI*q->m_fK;
    double dK = m_fR*q->m_fK + m_fK*q->m_fR + m_fI*q->m_fJ - m_fJ*q->m_fI;
    
    m_fR = dR;
    m_fI = dI;
    m_fJ = dJ;
    m_fK = dK;
}

//------------------------------------------------------------------------
// mult
//   multiplies this quaternion with q and places result in qRes
//   result: first q, then this
//
void Quat::mult(Quat *q, Quat *qRes) {
    double dR = m_fR*q->m_fR - m_fI*q->m_fI - m_fJ*q->m_fJ - m_fK*q->m_fK;
    double dI = m_fR*q->m_fI + m_fI*q->m_fR + m_fJ*q->m_fK - m_fK*q->m_fJ;
    double dJ = m_fR*q->m_fJ + m_fJ*q->m_fR + m_fK*q->m_fI - m_fI*q->m_fK;
    double dK = m_fR*q->m_fK + m_fK*q->m_fR + m_fI*q->m_fJ - m_fJ*q->m_fI;
    
    qRes->m_fR = dR;
    qRes->m_fI = dI;
    qRes->m_fJ = dJ;
    qRes->m_fK = dK;
}

//------------------------------------------------------------------------
// calcNorm
//   return the length of this
//
double Quat::calcNorm() {
    double f =  sqrt(m_fR*m_fR + m_fI*m_fI + m_fJ*m_fJ + m_fK*m_fK);
    return f;
}
    
//------------------------------------------------------------------------
// normalize 
//   normalizes this quaternion
//
void Quat::normalize() {
    double f = calcNorm();
    scale(1/f);
}
    
//------------------------------------------------------------------------
// invert 
//   inverts this quaternion
//
void Quat::invert() {
    double f = calcNorm();
    conjugate();
    scale(1/f*f);
}
    
//------------------------------------------------------------------------
// scale
//   returns a new quaternion which is a scaled version of this
//
void Quat::scale(double f) {
    m_fR *= f;
    m_fI *= f;
    m_fJ *= f;
    m_fK *= f;
}
    
//------------------------------------------------------------------------
// apply
//   applying this to q
//   For a given quaternion p 
//   p.apply(q) = p*q*p^{-1}
//   returns a new quaternion which is the result of
//   conjugating q with this.
//   (this is not changed)
//
Quat *Quat::apply(Quat *q) {
    // inverse quaternion for this
    Quat *qRes = new Quat(this);
    Quat *qInv = new Quat(this);
    qInv->invert();
    // multiply with argument
    qRes->mult(q);
    //multiply result with this
    qRes->mult(qInv);
    delete qInv;
    return qRes;
}
    
//------------------------------------------------------------------------
// apply
//   applying this quaternion to the vector v
//   For a given quaternion p 
//   p.apply(v) = p*v'*p^{-1}
//   where v' is the pure quaternion (0, v.x, v.y, v.z)
//   returns a new vector which is the result of
//   conjugating v (interpreted as a pure quaternion) with this
//   (this is not changed)
//
Vec3D *Quat::apply(Vec3D *v) {
    Quat *qTemp = new Quat(v);
    Quat *q = apply(qTemp);
    Vec3D *pVRes = new Vec3D(q->m_fI, q->m_fJ, q->m_fK);
    delete q;
    delete qTemp;
    return pVRes;
}
    
