#include <stdio.h>
#include <math.h>
#include <string.h>
#include "trackball.h"
#include "interquat.h"

//----------------------------------------------------------------------------
// qinvert
//
void qinvert(float q0[4], float qRes[4]) {
    for (int i = 0; i < 3; i++) {
        qRes[i] = -q0[i];
    } 
    qRes[3] = q0[3];
}


//----------------------------------------------------------------------------
// qpower
//
void qpower(float q0[4], float t, float qRes[4]) {
    if (q0[3] > 1) {
        q0[3] = 1;
    } else if (q0[3] < -1) {
        q0[3] = -1;
    }
    float fTheta = acos(q0[3]);
    float fS0 = sin(fTheta);
    if (fS0 == 0) {
        // fS0 == 0 <=> fTheta = k *PI <=> rot = k*2*PI = identity
        memcpy(qRes, q0, 4*sizeof(float));
    } else {
        fTheta *= t;
        float fS = sin(fTheta)/fS0;
        for (int i = 0; i < 3; i++) {
            qRes[i] = q0[i]*fS;
        }
        qRes[3] = cos(fTheta);
    } 
}


//----------------------------------------------------------------------------
// slerp
//
void slerp(float t, float q0[4], float q1[4], float qRes[4]) {
    float qA[4];

    // set qA to the inverse of q0
    qinvert(q0, qA);

    // multiply inverse of q0 with q1
    float qB[4];
    add_quats(q1, qA, qB);

    // raise the product to power t
    qpower(qB, t, qA);

    add_quats(qA, q0, qRes);
}

//----------------------------------------------------------------------------
// squad
//
void squad(float t, float p[4], float a[4], float b[4], float q[4], float qRes[4]) {
    float q1[4];
    slerp(t, p, q, q1);

    float q2[4];
    // set q2 to the inverse of q1
    qinvert(q1, q2);

    float q3[4];
    slerp(t, a, b, q3);

    float q4[4];
    add_quats(q3, q2, q4);

    float q5[4];
    qpower(q4, 2*t*(1-t), q5);
    
    add_quats(q5, q1, qRes);
}

//----------------------------------------------------------------------------
// qlog
//
void qlog(float q0[4], float qRes[3]) {
    
    if (q0[3] > 1) {
        q0[3] = 1;
    } else if (q0[3] < -1) {
        q0[3] = -1;
    }
    
    float fTheta = acos(q0[3]);
    float fS0 = sin(fTheta);
    if (fS0 == 0) {
        // fS0 == 0 <=> fTheta = k *PI <=> rot = k*2*PI = identity
        memcpy(qRes, q0, 3*sizeof(float));
    } else {
        float fS = fTheta/fS0;
        for (int i = 0; i < 3; i++) {
            qRes[i] = q0[i]*fS;
        }
    } 
}



//----------------------------------------------------------------------------
// qexp
//
void qexp(float q0[3], float qRes[4]) {
    float fTheta = sqrt(q0[0]*q0[0] + q0[1]*q0[1] + q0[2]*q0[2]);
    float fS = sin(fTheta);
    if (fTheta == 0) {
        fS = 1;
    } else {
        fS /= fTheta;
    }

    for (int i = 0; i < 3; i++) {
        qRes[i] = q0[i]*fS;
    }
    qRes[3] = cos(fTheta);
}

void qapply(float q0[4], float vIn[3], float vRes[3]) {
    float axRes[4];
    float axIn[4];
    memcpy(axIn, vIn, 3*sizeof(float));
    axIn[3] = 0;

    float qInv[4];
    qinvert(q0, qInv);
    float qTemp[4];

    add_quats(qInv, axIn,  qTemp);
    add_quats(qTemp, q0, axRes);
    memcpy(vRes, axRes, 3*sizeof(float));
}



//----------------------------------------------------------------------------
// qshow
//
void qshow(float q[4]) {
    printf("  %f %f, %f, %f", q[0], q[1], q[2], q[3]);
    
    float fL[3];
    qlog(q, fL);

    float fTheta = sqrt(fL[0]*fL[0] + fL[1]*fL[1] + fL[2]*fL[2]);
    float fN = q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
    printf(": rotation of %f deg around %f, %f, %f; s:%f, L %f\n", 
           360*fTheta/M_PI, fL[0]/fTheta, fL[1]/fTheta, fL[2]/fTheta, fL[0]*fL[0] + fL[1]*fL[1] + fL[2]*fL[2],fN);


}

//----------------------------------------------------------------------------
// ushow
//
void ushow(float u[3]) {
    printf("  %f %f, %f", u[0], u[1], u[2]);
    
    float fTheta = sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2]);
    
    printf(": %f*(%f, %f, %f)\n", 
           360*fTheta/M_PI, u[0]/fTheta, u[1]/fTheta, u[2]/fTheta);
    

}


//----------------------------------------------------------------------------
// qscale
//
void qscale(float q[4], float f) {
    for (int i = 0; i < 4; i++) {
        q[i] *= f;
    }
}
