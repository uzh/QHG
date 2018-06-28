#include <stdio.h>
#include <math.h>

#include "Vec3D.h"
#include "Tegmark.h"

const double S3  = sqrt(3);
const double T0  = tan(M_PI/5);
// side of projected ico-face (inscribed in unit sphere) 
const double A   = sqrt(9*T0*T0-3);
// proportionality factor
const double ETA = A*sqrt(5*S3/(4*M_PI)); // 1.09843845

// elements of 120° rotations
const double CC = cos(2*M_PI/3);
const double SS = sin(2*M_PI/3);

//--------------------------------------------------------------
// mirr 
//  mirror the vector at the y-axis (in place) &  return it
//
Vec3D *mirr(Vec3D *pV) {
    pV->m_fX *= -1;
    return pV;
}

//--------------------------------------------------------------
// rotplus
//  rotate the vector by 120° (in place) &  return it
//
Vec3D *rotplus(Vec3D *pV) {
    double dX = CC*pV->m_fX - SS*pV->m_fY;
    double dY = SS*pV->m_fX + CC*pV->m_fY;
    pV->m_fX = dX;
    pV->m_fY = dY;
    return pV;
}

//--------------------------------------------------------------
// rotplus
//  rotate the vector by -120° (in place) &  return it
//
Vec3D *rotminus(Vec3D *pV) {
    double dX =  CC*pV->m_fX + SS*pV->m_fY;
    double dY = -SS*pV->m_fX + CC*pV->m_fY;
    pV->m_fX = dX;
    pV->m_fY = dY;
    return pV;
}


//--------------------------------------------------------------
// distort
//  the reverse transformation from Tegmark's 1996 paper
//    tegmark triangle -> ico face
//   
//  the formula in Tegmartk's paper is not correct:
//  x and y have to be scaled by ETA before 
// (and the scaling in the formula for y has to bee omitted)
//  ATTENTION: only valid in upper right subtriangle (see paper)
//
void Tegmark::distortLocal(double dX, double dY, double *pdX1, double *pdY1) {
    dX /= ETA;
    dY /= ETA;
    //    double z = tan(S3*dY*dY/(2*ETA*ETA));
    double z = tan(S3*dY*dY/2);
    double dT = (1+S3*z)/(S3-z);
    
    *pdY1 = 0.5*sqrt(3*dT*dT-1);
    double dY2 = (*pdY1)*(*pdY1);
    // for isome subdivs (75, 99, 105, 117, ...) both fX and dy become exactly 0 -> nan
    if (isnan(*pdY1)) {
        printf("transform2 nan for (%f %f)\n", dX, dY);
        printf("  z: %f\n", z);
        printf("  T: %f\n", dT);
        printf("  3T^2: %f\n", 3*dT*dT);
    }
    if ((dY==0) && (dX == 0)) {
        *pdX1 = 0;
    } else {
        *pdX1 = dX*(*pdY1)*sqrt((1+dY2)/(dY*dY*(1+4*dY2)-dX*dX*dY2));
    }
}


//--------------------------------------------------------------
// straighten
//  the  transformation from Tegmark's 1996 paper
//    ico face->tegmark triangle
//   
//  the formula in Tegmartk's paper is not correct:
//  x needs not to be multiplied by ETA (ETA is already in y1)
//  ATTENTION: only valid in upper right subtriangle (see paper)
//
void Tegmark::straightenLocal(double dX, double dY, double *pdX1, double *pdY1) {
    double y4 = sqrt(1+4*dY*dY);
    *pdY1 = ETA*sqrt((2/S3)*atan(S3*(y4-1)/(y4+3)));
    *pdX1 = ((dX**pdY1)/dY)*y4/sqrt(1+dX*dX+dY*dY);

}


//--------------------------------------------------------------
// distortLocalV
//  the reverse transformation from Tegmark's 1996 paper
//  ATTENTION: only valid in upper right subtriangle (see paper)
//
Vec3D *Tegmark::distortLocalV(Vec3D *pV) {
    double dXT=0;
    double dYT=0;
    distortLocal(pV->m_fX, pV->m_fY, &dXT, &dYT);
    pV->m_fX = dXT;
    pV->m_fY = dYT;
    return pV;
}


