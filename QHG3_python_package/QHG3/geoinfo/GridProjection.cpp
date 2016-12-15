#include <stdio.h>
#include <math.h>
#include "Vec3D.h"
#include "GeoInfo.h"
#include "Projector.h"
#include "GridProjection.h"


//----------------------------------------------------------------------------
// constructor
//
GridProjection::GridProjection(int iGridW,
                               int iGridH,
                               double dWidth,
                               double dHeight,
                               double dRadius,
                               Projector *pProj,
                               bool bDeleteProj) 
    : m_pProj(pProj),
      m_pProjGrid(NULL),
      m_bDeleteProj(bDeleteProj),
      m_bDeleteProjGrid(true) {

    m_pProjGrid = new ProjGrid(iGridW, iGridH, dWidth, dHeight,-iGridW/2, -iGridH/2, dRadius);
    init();
}

//----------------------------------------------------------------------------
// constructor
//
GridProjection::GridProjection(const ProjGrid *ppg, 
                               Projector *pProj,
                               bool bDeleteGrid,
                               bool bDeleteProj) 
    : m_pProj(pProj),
      m_pProjGrid(ppg),
      m_bDeleteProj(bDeleteProj),
      m_bDeleteProjGrid(bDeleteGrid) {

    init();
}


//----------------------------------------------------------------------------
// init
//
void GridProjection::init() {
    if (m_pProjGrid != NULL) {
        double dWidth  = m_pProjGrid->m_dRealW/m_pProjGrid->m_dRadius;
        double dHeight = m_pProjGrid->m_dRealH/m_pProjGrid->m_dRadius;
        m_dDeltaX = dWidth/m_pProjGrid->m_iGridW;
        m_dDeltaY = dHeight/m_pProjGrid->m_iGridH;
        m_dXOffs   = m_pProjGrid->m_dOffsX;
        m_dYOffs   = m_pProjGrid->m_dOffsY;
    } else {
        printf("No proj data passed\n");
    }
}


//----------------------------------------------------------------------------
// destructor
//
GridProjection::~GridProjection() {

    if (m_bDeleteProj) {
        delete m_pProj;
    }
    if (m_bDeleteProjGrid) {
        delete m_pProjGrid;
    }
    
}

#define EPS 1e-8

//----------------------------------------------------------------------------
// gridToSphere
//
bool GridProjection::gridToSphere(double dGridX, 
                                  double dGridY, 
                                  double &dLon, 
                                  double &dLat) {
    bool bOK = true;

    // from grid to (normalized) plane
    //    double dV = m_dXMin + m_dDeltaX*dGridX;
    //    double dW = m_dYMin + m_dDeltaY*dGridY;
    double dV = m_dDeltaX*(m_dXOffs+dGridX);
    double dW = m_dDeltaY*(m_dYOffs+dGridY);
    // project from plane to sphere
    m_pProj->plane2Sphere(dV, dW, dLon, dLat);
    if (m_pProj->getID() != PR_LINEAR) {
        if (dLon < -M_PI-EPS) {
            dLon += 2*M_PI;
        } else if (dLon > M_PI+EPS) {
            dLon -= 2*M_PI;
        }
        if (dLat < -M_PI/2-EPS) {
            dLat += M_PI;
        } else if (dLat > M_PI/2+EPS) {
            dLat -= M_PI;
        }
    }
    //    printf("g %f,%f -> p %f,%f -> s %f, %f\n", dGridX, dGridY, dV, dW, dLon, dLat);
    /*
    if ((dGridX == 360) && (dGridY == 360)) {
        printf("gridToSphere type %s, center %e,%e\n", m_pProj->getName(), m_pProj->getLambda0(), m_pProj->getPhi0());
        printf("gridToSphere delta (%e,%e) min (%e,%e)\n", m_dDeltaX, m_dDeltaY, m_dXOffs,m_dYOffs);
        printf("gridToSphere %f,%f -> %e, %e -> %e, %e\n", dGridX, dGridY, dV, dW, dLon, dLat);
    } 
    */
    return bOK;
}

