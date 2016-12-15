#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Vec3D.h"
#include "Quat.h"
#include "PieCreator.h"


#define STAR_SPIKE  2.0
#define BARS_RECT_H 2.0
#define BARS_RECT_W 2.0

//----------------------------------------------------------------------------
// constructor
//
PieCreator *PieCreator::createInstance(uint iDim, uint iNumTri) {
    PieCreator *pPC = new PieCreator();
    int iResult = pPC->init(iDim, iNumTri);
    if (iResult != 0) {
        delete pPC;
        pPC = NULL;
    }
    return pPC;
}


//----------------------------------------------------------------------------
// constructor
//
PieCreator::PieCreator() 
    : m_iDim(0),
      m_iNumTri(0),
      m_aTriangleListPie(NULL),
      m_aTriangleListStar(NULL),
      m_aTriangleListBars(NULL),
      m_aBGTriangleListPie(NULL),
      m_aBGTriangleListStar(NULL),
      m_aBGTriangleListBars(NULL),
      m_dScale(1.0),
      m_dL(1.0),
      m_dMax(0),
      m_dSuperMax(0),
      m_dStarMaxSpike(STAR_SPIKE),
      m_dBarsRectWidth(BARS_RECT_W),
      m_dBarsRectHeight(BARS_RECT_H),
      m_bBG(true) {

}

//----------------------------------------------------------------------------
// destructor
//
PieCreator::~PieCreator() {
    if (m_aTriangleListPie != NULL) {
        delete[] m_aTriangleListPie;
    }
    if (m_aTriangleListStar != NULL) {
        delete[] m_aTriangleListStar;
    }
    if (m_aTriangleListBars != NULL) {
        delete[] m_aTriangleListBars;
    }

    if (m_aBGTriangleListPie != NULL) {
        delete[] m_aBGTriangleListPie;
    }
    if (m_aBGTriangleListStar != NULL) {
        delete[] m_aBGTriangleListStar;
    }
    if (m_aBGTriangleListBars != NULL) {
        delete[] m_aBGTriangleListBars;
    }

}