//--------------------------------------------------------------
// straightenLocal
//  the transformation from Tegmark's 1996 paper
//  ATTENTION: only valid in upper right subtriangle (see paper)
//
Vec3D *Tegmark::straightenLocalV(Vec3D *pV) {
    double dXT=0;
    double dYT=0;

    straightenLocal(pV->m_fX, pV->m_fY, &dXT, &dYT);
    pV->m_fX = dXT;
    pV->m_fY = dYT;

    return pV;
}


//--------------------------------------------------------------
// invtransV
//  apply the inverse tegmark tp pV:
//  - determine the subtriangle pV lies in
//  - perform appropriate reflections and rotations to bring
//    pV to the upper right subtriangle
//  - apply inverse tegmark
//  - undo reflections and rotations
//
Vec3D *Tegmark::straighten(Vec3D *pV) {
    // find sector of triangle pV lies in
    int iSector = 0;
    if (pV->m_fX >= 0) {
        iSector ++;
    }
    iSector <<= 1;


    if (pV->m_fY >= pV->m_fX/sqrt(3)) {
        iSector ++;
    }
    iSector <<= 1;

    if (pV->m_fY >= -pV->m_fX/sqrt(3)) {
        iSector ++;
    }

    // unstretch it
    switch (iSector) {
    case 0:
        //        printf("Sector D\n");
        pV = rotplus(mirr(straightenLocalV(mirr(rotminus(pV)))));
        break;
    case 1:
        printf("*****impossible 1**********\n");
        break;
    case 2:
        //        printf("Sector C\n");
        pV = rotplus(straightenLocalV(rotminus(pV)));
        break;
    case 3:
        //        printf("Sector B\n");
        pV = mirr(straightenLocalV(mirr(pV)));
        break;
    case 4:
        //        printf("Sector E\n");
        pV = rotminus(straightenLocalV(rotplus(pV)));
        break;
    case 5:
        //        printf("Sector F\n");
        pV = rotminus(mirr(straightenLocalV(mirr(rotplus(pV)))));
        break;
    case 6:
        printf("*****impossible 2**********\n");
        break;
    case 7:
        //        printf("Sector A\n");
        pV = straightenLocalV(pV);
    }
    return pV;
}


//--------------------------------------------------------------
// distort
//  apply the tegmark to pV:
//  - determine the subtriangle pV lies in
//  - perform appropriate reflections and rotations to bring
//    pV to the upper right subtriangle
//  - apply tegmark
//  - undo reflections and rotations
//
Vec3D *Tegmark::distort(Vec3D *pV) {
    // find sector of triangle pV lies in
    int iSector = 0;
    if (pV->m_fX >= 0) {
        iSector ++;
    }
    iSector <<= 1;


    if (pV->m_fY >= pV->m_fX/sqrt(3)) {
        iSector ++;
    }
    iSector <<= 1;

    if (pV->m_fY >= -pV->m_fX/sqrt(3)) {
        iSector ++;
    }

    // unstretch it
    switch (iSector) {
    case 0:
        /////        printf("Sector D\n");
        pV = rotplus(mirr(distortLocalV(mirr(rotminus(pV)))));
        break;
    case 1:
        printf("*****impossible 1**********\n");
        break;
    case 2:
        /////        printf("Sector C\n");
        pV = rotplus(distortLocalV(rotminus(pV)));
        break;
    case 3:
        /////        printf("Sector B\n");
        pV = mirr(distortLocalV(mirr(pV)));
        break;
    case 4:
        /////        printf("Sector E\n");
        pV = rotminus(distortLocalV(rotplus(pV)));
        break;
    case 5:
        /////        printf("Sector F\n");
        pV = rotminus(mirr(distortLocalV(mirr(rotplus(pV)))));
        break;
    case 6:
        printf("*****impossible 2**********\n");
        break;
    case 7:
        /////        printf("Sector A\n");
        pV = distortLocalV(pV);
    }
    return pV;
}
