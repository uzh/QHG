#include <math.h>

#ifdef OMP
#include <omp.h>
#endif

#include "types.h"
#include "utils.h"
#include "geomutils.h"
#include "Vec3D.h"
#include "Quat.h"

#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"

#include "TopoTemp.h"
#include "GradiTemp.h"


//-------------------------------------------------------------------------------------------------
// constructor
//
GradiTemp::GradiTemp(char *pAltFile, char *pTempFile)
    : TopoTemp(pAltFile, pTempFile) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
GradiTemp::GradiTemp(ValReader *pVRAltitude, char *pTempFile)
    : TopoTemp(pVRAltitude, pTempFile) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
GradiTemp::GradiTemp(char *pAltFile, ValReader *pVRTemperature)
    : TopoTemp(pAltFile, pVRTemperature) {

}

//-------------------------------------------------------------------------------------------------
// constructor
//
GradiTemp::GradiTemp(ValReader *pVRAltitude, ValReader *pVRTemperature)
    : TopoTemp(pVRAltitude, pVRTemperature) {

}

//-----------------------------------------------------------------------------------------------
// destructor
//
GradiTemp::~GradiTemp() {
}

//-----------------------------------------------------------------------------------------------
// adjustTemperature
//
int GradiTemp::adjustTemperature(double fParam, bool bAccumulateTemp) {
    int iResult = -1;
    
    if (m_bReady) {
        iResult = 0;
    
 
        double dDistance = m_pQMRAltitude->getDLon()*M_PI/180;
#ifdef OMP
#pragma omp parallel for
#endif
        for (unsigned int iY = 0; iY < m_iH; iY++) {
            double dPhi = m_pQMRAltitude->Y2Lat(iY);
#ifdef OMP
            if (omp_get_thread_num() == 0) {
                if (iY%40==0){printf("...Phi: %f\r", dPhi);fflush(stdout);}
            }
#else
            if (iY%40==0){printf("...Phi: %f\r", dPhi);fflush(stdout);}
#endif
            for (unsigned int iX = 0; iX < m_iW; iX++) {
                double dTheta = m_pQMRAltitude->X2Lon(iX);
                
                double dAlt = m_pQMRAltitude->getDValue(iX, iY);
                if (dAlt >= -1000) {
                    Vec3D vGradient;
                    DPOINT dP(dTheta, dPhi);      
                    
                    if ((iX == 283) && (iY == 481)) {
                        //                        m_bVerbose = true;
                    }
                    int iRes = getGradient(dP, dDistance, &vGradient);
                    if (iRes != 0) {
                        //bOK = false;
                    }
                    vGradient.normalize();
                    Vec3D *pSunray = polarD2Cart(&dP);
                    pSunray->m_fZ = 0;
                    pSunray->normalize();
                    float dDot = (float)pSunray->dotProduct(&vGradient);
                    dDot = (dDot>1)?1:(dDot<-1)?-1:dDot;
                    delete pSunray;
                    // calculate solarrelativedir dot gradient * dParam (dParam: Temp amplitude)
                    double dSolar = dDot*fParam;
                    
                    double dTemp0 = m_pQMRTemp->getDValue(dTheta, dPhi);
                    if (m_bVerbose) {
                        printf("Temperature: %f\n", dTemp0);
                        printf("Sunray:      %f, %f, %f\n", pSunray->m_fX,  pSunray->m_fY,  pSunray->m_fZ);
                        printf("Gradient:    %f, %f, %f\n", vGradient.m_fX, vGradient.m_fY, vGradient.m_fZ);
                        printf("Dot:         %f\n", dDot);
                        printf("Param:       %f\n", fParam);
                        printf("Solar:       %f\n", dSolar);
                    }
                    double dTempNew = bAccumulateTemp?m_pQMRTemp->getDValue(dTheta, dPhi)+dSolar:dSolar;
                    
                    if (!isfinite(dTempNew)) {
                        //    printf("BadVal (%f) at %d,%d\n", dTemp0, iX, iY);
                        dTempNew = dNaN;
                    }
                    if (dDot > 1) printf("Dot>1 at [%d,%d]\n", iX, iY); 
                    m_ppData[iY][iX] = (float) dTempNew;
                    
                    if ((iX == 283) && (iY == 481)) {
                        //                        printf("at %d, %d: Alt %f, Temp %f\n", iX, iY, m_pQMRAltitude->getDValue(iX, iY), dTempNew);
                        m_bVerbose = false;
                    }
                } else {
                    m_ppData[iY][iX] = dNaN;
                }
            }
        }
    }

    return iResult;
}