//----------------------------------------------------------------------------
// init
//
int PieCreator::init(uint iDim, uint iNumTri) {
    int iResult = 0;
    if ((iDim > 1) && (iNumTri > 2*iDim)) {
        m_iDim    = iDim;
        m_iNumTri = iNumTri;
        iResult = createAllTrianglesPie();
        if (iResult == 0) {
            iResult = createAllTrianglesStar();
            if (iResult == 0) {
                iResult = createAllTrianglesBars();
                if (iResult == 0) {
                    iResult = createAllBGTrianglesPie();
                    if (iResult == 0) {
                        iResult = createAllBGTrianglesStar();
                        if (iResult == 0) {
                            iResult = createAllBGTrianglesBars();
                        }
                    }
                }
            }
        }
    } else {
        iResult = -1;
        if (iDim <= 1) {
            printf("Dim must be greater than one\n");
        }
        if (iNumTri <= 2*iDim) {
            printf("NumTri must be greater than 2*Dim\n");
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createAllTrianglesPie
//   create triangles for pie.
//   start with triabgle lying on the x-axis; continue ccw
//
int PieCreator::createAllTrianglesPie() {
    int iResult = 0;

    Vec3D pPrev(1, 0, 0); 

 
    m_aTriangleListPie = new triangle[m_iNumTri];
    triangle *pT = m_aTriangleListPie;
    double dDelta = (2*M_PI)/m_iNumTri;
    for (uint i = 0; i < m_iNumTri; ++i) {
        Vec3D pNext(cos((i+1)*dDelta), sin((i+1)*dDelta), 0.0);
        //    + .
        //   /| .
        //  / | .
        // +  | .
        //  \ | .
        //   \| .
        //    + .
        pT->avVert[0].set(0.0, 0.0, 0.0);
        pT->avVert[1].set(&pPrev);
        pT->avVert[2].set(&pNext);
        pT++;
     
        pPrev.set(&pNext);
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// createAllBGTrianglesPie
//   create triangles for a ring around the pie.
//   start with segment lying on the x-axis; continue ccw
//
int PieCreator::createAllBGTrianglesPie() {
    int iResult = 0;
    float fFrame = 1.1;

    Vec3D pPrev(1, 0, 0); 


    m_aBGTriangleListPie = new triangle[2*m_iNumTri];
    triangle *pT = m_aBGTriangleListPie;
    double dDelta = (2*M_PI)/m_iNumTri;

    for (uint i = 0; i < m_iNumTri; ++i) {
        Vec3D pPrev2(pPrev);
        pPrev2.scale(fFrame);
        Vec3D pNext(cos((i+1)*dDelta), sin((i+1)*dDelta), 0.0);
        Vec3D pNext2(pNext);
        pNext2.scale(fFrame);

        //       + .
        //      /| .
        //     / | . 
        //    /  | .
        //   /   | .
        //  /    | .
        // + ----+ .
        pT->avVert[0].set(&pPrev);
        pT->avVert[1].set(&pPrev2);
        pT->avVert[2].set(&pNext2);
        pT++;

        // +-----+ .
        // |    /  .
        // |   /   . 
        // |  /    .
        // | /     .
        // |/      .
        // +       .
        pT->avVert[0].set(&pPrev);
        pT->avVert[1].set(&pNext2);
        pT->avVert[2].set(&pNext);
        pT++;
        
        pPrev.set(&pNext);
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// createAllTrianglesStar
//   create triangles for stars.
//   start with triangles bisected by the x-axis; continue ccw
//
int PieCreator::createAllTrianglesStar() {
    int iResult = 0;
    
    double dDelta = (2*M_PI)/m_iDim;
    Vec3D pPrev(cos(-dDelta/2), sin(-dDelta/2), 0.0);

    m_aTriangleListStar = new triangle[2*m_iDim];
    triangle *pT = m_aTriangleListStar;
    for (uint i = 0; i < m_iDim; ++i) {
        //        printf("Star %d:\n", i);
        Vec3D pNext(cos((i+0.5)*dDelta), sin((i+0.5)*dDelta), 0.0);
        //    + .
        //   /| .
        //  / | .
        // +  | . 
        //  \ | .
        //   \| .
        //    + .
        pT->avVert[0].set(0.0, 0.0, 0.0);
        pT->avVert[1].set(&pPrev);
        pT->avVert[2].set(&pNext);
        pT++;

        // +    .
        // |\   . 
        // | \  .
        // |  + .
        // | /  .
        // |/   . 
        // +    .
        pT->avVert[0].set(&pNext);
        pT->avVert[1].set(&pPrev);
        pT->avVert[2].set(2*cos((i+0.5)*dDelta), 2*sin((i+0.5)*dDelta), 0.0);
        pT++;

        pPrev.set(&pNext);
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// createAllBGTrianglesStar
//   create triangles for stars.
//   start with triangles bisected by the x-axis; continue ccw
//
int PieCreator::createAllBGTrianglesStar() {
    int iResult = 0;
    
    double dDelta = (2*M_PI)/m_iDim;
    //    Vec3D pPrev(2*cos(-dDelta/2), 2*sin(-dDelta/2), 0.0);
    Vec3D pPrev(2, 0, 0);

    m_aBGTriangleListStar = new triangle[m_iDim];
    triangle *pT = m_aBGTriangleListStar;
    for (uint i = 0; i < m_iDim; ++i) {
        //        printf("Star %d:\n", i);
        //        Vec3D pNext(2*cos((i+0.5)*dDelta), 2*sin((i+0.5)*dDelta), 0.0);
        Vec3D pNext(cos((i+1)*dDelta), 2*sin((i+1)*dDelta), 0.0);
        //    + .
        //   /| .
        //  / | .
        // +  | . 
        //  \ | .
        //   \| .
        //    + .
        pT->avVert[0].set(&pPrev);
        pT->avVert[1].set(&pNext);
        //        pT->avVert[2].set(cos(i*dDelta), sin(i*dDelta), 0.0);
        pT->avVert[2].set(cos((i+0.5)*dDelta), sin((i+0.5)*dDelta), 0.0);
        pT++;

 

        pPrev.set(&pNext);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createAllTrianglesBars
//   create triangles for bars.
//   start with triangles for uppermost bar
//
int PieCreator::createAllTrianglesBars() {
    int iResult = 0;

    double dH  = m_dBarsRectHeight/m_iDim;
    double dY0 = m_dBarsRectHeight/2.0;
    double dX0 = 0.0;
    double dW  = m_dBarsRectWidth;
    double dX1 = dX0+dW;
    m_aTriangleListBars = new triangle[2*m_iDim];
    triangle *pT = m_aTriangleListBars;

    for (uint i = 0; i < m_iDim; ++i) {
        double dY1 = dY0 - dH;

        // +--+ .
        //  \ | .
        //   \| .
        //    + .
        //
        pT->avVert[0].set(dX0, dY0, 0.0);
        pT->avVert[1].set(dX1, dY1, 0.0);
        pT->avVert[2].set(dX1, dY0, 0.0);
        pT++;

        //  +    .
        //  |\   .
        //  | \  .
        //  +--+ .
        //
        pT->avVert[0].set(dX0, dY0, 0.0);
        pT->avVert[1].set(dX0, dY1, 0.0);
        pT->avVert[2].set(dX1, dY1, 0.0);
        pT++;
        
        dY0 = dY1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// createAllBGTrianglesBars
//   create triangles for bars.
//   start with triangles for uppermost bar
//
int PieCreator::createAllBGTrianglesBars() {
    int iResult = 0;
    float fFrame = 1.1;
    double dH  = m_dBarsRectHeight/m_iDim;
    double dY0 = m_dBarsRectHeight/2.0;
    double dW  = m_dBarsRectWidth;
    double dX0 = dW;
    double dX1 = fFrame*dW;
    m_aBGTriangleListBars = new triangle[2*m_iDim+6];
    triangle *pT = m_aBGTriangleListBars;
 

    for (uint i = 0; i < m_iDim; ++i) {
        double dY1 = dY0 - dH;

        // +--+ .
        //  \ | .
        //   \| .
        //    + .
        //
        pT->avVert[0].set(dX0, dY0, 0.0);
        pT->avVert[1].set(dX1, dY1, 0.0);
        pT->avVert[2].set(dX1, dY0, 0.0);
        pT++;

        //  +    .
        //  |\   .
        //  | \  .
        //  +--+ .
        //
        pT->avVert[0].set(dX0, dY0, 0.0);
        pT->avVert[1].set(dX0, dY1, 0.0);
        pT->avVert[2].set(dX1, dY1, 0.0);
        pT++;
        
        dY0 = dY1;
    }

   // fist the frame around all
    double dD = 0.1*dW;

    double dYA = m_dBarsRectHeight/2.0;
    double dYB = m_dBarsRectHeight/2.0+dD;

    // rectangle at base
    pT->avVert[0].set(0.0, -dYA, 0.0);
    pT->avVert[1].set(0.0, dYA, 0.0);
    pT->avVert[2].set(-dD, dYA, 0.0);
    
    pT++;
    
    pT->avVert[0].set(0.0, -dYA, 0.0);
    pT->avVert[1].set(-dD, dYA, 0.0);
    pT->avVert[2].set(-dD, -dYA, 0.0);
    
    pT++;


    pT->avVert[0].set(-dD, dYA, 0.0);
    pT->avVert[1].set(dX1, dYA, 0.0);
    pT->avVert[2].set(dX1, dYB, 0.0);
    
    pT++;
    
    pT->avVert[0].set(-dD, dYA, 0.0);
    pT->avVert[1].set(dX1, dYB, 0.0);
    pT->avVert[2].set(-dD, dYB, 0.0);
   
    pT++;

    dYA = -dYA;
    dYB = -dYB;
    pT->avVert[0].set(-dD, dYA, 0.0);
    pT->avVert[1].set(dX1, dYB, 0.0);
    pT->avVert[2].set(dX1, dYA, 0.0);
    
    pT++;
    
    pT->avVert[0].set(-dD, dYA, 0.0);
    pT->avVert[1].set(-dD, dYB, 0.0);
    pT->avVert[2].set(dX1, dYB, 0.0);
    
    pT++;

    return iResult;
}

//----------------------------------------------------------------------------
// calcPieSectors
//
int PieCreator::calcPieSectors(double *dVals) {
    int iResult = -1;
     
    // create normalized array from iVals
    uint *c = new uint[m_iDim+1];
    memset(c, 0, (m_iDim+1)*sizeof(uint));
    double dSum = 0;
    for (uint i = 0; i < m_iDim; ++i) {
        dSum += fabs(dVals[i]);
    }
    if (dSum > 0) {
        c[0] =  0;
        for (uint i = 0; i < m_iDim; ++i) {
            c[i+1] = c[i]+(uint)round(fabs(1.0*m_iNumTri*dVals[i])/dSum);
        }
    } else {
        for (uint i = 0; i <= m_iDim; ++i) {
            c[i] = i*(uint)((1.0*m_iNumTri)/m_iDim);
        }
    }
    // just to be sure
    c[m_iDim] = m_iNumTri;


    for (uint i = 0; i < m_iDim; ++i) {
        m_mvTriangleList[i].clear();
        if (c[i+1] > c[i]) {
            m_mvTriangleList[i].insert(m_mvTriangleList[i].begin(), m_aTriangleListPie+c[i], m_aTriangleListPie+c[i+1]);
        }
    }

    m_vBGTriangleList.clear();
    if (m_bBG) {
        for (uint i = 0; i < 2*m_iNumTri; ++i) {
            m_vBGTriangleList.push_back(m_aBGTriangleListPie[i]);
        }
    }

    delete[] c;

    return iResult;
}


//----------------------------------------------------------------------------
// calcStarPoints
//
int PieCreator::calcStarPoints(double *dVals) {
    int iResult = 0;
    // TODO: "natural" scaling (e.g. maximum spike length)
    if (m_dMax == 0) {
        m_dMax = 1;
    }
    double dDelta = (2*M_PI)/m_iDim;
    triangle *pT  =  m_aTriangleListStar;
    
    for (uint i = 0; i < m_iDim; ++i) {
        triangle *pT0 = pT;
        // we have to use every second triangle

        pT++;    
        pT->avVert[2].set((1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*cos((i)*dDelta), 
                          (1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*sin((i)*dDelta),
                          0.0);

        pT++;

        m_mvTriangleList[i].clear();
        m_mvTriangleList[i].insert(m_mvTriangleList[i].begin(), pT0, pT);

    }



    m_vBGTriangleList.clear();
    if (m_bBG) {
        double dDelta = (2*M_PI)/m_iDim;
        triangle *pT  =  m_aBGTriangleListStar;
        
        for (uint i = 0; i < m_iDim; ++i) {
            Vec3D p0((1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*cos((i)*dDelta), 
                     (1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*sin((i)*dDelta),
                              0.0);
            Vec3D p1((1+fabs(m_dL*dVals[(i+1)%m_iDim]*m_dStarMaxSpike)/m_dMax)*cos((i+1)*dDelta), 
                     (1+fabs(m_dL*dVals[(i+1)%m_iDim]*m_dStarMaxSpike)/m_dMax)*sin((i+1)*dDelta),
                     0.0);
            

            pT->avVert[0].set((1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*cos((i)*dDelta), 
                              (1+fabs(m_dL*dVals[i]*m_dStarMaxSpike)/m_dMax)*sin((i)*dDelta),
                              0.0);
            pT->avVert[1].set((1+fabs(m_dL*dVals[(i+1)%m_iDim]*m_dStarMaxSpike)/m_dMax)*cos((i+1)*dDelta), 
                              (1+fabs(m_dL*dVals[(i+1)%m_iDim]*m_dStarMaxSpike)/m_dMax)*sin((i+1)*dDelta),
                              0.0);

            m_vBGTriangleList.push_back(*pT);
            pT++;
            
        }

    }



   
    return iResult;
}


//----------------------------------------------------------------------------
// calcBarHeights
//
int PieCreator::calcBarHeights(double *dVals) {
    int iResult = 0;
    // TODO: "natural" scaling (e.g. rectangle containing bars)
    m_dL = m_dBarsRectWidth/m_dMax;
 
    triangle *pT  =  m_aTriangleListBars;
    
    for (uint i = 0; i < m_iDim; ++i) {
        triangle *pT0 = pT;
        pT++;
        pT++;    

        m_mvTriangleList[i].clear();
        m_mvTriangleList[i].insert(m_mvTriangleList[i].begin(), pT0, pT);
    }
    
    trianglemap::iterator itS;
    for (itS = m_mvTriangleList.begin(); itS != m_mvTriangleList.end(); ++itS) {
        for (uint j = 0; j < itS->second.size(); j+=2) {
            triangle &t1 = itS->second[j];
            t1.avVert[1].m_fX = fabs(m_dL*dVals[itS->first]);
            t1.avVert[2].m_fX = fabs(m_dL*dVals[itS->first]);
            triangle &t2 = itS->second[j+1];
            t2.avVert[2].m_fX = fabs(m_dL*dVals[itS->first]);
        }
    }


    m_vBGTriangleList.clear();
    if (m_bBG) {
        triangle *pT  =  m_aBGTriangleListBars;
        for (uint i = 0; i < 2*m_iDim+6; ++i) {
            m_vBGTriangleList.push_back(*pT);
            pT++;
        }

        for (uint j = 0; j < m_vBGTriangleList.size()-6; j+=2) {
            
            triangle &t1 = m_vBGTriangleList[j];
            t1.avVert[0].m_fX = fabs(m_dL*dVals[j/2]);
            
            triangle &t2 = m_vBGTriangleList[j+1];
            t2.avVert[0].m_fX = fabs(m_dL*dVals[j/2]);
            t2.avVert[1].m_fX = fabs(m_dL*dVals[j/2]);
            
        }

        
    }
   

    return iResult;
}


//----------------------------------------------------------------------------
// calcRotation
//
Quat *PieCreator::calcRotation(Vec3D *pvPos) {
    // rotate triangles to be tangential to sphere
    // rotGlob rotates (0,0,1) to pvPos
    // rotLoc  rotates image of (1,0,0) around n as close as possible to Z 
    Vec3D vZ(0,0,1);
    Quat *rotLoc  = NULL;
    Quat *rotGlob = NULL;
    Quat *rotTot  = new Quat(1);

    Vec3D vPos(pvPos);
    vPos.normalize();
    Vec3D n1(pvPos);
    Vec3D xtarget(0,0,1);
    double dAngle = n1.getAngle(&xtarget);
    if (dAngle < EPSC) {
        //        printf("z  (%.3f,%.3f,%.3f)is parallel to n1 (%.3f,%.3f,%.3f)\n", xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ, n1.m_fX, n1.m_fY, n1.m_fZ);
        // special case: z1 || n1 (pole):
        //   no rotGlob required; xtarget is x.dir 
        rotGlob  = new Quat(1);
        xtarget.set(1, 0, 0);
    }  else if (dAngle > M_PI-EPSC) {
        //        printf("z  (%.3f,%.3f,%.3f) is antiparallel to n1 (%.3f,%.3f,%.3f)\n", xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ, n1.m_fX, n1.m_fY, n1.m_fZ);
        // special case: z1 || n1 (pole):
        //   180° rotGlob required; xtarget is x.dir 
        //        rotGlob  = new Quat(-1);
        rotGlob  = Quat::makeRotation(M_PI, 1, 0, 0);
        xtarget.set(1, 0, 0);
    } else {
        // in all other cases, rotGlob is required
        rotGlob = Quat::makeRotation(&vZ, &vPos);
        
        if (fabs(n1.dotProduct(&xtarget)) < EPSC){
            //            printf("special case: z1 (%.3f,%.3f,%.3f) orthogonal n1(%.3f,%.3f,%.3f)\n", xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ, n1.m_fX, n1.m_fY, n1.m_fZ);
            // special case: z1 orthogonal n1:
            //   rotGlob required; xtarget is z.dir 
            xtarget.set(0, 0, 1);
        } else {
            //            printf("z is not in the plane  and not parallel to n\n");
            // z is not in the plane  and not parallel to n: 
            // project it onto the plane to find target for x
            n1.normalize();
            n1.scale(n1.m_fZ);
            //            printf("n1     (%.3f,%.3f,%.3f)\n", n1.m_fX, n1.m_fY, n1.m_fZ);
            xtarget.subtract(&n1);
            //            printf("xtarget aft (%.3f,%.3f,%.3f)\n", xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ);

        }
    }

    // sanity check 
    if (fabs(xtarget.dotProduct(&vPos)) > EPSC) {
        printf("Target (%.3f,%.3f,%.3f) not orthogonal to vertex vector (%.3f,%.3f,%.3f):%f\n", 
               xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ, 
               vPos.m_fX, vPos.m_fY, vPos.m_fZ, 
               xtarget.dotProduct(&vPos));
    }

    // here we have a valid xtarget and a valid rotGlob
    Vec3D x0(1,0,0);
    Vec3D *pvx1 = rotGlob->apply(&x0);  
    /*
    printf("pvx1  (%.3f,%.3f,%.3f)\n", pvx1->m_fX, pvx1->m_fY, pvx1->m_fZ);
    printf("xtar  (%.3f,%.3f,%.3f)\n", xtarget.m_fX, xtarget.m_fY, xtarget.m_fZ);
    */
    double dAngleXT = xtarget.getAngle(pvx1);
    //    printf("Angle(globrot(x), target): %f\n", dAngleXT);
    if (dAngleXT > EPSC) {
        if (dAngleXT > M_PI-EPSC) {
            //            printf("flip\n");
            rotLoc  = Quat::makeRotation(M_PI, vPos.m_fX, vPos.m_fY, vPos.m_fZ);
        } else {
            rotLoc = Quat::makeRotation(pvx1, &xtarget);
        }

        // sanity checks
        Vec3D *pvx2 = rotLoc->apply(pvx1);
        if (pvx2->getAngle(&xtarget) > EPSC) {
            printf("locrot(globrot(x)) is not xtarget\n");
        }
        delete pvx2;

        Vec3D *pvx3 = rotLoc->apply(pvPos);
        if (pvx3->getAngle(pvPos) > EPSC) {
            printf("locrot(globrot(vertex)) is not vertex\n");
        }
        delete pvx3;
        
    } else {
        rotLoc = new Quat(1);
    }
    delete pvx1;


    rotLoc->mult(rotGlob, rotTot);
    
    delete rotGlob;
    delete rotLoc;


    return rotTot;
}


//----------------------------------------------------------------------------
// makeTangentTriangles
//
int PieCreator::makeTangentTriangles(double *pfPos, double *pfNorm, double *dVals, ShapeMode sMode, float fScale, bool bFramed) {
    
    Vec3D vPos(pfPos[0],pfPos[1],pfPos[2]);
    Vec3D vNorm(pfNorm[0],pfNorm[1],pfNorm[2]);
    return makeTangentTriangles(&vPos, &vNorm, dVals, sMode, fScale, bFramed);
}

//----------------------------------------------------------------------------
// makeTangentTriangles
//
int PieCreator::makeTangentTriangles(Vec3D *pvPos, Vec3D *pvNorm, double *dVals, ShapeMode sMode, float fScale, bool bFramed) {
    int iResult = 0;
    m_bBG = bFramed;
    m_dMax = 0;
    for (uint i = 0; i < m_iDim; ++i) {
        if (fabs(dVals[i]) > m_dMax) {
            m_dMax = fabs(dVals[i]);
        }
    }
    if (m_dMax > m_dSuperMax) {
        m_dSuperMax = m_dMax;
    }
       
    // make calculations for given values 
    m_dScale = fScale;
    switch (sMode) {
    case MODE_PIE:
        calcPieSectors(dVals);
        break;
    case MODE_STAR:
        calcStarPoints(dVals);
        break;
    case MODE_BARS:
        calcBarHeights(dVals);
        break;
    default:
        iResult = -1;
        printf("unknown mode: %d\n", sMode);
    }
    
    if (iResult == 0) {
        
        scaleTriangles();
 
        rotateTriangles(pvNorm);
        
        translateTriangles(pvPos);

        if (m_bBG) {
            scaleBGTriangles();
            
            rotateBGTriangles(pvNorm);
            
            translateBGTriangles(pvPos);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// scaleTriangles
//
void PieCreator::scaleTriangles() {
    // scale triangles
    trianglemap::iterator it;
    for (it = m_mvTriangleList.begin(); it != m_mvTriangleList.end(); ++it) {
        for (uint j = 0; j < it->second.size(); ++j) {
            triangle &t = it->second[j];
            for (int k = 0; k < 3; k++) {
                t.avVert[k].scale(m_dScale);
            }
        }
    }
}

//----------------------------------------------------------------------------
// scaleBGTriangles
//
void PieCreator::scaleBGTriangles() {
    // scale triangles
    trianglevec::iterator it;
    for (it = m_vBGTriangleList.begin(); it != m_vBGTriangleList.end(); ++it) {
        for (int k = 0; k < 3; k++) {
            it->avVert[k].scale(m_dScale);
        }
    }
}


//----------------------------------------------------------------------------
// rotateTriangles
//
void PieCreator::rotateTriangles(Vec3D *pvNorm) {
    // rotate all triangles
    Quat *rotTot = calcRotation(pvNorm);
    trianglemap::iterator it;

    for (it = m_mvTriangleList.begin(); it != m_mvTriangleList.end(); ++it) {
        
        for (uint i = 0; i < it->second.size(); ++i) {
            for (uint j = 0; j < 3; ++j) {
                Vec3D &pCur = it->second[i].avVert[j];
                Vec3D *p1  = rotTot->apply(&pCur);
                
                pCur.set(p1);
                //    printf("Triangle #%d vertex #%d: dot with vertex %d: %f\n", i, j, it->first, p1->dotProduct(pvPos)); 
                delete p1;
            }
        }
    }
    delete rotTot;
}


//----------------------------------------------------------------------------
// rotateBGTriangles
//
void PieCreator::rotateBGTriangles(Vec3D *pvNorm) {
    // rotate all triangles
    Quat *rotTot = calcRotation(pvNorm);
    trianglevec::iterator it;

    for (it = m_vBGTriangleList.begin(); it != m_vBGTriangleList.end(); ++it) {
        for (uint j = 0; j < 3; ++j) {
            Vec3D &pCur = it->avVert[j];
            Vec3D *p1  = rotTot->apply(&pCur);
                
            pCur.set(p1);
            //    printf("Triangle #%d vertex #%d: dot with vertex %d: %f\n", i, j, it->first, p1->dotProduct(pvPos)); 
            delete p1;
        }
    }
    delete rotTot;
}

//----------------------------------------------------------------------------
// translateTriangles
//
void PieCreator::translateTriangles(Vec3D *pvPos) {
        
    // shift triangles to positions on sphere
    trianglemap::iterator it;
    for (it = m_mvTriangleList.begin(); it != m_mvTriangleList.end(); ++it) {
        for (uint i = 0; i < it->second.size(); ++i) {
            // add pvPos
            for (uint j = 0; j < 3; ++j) {
                it->second[i].avVert[j].add(pvPos);
            }
        }
    }
}

//----------------------------------------------------------------------------
// translateBGTriangles
//
void PieCreator::translateBGTriangles(Vec3D *pvPos) {
        
    // shift triangles to positions on sphere
    trianglevec::iterator it;
    for (it = m_vBGTriangleList.begin(); it != m_vBGTriangleList.end(); ++it) {
        // add pvPos
        for (uint j = 0; j < 3; ++j) {
            it->avVert[j].add(pvPos);
        }
    }
}


//----------------------------------------------------------------------------
// displayTriangles
//
void PieCreator::displayTriangles() {
    trianglemap::iterator itS0;
    for (itS0 = m_mvTriangleList.begin(); itS0 != m_mvTriangleList.end(); ++itS0) {
        printf("Triangles for vlu #%d\n", itS0->first);
        
        for (uint k = 0; k < itS0->second.size(); k++) {
            const triangle &t =  itS0->second[k];
            printf("(%.2f %.2f %.2f) ", t.avVert[0].m_fX, t.avVert[0].m_fY, t.avVert[0].m_fZ);
            printf("(%.2f %.2f %.2f) ", t.avVert[1].m_fX, t.avVert[1].m_fY, t.avVert[1].m_fZ);
            printf("(%.2f %.2f %.2f) ", t.avVert[2].m_fX, t.avVert[2].m_fY, t.avVert[2].m_fZ);
            printf("\n");
        }
    }
}
