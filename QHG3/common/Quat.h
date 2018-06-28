/*============================================================================
| Quat
| 
|  A simple class for quaternions.
|  Note: all operations defined below DO NOT change the this quaternion, but
|  create new instances.
|  
|  It's the caller's responability to clean them  up.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __QUAT_H__
#define __QUAT_H__

class Vec3D;

class Quat {

public:
    
    Quat(double fR, double fI, double fJ, double fK);
    Quat(double fI, double fJ, double fK);
    Quat(Quat *pq);
    Quat(Vec3D *pv);
    Quat(double fR);
    Quat();
    static Quat *makeRotation(double fAngle, double fX, double fY, double fZ);
    static Quat *makeRotation(double fAngle, const Vec3D &v);
    static Quat *makeRotation(const  Vec3D *pvFrom, const Vec3D *pvTo);
    void add(Quat *q);
    void sub(Quat *q);
    void conjugate();
    void mult(Quat *q);
    double calcNorm();
    void normalize();
    void invert();
    void scale(double f);
    Quat *apply(Quat *q);
    Vec3D *apply(Vec3D *v);
    void apply(Vec3D *v, Vec3D *vR);


    void mult(Quat *q, Quat *qRes);

    // the quaternions components
    double m_fR;
    double m_fI;
    double m_fJ;
    double m_fK;
    
};

#endif
