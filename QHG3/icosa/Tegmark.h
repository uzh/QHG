#ifndef __TEGMARK_H__
#define __TEGMARK_H__

class Vec3D;

class Tegmark {

public:
    static void distortLocal(double dX, double dY, double *pdX1, double *pdY1);
    static void straightenLocal(double dX, double dY, double *pdX1, double *pdY1);



    static Vec3D *distortLocalV(Vec3D *pV);
    static Vec3D *straightenLocalV(Vec3D *pV);



    static Vec3D *distort(Vec3D *pV);
    static Vec3D *straighten(Vec3D *pV);


};

#endif