//--------------------------------------------------------------------------------------------------
// findPointAt
//
//   find a point at distance d from Q, at anangle of alpha from north (westward)
//   to meridian M through Q.
//
int GradiTemp::findPointAt(Vec3D vQ, double dD, double dAlpha, Vec3D *pvP) {
    int iResult = 0;
    // meridian rotation
    double dNQx =  vQ.m_fY;
    double dNQy = -vQ.m_fX;
    double dScale = sqrt(dNQx*dNQx+dNQy*dNQy);
    // ass: dScale > 0
    dNQx /= dScale;
    dNQy /= dScale;
   
    Vec3D vM(dNQx, dNQy, 0);

    // create a point at distance d north of Q (distance on unit circle = angle)
    Quat *pM = Quat::makeRotation(dD, vM);
    Vec3D *pP0 = pM->apply(&vQ);

    delete pM;
    /*if (m_bVerbose) {
    printf("Point north of (%f,%f,%f): (%f,%f,%f)\n", vQ.m_fX, vQ.m_fY, vQ.m_fZ, pP0->m_fX, pP0->m_fY, pP0->m_fZ);
    DPOINT *pDPQ = cart2Polar(&vQ);
    DPOINT *pDPP0 = cart2Polar(pP0);
    printf("                  (%f,%f):    (%f,%f)\n", pDPQ->first, pDPQ->second, pDPP0->first, pDPP0->second);
    }*/

    // rotate this point about axis Q 
    Quat *pQ = Quat::makeRotation(dAlpha, vQ);
    //if (m_bVerbose)        printf("Rotation quat (%f,%f,%f,%f)\n", pQ->m_fR, pQ->m_fI, pQ->m_fJ, pQ->m_fK);
    Vec3D *pvP1 = pQ->apply(pP0); 
    //if (m_bVerbose)    printf("rotated: (%f,%f,%f)\n", pvP1->m_fX, pvP1->m_fY, pvP1->m_fZ);
    delete pQ;
    delete pP0;
    
    DPOINT *pdP = cart2PolarD(pvP1);
    double dAltitude = m_pQMRAltitude->getDValue(pdP->first, pdP->second);
    if (m_bVerbose) printf("Orig Altitude at (%f,%f) : %e\n", pdP->first, pdP->second, dAltitude);
    if (dAltitude < -5) {
        //       dAltitude = dNaN;
    }
    dAltitude = (dAltitude+RADIUS_EARTH)/RADIUS_EARTH;
    if (m_bVerbose) printf("oriAltitude at (%f,%f) : %e\n", pdP->first, pdP->second, dAltitude);
    delete pdP;
    pvP1->scale(dAltitude);
        if (m_bVerbose) printf("scaled: (%f,%f,%f)\n", pvP1->m_fX, pvP1->m_fY, pvP1->m_fZ);
    pvP->m_fX = pvP1->m_fX;
    pvP->m_fY = pvP1->m_fY;
    pvP->m_fZ = pvP1->m_fZ;
    delete pvP1;
    return iResult;
}    

//--------------------------------------------------------------------------------------------------
// getGradient
//
//   find the gradient of the point at spherical coordinates dCenter by measuring the altitudes
//   of six equidistant points at distance dDistance from dCenter.
//  
int GradiTemp::getGradient(DPOINT dCenter, double dDistance, Vec3D *pvGradient) {
    int iResult = 0;
    Vec3D *pCenter = polarD2Cart(&dCenter);

 
    /*   
    double dPx = 0;
    double dPy = 0;
    double dPz = 0;
    */
    double dAngle = -M_PI/2;
    Vec3D avNeighbors[6];
    for (int i = 0; i < 6; i++) {

        findPointAt(pCenter, dDistance, dAngle, &avNeighbors[i]);
        dAngle += M_PI/3;
    }

    delete pCenter;

    Vec3D vAxis1(&avNeighbors[0]);
    Vec3D vAxis2(&avNeighbors[2]);
    Vec3D vNegC1(&avNeighbors[4]);

    vNegC1.scale(-1);
    vAxis1.add(&vNegC1);
    vAxis2.add(&vNegC1);

    Vec3D *pGrad1 = vAxis1.crossProduct(&vAxis2);
    pGrad1->normalize();
        if (m_bVerbose) printf("Grad1: (%f, %f, %f) \n", pGrad1->m_fX, pGrad1->m_fY, pGrad1->m_fZ);

    Vec3D vAxis3(&avNeighbors[1]);
    Vec3D vAxis4(&avNeighbors[3]);
    Vec3D vNegC2(&avNeighbors[5]);

    vNegC2.scale(-1);
    vAxis3.add(&vNegC2);
    vAxis4.add(&vNegC2);
    
    Vec3D *pGrad2 = vAxis3.crossProduct(&vAxis4);
    pGrad2->normalize();
        if (m_bVerbose) printf("Grad2: (%f, %f, %f) \n", pGrad2->m_fX, pGrad2->m_fY, pGrad2->m_fZ);

    pGrad1->add(pGrad2);
    pGrad1->scale(0.5);

    pvGradient->m_fX = pGrad1->m_fX;
    pvGradient->m_fY = pGrad1->m_fY;
    pvGradient->m_fZ = pGrad1->m_fZ;

    delete pGrad1;
    delete pGrad2;

        if (m_bVerbose) printf("Grad: (%f, %f, %f) \n", pvGradient->m_fX, pvGradient->m_fY, pvGradient->m_fZ);


    if (!isfinite(pvGradient->m_fX)) {
        iResult = -1;
        /*
        printf("Gradient = (%f,%f,%f)\n", pvGradient->m_fX, pvGradient->m_fY, pvGradient->m_fZ);
        printf("Center   = (%f,%f)\n", dCenter.first, dCenter.second);
        for (int i = 0; i < 6; i++) {
            printf("   Neighbor[%d]: (%f,%f,%f)\n", i, avNeighbors[i].m_fX, avNeighbors[i].m_fY, avNeighbors[i].m_fX);
            
        }
        */
    }
    return iResult;
}
