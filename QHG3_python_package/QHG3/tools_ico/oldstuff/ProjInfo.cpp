#include <string.h>
#include <strings.h>

#include "Observable.h"

#include "strutils.h"
#include "GeoInfo.h"
#include "GridProjection.h"

#include "LookUp.h"
#include "LookUpFactory.h"

#include "VertexLinkage.h"
#include "Surface.h"
#include "PolyFace.h"
#include "Vec3D.h"

#include "HeaderBase.h"
#include "SnapHeader.h"
#include "PopHeader.h"

#include "QMapHeader.h"
#include "ValReader.h"
#include "QMapUtils.h"

#include "ValueProvider.h"
#include "ProjInfo.h"

#include "icoutil.h"
#include "trackball.h"
#include "interquat.h"

#include "Lattice.h"
#include "Interpolator.h"

#include "notification_codes.h"

#include "SimpleFace.h"
//-----------------------------------------------------------------------------
// constructor 
//
ProjInfo::ProjInfo(IQOverlay *pOverlay) 
    : m_pSurface(NULL),
      m_pGP(NULL),
      m_pLU(NULL),
      m_pVR(NULL),
      m_pVP(NULL),
      m_bFlat(false),
      m_pOverlay(pOverlay) {

    float q0[] = {0,0,0,1};
    setQuat(q0);

    double r01[] = {0, 1};
    setLU(LOOKUP_RAINBOW, 2, r01);

}

//-----------------------------------------------------------------------------
// destructor 
//
ProjInfo::~ProjInfo() {
    if (m_pLU != NULL) {
        delete m_pLU;
    }
    if (m_pGP != NULL) {
        delete m_pGP;
    }

}



//-----------------------------------------------------------------------------
// setLU 
//
void ProjInfo::setLU(int iLUType, int iNumParams, double *adLUParams) {
    if (m_pLU != NULL) {
        delete m_pLU;
        m_pLU = NULL;
    }
    m_pLU = LookUpFactory::instance()->getLookUp(iLUType, adLUParams, iNumParams);
    // notify LUChange
    notifyObservers(NOTIFY_LU_CHANGE,NULL);
    if (readyForAction()) {
        notifyObservers(0, (void*) NOTIFY_ALL_SET);
    }
}

//-----------------------------------------------------------------------------
// setGP 
//
void ProjInfo::setGP(ProjType *pPT, ProjGrid *pPG) {
    if (m_pGP != NULL) {
        delete m_pGP;
        m_pGP = NULL;
    }

    Projector *pProj = GeoInfo::instance()->createProjector(pPT);
    m_pGP = new GridProjection(pPG, pProj, true, true);

    notifyObservers(0, (void*) NOTIFY_GP_SET);
    if (readyForAction()) {
        notifyObservers(0, (void*) NOTIFY_ALL_SET);
    }
}

//-----------------------------------------------------------------------------
// setGP 
//
void ProjInfo::setQuat(float q[4]) {
    memcpy(m_Quat, q, 4*sizeof(float));
    qinvert(m_Quat, m_QuatInv);
}

//-----------------------------------------------------------------------------
// setData 
//
void ProjInfo::setData(ValueProvider *pVP, bool bForceCol) {
    m_pVP = pVP;
    printf("PI going to notify...\n");
    if (m_pVP != NULL) {
        if (bForceCol) {
            double dMin;
            double dMax;
            m_pVP->getRange(&dMin, &dMax);
            m_pLU->m_dMinLevel = dMin;
            m_pLU->m_dMaxLevel = dMax;
        }
        notifyObservers(0, (void*) NOTIFY_NEW_DATA);
        notifyObservers(0, (void*) NOTIFY_DATA_LOADED);

    }
}
    
//-----------------------------------------------------------------------------
// fillFlatInterpolated 
//
int ProjInfo::fillFlatInterpolated(int iW, int iH, unsigned char *pImgData) {
    int iResult = 0;

    unsigned char uRed;
    unsigned char uGreen;
    unsigned char uBlue;
    unsigned char uAlpha;
    
    tbox *pBox = m_pSurface->getBox();
    int iNumX  = (int) pBox->dLonMax;
    int iNumY  = (int) pBox->dLatMax;
    
    double *adGridX = new double[iNumX];
    double *adGridY = new double[iNumY];
    double **aadVals = new double *[iNumY];
    int iC = 0;
    for (int i = 0; i < iNumY; i++) {
        adGridY[i] = i;
            aadVals[i] = new double[iNumX];
            for (int j = 0; j < iNumX; j++) {
                aadVals[i][j] =  m_pVP->getValue(iC);
                iC++;
            }
    }
    for (int j = 0; j < iNumX; j++) {
        adGridX[j] = j;
    }
    
    Interpolator *pI = new Interpolator(adGridX, iNumX, adGridY, iNumY, aadVals);
    
    iC = 0;
    for (int i = 0; i < iH; i++) {
        double dY = (1.0*i*iNumY)/iH;
        for (int j = 0; j < iW; j++) {
            double dX = (1.0*j*iNumX)/iW;
            double dVal = pI->bilin(dX, dY);
            m_pLU->getColor(dVal, uRed,uGreen,uBlue,uAlpha);
            pImgData[iC*3]   = uRed;
            pImgData[iC*3+1] = uGreen;
            pImgData[iC*3+2] = uBlue;
            iC++;
            }
    }
    delete pI;
    delete[] adGridX;
    delete[] adGridY;
    for (int i = 0; i < iNumY; i++) {
        delete[] aadVals[i];
    }
    delete[] aadVals;
    
    return iResult;
}
    
