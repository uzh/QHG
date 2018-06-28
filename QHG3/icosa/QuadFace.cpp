#include <string.h>

#include "Vec3D.h"
#include "icoutil.h"
#include "SimpleFace.h"
#include "SimpleFace.cpp"
#include "QuadFace.h"
 
//-----------------------------------------------------------------------------
// createIcoFace 
//
QuadFace *QuadFace::createFace(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, Vec3D *pV4) {
    QuadFace *pQF = new QuadFace();
    int iResult = pQF->init(pV1, pV2, pV3, pV4);
    if (iResult != 0) {
        delete pQF;
        pQF = NULL;
    }
    return pQF;
}

//-----------------------------------------------------------------------------
// createIcoFace 
//
QuadFace *QuadFace::createFace(Vec3D **ppV) {
    return createFace(ppV[0], ppV[1], ppV[2], ppV[3]);
}


//-----------------------------------------------------------------------------
// constructor
//
QuadFace::QuadFace() {
}
 
//-----------------------------------------------------------------------------
// destructor
//
QuadFace::~QuadFace() {
}


//-----------------------------------------------------------------------------
// init
//
int QuadFace::init(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, Vec3D *pV4) {
    int iResult = 0;
    setVerts(pV1, pV2, pV3, pV4);
    
    for (int i = 0; i < 4; i++) {
        cart2Sphere(m_apVerts[i], &( m_adLons[i]), &(m_adLats[i]));
    }            

    Vec3D vEdge0(m_apVerts[0]);
    vEdge0.subtract(m_apVerts[2]);
    Vec3D vEdge1(m_apVerts[1]);
    vEdge1.subtract(m_apVerts[0]);

    m_pNormal = vEdge0.crossProduct(&vEdge1);
    m_dArea = m_pNormal->calcNorm();

    // face normal
    m_pNormal->normalize();
 
    return iResult;
}

//-----------------------------------------------------------------------------
// contains
//
PolyFace *QuadFace::contains(Vec3D *pP) {
    int iDir = 0;
    planify(pP);
    Vec3D *pPrev = m_apVerts[3];
    bool bContains = true;
    for (int i = 0; bContains &&(i < 4); i++) {
        Vec3D e(m_apVerts[i]);
        e.subtract(pPrev);
        Vec3D f(pP);
        f.subtract(pPrev);
        double dDir = e.m_fX*f.m_fY - e.m_fY*f.m_fX;
        int iDirTemp = (dDir>0)?1:((dDir < 0)?-1:0);
        if (iDir == 0) {
            iDir = iDirTemp;
        } else {
            if (iDir != iDirTemp) {
                bContains = false;
            }
        }
        
    }
    return bContains?this:NULL;
}

