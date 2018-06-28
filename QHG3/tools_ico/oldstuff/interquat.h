#ifndef __INTERQUAT_H__
#define __INTERQUAT_H__


// quaternion: q[0]*i+q[1]*j+q[2]*k+q[3]

void qinvert(float q0[4], float qRes[4]);
void qpower(float q0[4], float t, float qRes[4]);
void slerp(float t, float q0[4], float q1[4], float qRes[4]);
void squad(float t, float p[4], float a[4], float b[4], float q[4], float qRes[4]);

void qapply(float q0[4], float vIn[3], float vRes[3]);
void qlog(float q0[4], float qRes[3]);
void qexp(float q0[3], float qRes[4]);
void qscale(float q[4], float f);

void qshow(float q[4]);
void ushow(float u[3]);
#endif