//-----------------------------------------------------------------------------
// fillRGBInterpolated 
//
int ProjInfo::fillRGBInterpolated(int iW, int iH, unsigned char *pImgData) {
    int iResult = 0;
    printf("[ProjInfo::fillRGBInterpolated] %dx%d\n", iW, iH);

    int iC = 0;
 
    // triangular barycentric interpolation
    double dLon;
    double dLat;
    if (m_bFlat) {
        
        iResult = fillFlatInterpolated(iW, iH, pImgData);


    } else {

        for (int i = 0; i < iH; i++) {
            for (int j = 0; j < iW; j++) {

                // make sure preview image is right side up (i -> iH-i-1)
                m_pGP->gridToSphere(j, iH-i-1, dLon, dLat);
        
                // prevent strange color effect at dLon == 0
                if (dLon == 0) {
                    dLon += 1e-5;
                }

                iResult = putColorBarycentric(dLon, dLat, 3*iC, pImgData);
                
                iC++;
            }
        }
    }
    printf("Converted %d gid coords to spherical\n", iC);
    return iResult;
}

//-----------------------------------------------------------------------------
// fillRGBInterpolated 
//
int ProjInfo::fillRGBInterpolatedQ(int iW, int iH, unsigned char *pImgData) {
    int iResult = 0;
    printf("[ProjInfo::fillRGBInterpolatedQ]\n");

    int iC = 0;
 
    if (m_bFlat) {
        iResult = fillFlatInterpolated(iW, iH, pImgData);
    } else {
        // triangular barycentric interpolation
        double dLon;
        double dLat;

        // to align our projection to the opengl projection
        // we have to rotate everything around z by 90°
        // and then around x by 90° and finally bay the quaternion


        // x-rotation
        float qX[4];
        qX[0] = -sin(M_PI/4);
        qX[1] = 0;
        qX[2] = 0;
        qX[3] = cos(M_PI/4);
        // inverse of qX
        float qXI[4];
        qXI[0] = sin(M_PI/4);
        qXI[1] = 0;
        qXI[2] = 0;
        qXI[3] = cos(M_PI/4);
    
        // combined x-rotation and OGL rotation
        float qQ[4];
        add_quats(qX, m_Quat, qQ);
        // inverse of qQ
        float qQI[4];
        add_quats(m_QuatInv, qXI, qQI);

        for (int i = 0; i < iH; i++) {
            for (int j = 0; j < iW; j++) {
                // make sure preview image is right side up
                m_pGP->gridToSphere(j, iH-i-1, dLon, dLat);
            
                // z-rotation is easy
                dLon -= M_PI/2;
                // convert Lon, Lat to (x,y,z)
                float aPos1[4];
                float aPos2[4];
          
                float fc = cos(dLat);
                aPos1[0] = fc*cos(dLon);
                aPos1[1] = fc*sin(dLon);
                aPos1[2] = sin(dLat);
                aPos1[3] = 0;

                float qTemp[4];
                // transform (x,y,z) with x-rot quaternion (x',y',z')
                add_quats(qQI, aPos1,  qTemp);
                add_quats(qTemp, qQ, aPos2);

                Vec3D v(aPos2[0], aPos2[1], aPos2[2]);

                // convert (x',y',z') to lon, lat
                double dLon1;
                double dLat1;
                cart2Sphere(&v, &dLon1, &dLat1);
                iResult = putColorBarycentric(dLon1, dLat1, 3*iC, pImgData);

                iC++;
            }
        }
    }  
    return iResult;
}

//-----------------------------------------------------------------------------
// calcBary 
//
void calcBary(PolyFace *pF, Vec3D v,  double *pdL1, double *pdL2) {
    pF->planify(&v);
    // calc bary coords
    Vec3D A00(pF->getVertex(2));
    A00.subtract(pF->getVertex(1));
    Vec3D A01(v);
    A01.subtract(pF->getVertex(2));
    *pdL1 = A00.getCrossSize(&A01)/(2*pF->getArea());

    Vec3D A10(pF->getVertex(0));
    A10.subtract(pF->getVertex(2));
    Vec3D A11(v);
    A11.subtract(pF->getVertex(0));
    *pdL2 = A10.getCrossSize(&A11)/(2*pF->getArea());
    
}

