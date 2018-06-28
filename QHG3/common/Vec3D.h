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

#ifndef __VEC3D_H__
#define __VEC3D_H__

#define EPSC 1e-5

class Vec3D {
public:
    Vec3D();
    Vec3D(double fX, double fY, double fZ);
    Vec3D(const Vec3D *pv);
    void set(double fX, double fY, double fZ) { m_fX = fX;m_fY = fY; m_fZ = fZ;};
    void set(const Vec3D *pv) { m_fX = pv->m_fX;m_fY = pv->m_fY; m_fZ = pv->m_fZ;};
    void add(const Vec3D *pv);
    void subtract(const Vec3D *pv);
    void scale(double f);
    double calcNorm() const;
    double dist(const Vec3D *pv);
    void normalize();
    inline double dotProduct(const Vec3D *pv) const {return m_fX*pv->m_fX + m_fY*pv->m_fY + m_fZ*pv->m_fZ;};
    Vec3D *crossProduct(const Vec3D *pv) const;
    double getCrossSize(const Vec3D *pv) const;
    double getAngle(const Vec3D *pv) const;
    bool operator==(const Vec3D &v) const;
    
    void trunc();

    // oder relation first x, then y, then z
    //    inline bool operator<(const Vec3D &v) const { printf("%20.18f,%20.18f,%29.18f) < (%20.18f,%20.18f,%20.18f): (%e,%e,%e): %s\n", m_fX, m_fY, m_fZ, v.m_fX, v.m_fY, v.m_fZ, m_fX-v.m_fX, m_fY-v.m_fY, m_fZ-v.m_fZ,((m_fX < v.m_fX) || ((m_fX == v.m_fX)&&(m_fY < v.m_fY)) || ((m_fX == v.m_fX)&&(m_fY == v.m_fY)&&(m_fZ < v.m_fZ)))?"yes":"no");return (m_fX < v.m_fX) || ((m_fX == v.m_fX)&&(m_fY < v.m_fY)) || ((m_fX == v.m_fX)&&(m_fY == v.m_fY)&&(m_fZ < v.m_fZ));};
    // cast it to float to reduce precision (avoiding "epsilontics")
    /*
    inline bool operator<(const Vec3D &v) const { return (((float)(m_fX+EPSC) <   (float)(v.m_fX+EPSC)) || 
                                                          (((float)(m_fX+EPSC) == (float)(v.m_fX+EPSC)) && ((float)(m_fY+EPSC) <  (float)(v.m_fY+EPSC))) || 
                                                          (((float)(m_fX+EPSC) == (float)(v.m_fX+EPSC)) && ((float)(m_fY+EPSC) == (float)(v.m_fY+EPSC))  && ((float)(m_fZ+EPSC) < (float)(v.m_fZ+EPSC))));};
    */
    inline bool operator<(const Vec3D &v) const { return (((float)(m_fX+EPSC) <   (float)(v.m_fX+EPSC)) || 
                                                          ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && ((float)(m_fY+EPSC) <  (float)(v.m_fY+EPSC))) || 
                                                          ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && (!((float)(v.m_fY+EPSC) < (float)(m_fY+EPSC)))  && ((float)(m_fZ+EPSC) < (float)(v.m_fZ+EPSC))));};
    /*
    bool operator<(const Vec3D &v) const;
    */
    /*
    static Vec3D vX;
    static Vec3D vY;
    static Vec3D vZ;
    */

    // the components of the vector
    double m_fX;
    double m_fY;
    double m_fZ;

};


#endif