//----------------------------------------------------------------------------
// sphereToGrid
//
bool GridProjection::sphereToGrid(double  dLon, 
                                  double  dLat,
                                  double &dGridX, 
                                  double &dGridY) {
    bool bOK = true;
    
    double dV;
    double dW;

    /*
    // convert lat lon to radians
    double dLambda = M_PI*dLon/180;
    double dPhi    = M_PI*dLat/180;
    */



    // project from sphere to plane
    m_pProj->sphere2Plane(dLon, dLat, dV, dW);

   
    // from (normalized) plane to grid
    //    dGridX = (dV - m_dXMin)/m_dDeltaX;
    //    dGridY = (dW - m_dYMin)/m_dDeltaY;
    dGridX = dV/m_dDeltaX - m_dXOffs;
    dGridY = dW/m_dDeltaY - m_dYOffs;

    if (m_pProj->getID() != PR_LINEAR) {
        if (dGridX < 0) {
            dGridX += m_pProjGrid->m_iGridW;
        } else if (dGridX >=  m_pProjGrid->m_iGridW) {
            dGridX -= m_pProjGrid->m_iGridW;
        }
        if (dGridY < 0) {
            dGridY += m_pProjGrid->m_iGridH;
        } else if (dGridY >= m_pProjGrid->m_iGridH) {
            dGridY -= m_pProjGrid->m_iGridH;
        }
    }
    return bOK;
}

/*
bool GridProjection::getCellArea(double  dGridXE, 
                                 double  dGridYN, 
                                 double  dGridXW, 
                                 double  dGridYS,
                                 double &dArea) {
    bool bOK = false;
    if ((m_pProj->getID() == PR_LAMBERT_AZIMUTHAL_EQUAL_AREA) ||
        (m_pProj->getID() == PR_CYLINDRICAL_EQUAL_AREA) ||
        (m_pProj->getID() == PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA)) {

        double dW = fabs(dGridXE - dGridXW)*m_pProjGrid->m_dRealW/m_pProjGrid->m_iGridW;
        double dH = fabs(dGridYN - dGridYS)*m_pProjGrid->m_dRealH/m_pProjGrid->m_iGridH;
        dArea = dW*dH;
   

        bOK = true;
    }
    
    return bOK;
}
*/

//----------------------------------------------------------------------------
// calcArcPlane
//   calculate the normal of the plane defined by the two given points
//   and the origin
void calcArcPlane(double dLon0, double dLat0,
                  double dLon1, double dLat1,
                  Vec3D **ppvN) {

    Vec3D A(cos(dLon0)*cos(dLat0), sin(dLon0)*cos(dLat0), sin(dLat0));
    Vec3D B(cos(dLon1)*cos(dLat1), sin(dLon1)*cos(dLat1), sin(dLat1));
    *ppvN = A.crossProduct(&B);
    (*ppvN)->normalize();

}

//----------------------------------------------------------------------------
// calcTriangleArea
//   calculate a spherical triangle's area using the spherical excess
//   
double calcTriangleArea(double dLon0, double dLat0,
                        double dLon1, double dLat1,
                        double dLon2, double dLat2) {

    // double dArea = dNaN;

    // calculate the normals on the planes defined by two verticews and 0
    Vec3D *pN0=NULL;
    calcArcPlane(dLon0, dLat0, dLon1, dLat1, &pN0); 
    Vec3D *pN1=NULL;
    calcArcPlane(dLon1, dLat1, dLon2, dLat2, &pN1); 
    Vec3D *pN2=NULL;
    calcArcPlane(dLon2, dLat2, dLon0, dLat0, &pN2); 

    // angle between the normals is PI - angle between the planes,
    // i.e. angles of the triangle
    double a01, a12, a20;
    a01 = M_PI - pN0->getAngle(pN1);
    a12 = M_PI - pN1->getAngle(pN2);
    a20 = M_PI - pN2->getAngle(pN0);

    delete pN0;
    delete pN1;
    delete pN2;
    // spherical excess: alpha+beta+gamma-pi
    return a01+a12+a20-M_PI;
    // alternative: a_xy=pN_x->getAngle(pN_y)
    // excess = 2*PI-(a_01+a12+a20)
}

//----------------------------------------------------------------------------
// getCellArea
//   calculate the area of a grid-rectangle's preimage.
//   The preimage of this rectangle is assumed to be a spherical rectangle
//   the area of which is the sum of the areas of two spherical triangles.
//   
bool GridProjection::getCellArea(double  dGridXE, 
                                 double  dGridYN, 
                                 double  dGridXW, 
                                 double  dGridYS,
                                 double &dArea) {
    bool bOK = true;
    double dLon[4];
    double dLat[4];
    
    gridToSphere(dGridXW, dGridYS, dLon[0], dLat[0]);
    gridToSphere(dGridXE, dGridYS, dLon[1], dLat[1]);
    gridToSphere(dGridXE, dGridYN, dLon[2], dLat[2]);
    gridToSphere(dGridXW, dGridYN, dLon[3], dLat[3]);

    dArea = calcTriangleArea(dLon[0], dLat[0], dLon[1], dLat[1], dLon[2], dLat[2]) + 
            calcTriangleArea(dLon[0], dLat[0], dLon[2], dLat[2], dLon[3], dLat[3]);
   
    return bOK;
}