//-----------------------------------------------------------------------------
// putColorBarycentric 
//
int ProjInfo::putColorBarycentric(double dLon, double dLat, int iC3, unsigned char *pImgData) {
    int iResult = 0;

    unsigned char uRed;
    unsigned char uGreen;
    unsigned char uBlue;
    unsigned char uAlpha;
    unsigned char uRNaN;
    unsigned char uGNaN;
    unsigned char uBNaN;
    // since NaN generally appears very often, we calculate it once
    m_pLU->getColor(dNaN, uRNaN,uGNaN,uBNaN,uAlpha);
              
    int iOK = 0;
    int iBadNode = 0;
    int iNoFace =0;
    int iNanCoords =0;
    if (m_pSurface != NULL) {
        Vec3D v(cos(dLat)*cos(dLon),
                cos(dLat)*sin(dLon),
                sin(dLat));

        // find containing face of current lon/lat pair
        PolyFace *pF = m_pSurface->findFace(dLon, dLat);
        if (pF != NULL) {

            // calclate barycentric coordinates of point
            double dL[3];
            calcBary(pF, v, dL, dL+1);
            dL[2] = 1-dL[0]-dL[1];
            
            // do the interpolation
            bool bHitPath = false;
            double dVal = 0;
          
            for (int k = 0; k < 3; k++) {
                gridtype lID = pF->getVertexID(k);
                
                if (lID < 0) {
                    //                printf("vertex has no id!\n");
                    dVal = dNaN;
                } else {
                    dVal += dL[k]*m_pVP->getValue(lID);

                    if (m_pOverlay->hasData() && !bHitPath) {
                        if (m_pOverlay->contains(lID)) {
                            bHitPath = true;
                        }
                    }

                }
            }

            if (isnan(dVal)) {
                iBadNode++;
            } else {
                iOK++;
            }    
            
            // put color for value into array
            m_pLU->getColor(dVal, uRed,uGreen,uBlue,uAlpha);
            pImgData[iC3]   = uRed;
            pImgData[iC3+1] = uGreen;
            pImgData[iC3+2] = uBlue;
         
            if (bHitPath) {
                pImgData[iC3]   = 0;
                pImgData[iC3+1] = 0;
                pImgData[iC3+2] = 0;
            } 


        } else {
            // point outside of projection (nan coords) or other bad error
            // put color for NaN
            
            if (isnan(dLon) || isnan(dLat)) {
                iNanCoords++;
            } else {
                iNoFace++;
            }
            
            pImgData[iC3]   = uRNaN;
            pImgData[iC3+1] = uGNaN;
            pImgData[iC3+2] = uBNaN;
        }
    } else {
        printf("ProjInfO: m_pSurf is NULL\n");
    }
    return iResult;
}
    

//-----------------------------------------------------------------------------
// putColorDirect 
//
int ProjInfo::putColorDirect(double dLon, double dLat, int iC3, unsigned char *pImgData) {
    int iResult = 0;

    unsigned char uRed;
    unsigned char uGreen;
    unsigned char uBlue;
    unsigned char uAlpha;
    unsigned char uRNaN;
    unsigned char uGNaN;
    unsigned char uBNaN;
    // since NaN generally appears very often, we calculate it once
    m_pLU->getColor(dNaN, uRNaN,uGNaN,uBNaN,uAlpha);
              
    int iOK = 0;
    int iBadNode = 0;
    int iNoFace =0;
    int iNanCoords =0;
    if (m_pSurface != NULL) {

        Vec3D v(cos(dLat)*cos(dLon),
                cos(dLat)*sin(dLon),
                sin(dLat));

        // find containing face of current lon/lat pair
        PolyFace *pF = m_pSurface->findFace(dLon, dLat);
        if (pF != NULL) {
            gridtype lID = m_pSurface->findNode(pF->closestVertex(&v));
            double dVal = m_pVP->getValue(lID);

            if (isnan(dVal)) {
                iBadNode++;
            } else {
                iOK++;
            }    
            
            // put color for value into array
            m_pLU->getColor(dVal, uRed,uGreen,uBlue,uAlpha);
            pImgData[iC3]   = uRed;
            pImgData[iC3+1] = uGreen;
            pImgData[iC3+2] = uBlue;
         
            if (m_pOverlay->hasData()) {
                if (m_pOverlay->contains(lID)) {
                    pImgData[iC3]    = 0; // overlaycolR
                    pImgData[iC3+1]  = 0; // overlaycolG  
                    pImgData[iC3+2]  = 0; // overlaycolB
                }
            } 
        } else {
            // point outside of projection (nan coords) or other bad error
            // put color for NaN
            
            if (isnan(dLon) || isnan(dLat)) {
                iNanCoords++;
            } else {
                iNoFace++;
            }
            
            pImgData[iC3]   = uRNaN;
            pImgData[iC3+1] = uGNaN;
            pImgData[iC3+2] = uBNaN;
        }
    } else {
        printf("ProjInfo: m_pSurface is NULL\n");
    }
    return iResult;
}
    
    
//-----------------------------------------------------------------------------
// readyForAction 
//
bool ProjInfo::readyForAction() {
    return (m_pSurface != NULL) && (m_pLU != NULL) && (m_pGP!= NULL);
}

//-----------------------------------------------------------------------------
// calcImage 
//
int ProjInfo::calcImage(int iW, int iH, const char *pFileName) {
    return -1;
}

//-----------------------------------------------------------------------------
// getRange 
//
void ProjInfo::getRange(double *pdMin, double *pdMax) {
    if (m_pVP != NULL) {
        m_pVP->getRange(pdMin, pdMax);
    } else {
        *pdMin = 0;
        *pdMax = 1;
    }
}

//-----------------------------------------------------------------------------
// getLURange 
//
void ProjInfo::getLURange(double *pdMin, double *pdMax) {
    *pdMin = m_pLU->m_dMinLevel;
    *pdMax = m_pLU->m_dMaxLevel;
}


//-----------------------------------------------------------------------------
// fillRGBPoints 
//
int ProjInfo::fillRGBPoints(int iW, int iH, unsigned char *pImgData) {
    int iResult = 0;
    printf("[ProjInfo::fillRGBPoints] %dx%d\n", iW, iH);

    int iC = 0;
 
    // triangular barycentric interpolation
    double dLon;
    double dLat;
    if (m_bFlat) {
        unsigned char uRed;
        unsigned char uGreen;
        unsigned char uBlue;
        unsigned char uAlpha;

        for (int i = 0; i < iH; i++) {
            for (int j = 0; j < iW; j++) {
                double dVal = m_pVP->getValue(iC);
                 m_pLU->getColor(dVal, uRed,uGreen,uBlue,uAlpha);
                 pImgData[iC*3]   = uRed;
                 pImgData[iC*3+1] = uGreen;
                 pImgData[iC*3+2] = uBlue;
                 iC++;
            }
        }
    } else {
        for (int i = 0; i < iH; i++) {
            for (int j = 0; j < iW; j++) {
                
                // make sure preview image is right side up
                m_pGP->gridToSphere(j, iH-i-1, dLon, dLat);
                
                iResult = putColorDirect(dLon, dLat, 3*iC, pImgData);
                
                iC++;
            }
        }
    }
    printf("Converted %d gid coords to spherical\n", iC);
    return iResult;
}


//-----------------------------------------------------------------------------
// translateValArr 
//
void ProjInfo::translateValArr(int iW, int iH, double **dTempVal, unsigned char *pImgData) {
    unsigned char uRed;
    unsigned char uGreen;
    unsigned char uBlue;
    unsigned char uAlpha;
    unsigned char uRNaN;
    unsigned char uGNaN;
    unsigned char uBNaN;

    m_pLU->getColor(dNaN, uRNaN,uGNaN,uBNaN,uAlpha);
              
    int iC3 = 0;
    // translate to colors
    for (int i = 0; i < iH; i++) {
        for (int j = 0; j < iW; j++) {
            // calculate color of temp[i][j] and place to m_pImg
            if (isnan(dTempVal[i][j])) {
                pImgData[iC3++] = uRNaN;
                pImgData[iC3++] = uGNaN;
                pImgData[iC3++] = uBNaN;
            } else {
                m_pLU->getColor(dTempVal[i][j], uRed,uGreen,uBlue,uAlpha);
                pImgData[iC3++] = uRed;
                pImgData[iC3++] = uGreen;
                pImgData[iC3++] = uBlue;
            }
        }
    }
}

/*
//-----------------------------------------------------------------------------
// setOverlay 
//
void ProjInfo::setOverlay(std::vector<gridtype> vOverlays) {
    m_sOverlay.insert(vOverlays.begin(), vOverlays.end());
    printf("Overlay set - %zd elements\n", m_sOverlay.size());
    notifyObservers(NOTIFY_LU_CHANGE, NULL);
};

//-----------------------------------------------------------------------------
// clearOverlay 
//
void ProjInfo::clearOverlay() { 
    m_sOverlay.clear();
    notifyObservers(NOTIFY_LU_CHANGE, NULL);
}
*/
