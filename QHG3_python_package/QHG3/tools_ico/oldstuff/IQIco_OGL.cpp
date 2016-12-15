#include <GL/gl.h>
#include <GL/glut.h>

#include "utils.h"
#include "IQSurface_OGL.h"
#include "IQIco_OGL.h"
#include "IQOverlay.h"
#include "Icosahedron.h"





//----------------------------------------------------------------------------
// 
//
IQIco_OGL::IQIco_OGL(Icosahedron *pIco, IQOverlay *pOverlay) 
    : IQSurface_OGL(pOverlay),
      m_pIco(pIco)  {

}

/*
//----------------------------------------------------------------------------
// getCol
//
void IQIco_OGL::getCol(gridtype lNode, float fCol[4], float *pfScale) {
    double dVal = dNaN; 
    if (m_pVP != NULL) {
        dVal = m_pVP->getValue(lNode);
    }
    
    if (!isnan(dVal) && m_bUseAlt) {
        *pfScale = 1+m_fAltFactor*(dVal - m_fMinLevel)/(m_fMaxLevel-m_fMinLevel);
    } else {
        *pfScale = 1;
    }

    m_pCol->getCol(dVal, fCol);
}
*/


//----------------------------------------------------------------------------
// drawSurfaceLines
//   draw a line version of the surface
//
void IQIco_OGL::drawSurfaceLines() {
    int iColIndex = COL_BLUE;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    int iCount =0;
    
    PolyFace *pF = m_pIco->getFirstFace();
    while (pF != NULL) {
        glLoadName(99);
        glMaterialfv(GL_FRONT,  GL_EMISSION, MAT_COLORS[iColIndex]);
        glVertex3f(pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ);
        
        glMaterialfv(GL_FRONT,  GL_EMISSION, MAT_COLORS[iColIndex]);
        glVertex3f(pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ);
        
        glMaterialfv(GL_FRONT,  GL_EMISSION, MAT_COLORS[iColIndex]);
        glVertex3f(pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
        
        iCount++;
    
        pF = m_pIco->getNextFace();
    }
    
    glEnd();

}

//----------------------------------------------------------------------------
// 
//
void IQIco_OGL::drawNodePoints() {
    glBegin(GL_POINTS);
    float fCol[4];
   
    VertexLinkage *pVL = m_pIco->getLinkage();
    std::map<gridtype, Vec3D *> &mI2V = pVL->m_mI2V;
    std::map<gridtype, Vec3D *>::const_iterator it;
    for (it = mI2V.begin(); it != mI2V.end(); it++) {
        float fScale;
        getCol(it->first, fCol, &fScale);
        glColor4fv(fCol);
        glVertex3f(it->second->m_fX,it->second->m_fY,it->second->m_fZ);
    }
    glEnd();
}

//----------------------------------------------------------------------------
// drawNodeFacesAlt
//
void IQIco_OGL::drawNodeFacesAlt() {

    glBegin(GL_TRIANGLES);

    int iCount =0;

    //    VertexLinkage *pVL = m_pIco->getLinkage();

    PolyFace *pF = m_pIco->getFirstFace();
    while (pF != NULL) {
        // find worldmap value
        
        float fCol0[4];
        float fCol1[4];
        float fCol2[4];
        
        float fScale0;
        float fScale1;
        float fScale2;
        
        
        //        gridtype lID0 = pVL->getVertexID(pF->getVertex(0));
        gridtype lID0 = pF->getVertexID(0);
        getCol(lID0, fCol0, &fScale0);
        //        gridtype lID1 = pVL->getVertexID(pF->getVertex(1));
        gridtype lID1 = pF->getVertexID(1);
        getCol(lID1, fCol1, &fScale1);
        //        gridtype lID2 = pVL->getVertexID(pF->getVertex(2));
        gridtype lID2 = pF->getVertexID(2);
        getCol(lID2, fCol2, &fScale2);
        
        
        Vec3D vC(pF->getVertex(1));
        vC.scale(fScale1);
        Vec3D vCb(pF->getVertex(0));
        vCb.scale(fScale0);
        vC.subtract(&vCb);
        
        Vec3D vD(pF->getVertex(2));
        vD.scale(fScale2);
        Vec3D vDb(pF->getVertex(1));
        vDb.scale(fScale1);
        vD.subtract(&vDb);
        
        float fNX = vC.m_fY*vD.m_fZ - vC.m_fZ*vD.m_fY;
        float fNY = vC.m_fZ*vD.m_fX - vC.m_fX*vD.m_fZ;
        float fNZ = vC.m_fX*vD.m_fY - vC.m_fY*vD.m_fX;

        float fS = 1.0/sqrt(fNX*fNX+fNY*fNY+fNZ*fNZ);
        glNormal3f(fS*fNX, fS*fNY, fS*fNZ);

        glMaterialfv(GL_FRONT,  m_iMatType, fCol0);
        glVertex3f(fScale0*pF->getVertex(0)->m_fX,fScale0*pF->getVertex(0)->m_fY,fScale0*pF->getVertex(0)->m_fZ);
        
        glMaterialfv(GL_FRONT,  m_iMatType, fCol1);
        glVertex3f(fScale1*pF->getVertex(1)->m_fX,fScale1*pF->getVertex(1)->m_fY,fScale1*pF->getVertex(1)->m_fZ);
            
        glMaterialfv(GL_FRONT,  m_iMatType, fCol2);
        glVertex3f(fScale2*pF->getVertex(2)->m_fX,fScale2*pF->getVertex(2)->m_fY,fScale2*pF->getVertex(2)->m_fZ);
        
        iCount++;
        
        pF = m_pIco->getNextFace();
    }
    glEnd();
}

//----------------------------------------------------------------------------
// drawNodeFacesFlat
//
void IQIco_OGL::drawNodeFacesFlat() {
    glBegin(GL_TRIANGLES);
    int iCount =0;

    //    VertexLinkage *pVL = m_pIco->getLinkage();

    PolyFace *pF = m_pIco->getFirstFace();
    while (pF != NULL) {
        float fScale;
        float fCol[4];
            
        //        gridtype lID = pVL->getVertexID(pF->getVertex(0));
        gridtype lID = pF->getVertexID(0);
        getCol(lID, fCol, &fScale);
        glMaterialfv(GL_FRONT,  m_iMatType, fCol);
        glNormal3f(pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ);
        glVertex3f(fScale*pF->getVertex(0)->m_fX, fScale*pF->getVertex(0)->m_fY, fScale*pF->getVertex(0)->m_fZ);
            
        // lID = pVL->getVertexID(pF->getVertex(1));
        lID = pF->getVertexID(1);
        getCol(lID, fCol, &fScale);
        glMaterialfv(GL_FRONT,  m_iMatType, fCol);
        glNormal3f(pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ);
        glVertex3f(fScale*pF->getVertex(1)->m_fX, fScale*pF->getVertex(1)->m_fY, fScale*pF->getVertex(1)->m_fZ);
            
        //        lID = pVL->getVertexID(pF->getVertex(2));
        lID = pF->getVertexID(2);
        getCol(lID, fCol, &fScale);
        glMaterialfv(GL_FRONT,  m_iMatType, fCol);
        glNormal3f(pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
        glVertex3f(fScale*pF->getVertex(2)->m_fX, fScale*pF->getVertex(2)->m_fY, fScale*pF->getVertex(2)->m_fZ);
            
        
        iCount++;
            
        pF = m_pIco->getNextFace();
    }
    glEnd();
}

//----------------------------------------------------------------------------
// drawNodeFaces
//
void IQIco_OGL::drawNodeFaces() {
 
    glEnable(GL_LIGHTING);
    if (m_bUseAlt) {
        drawNodeFacesAlt();
    } else {
        drawNodeFacesFlat();
    }
    glDisable(GL_LIGHTING);
 
}

//----------------------------------------------------------------------------
// 
//
void IQIco_OGL::drawNodeHex() {
   
    int iCount =0;
    int iCountH =0;
    float fCol[4];
    glEnable(GL_LIGHTING);

    //    int iMatType = GL_AMBIENT_AND_DIFFUSE;
    // for unlighted:  iMatType = GL_EMISSION

    VertexLinkage *pVL = m_pIco->getLinkage();

    glCullFace(GL_BACK);

    std::map<gridtype, std::set<std::pair< double, gridtype> > >::const_iterator it;
    for (it = pVL->m_mI2H.begin(); it != pVL->m_mI2H.end(); ++it) {
        Vec3D *vNode = pVL->getVertex(it->first);
        float fScale;

        getCol(it->first, fCol, &fScale);
        glMaterialfv(GL_FRONT, m_iMatType, fCol);

        const std::set<std::pair< double, gridtype> > &s = it->second;
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(vNode->m_fX,vNode->m_fY,vNode->m_fZ);
        // vertex first
        glVertex3f(fScale*vNode->m_fX,fScale*vNode->m_fY,fScale*vNode->m_fZ);
        
        std::set<std::pair< double, gridtype> >::const_iterator  it2;
        double dAngPrev =  s.rbegin()->first-2*M_PI;
        for (it2 = s.begin(); it2 != s.end(); it2++) {
            // we only draw this triangle if the angle 
            // to the previous vertex is acute
            double dAngCur = it2->first;
            if (abs(dAngCur - dAngPrev) < M_PI) {
                Vec3D *vC = pVL->getCenter(it2->second);
                glNormal3f(vC->m_fX,vC->m_fY,vC->m_fZ);
                glVertex3f(fScale*vC->m_fX,fScale*vC->m_fY,fScale*vC->m_fZ);
            } else {
                // otherwise, we start a new fan
                glEnd();
                glBegin(GL_TRIANGLE_FAN);
                // vertex first
                glNormal3f(vNode->m_fX,vNode->m_fY,vNode->m_fZ);
                glVertex3f(fScale*vNode->m_fX,fScale*vNode->m_fY,fScale*vNode->m_fZ);
                Vec3D *vC = pVL->getCenter(it2->second);
                glNormal3f(vC->m_fX,vC->m_fY,vC->m_fZ);
                glVertex3f(fScale*vC->m_fX,fScale*vC->m_fY,fScale*vC->m_fZ);
            }
            dAngPrev = dAngCur;
        }
        if ((2*M_PI-dAngPrev) < M_PI) {
            Vec3D *vC = pVL->getCenter(s.begin()->second);
            glNormal3f(vC->m_fX,vC->m_fY,vC->m_fZ);
            glVertex3f(fScale*vC->m_fX,fScale*vC->m_fY,fScale*vC->m_fZ);
        }


        glEnd();
        iCount += s.size();
        iCountH++;
    
        // now do the prism sides: origin - v_n - v_(n-1)
        if (m_bUseAlt) {
            glBegin(GL_TRIANGLES);
            Vec3D *vD = pVL->getCenter(s.begin()->second);
            std::set<std::pair< double, gridtype> >::const_reverse_iterator  it3;
            for (it3 = s.rbegin(); it3 != s.rend(); it3++) {
                Vec3D *vC = pVL->getCenter(it3->second);


                float fNX=vC->m_fY*vD->m_fZ- vC->m_fZ*vD->m_fY;
                float fNY=vC->m_fZ*vD->m_fX- vC->m_fX*vD->m_fZ;
                float fNZ=vC->m_fX*vD->m_fY- vC->m_fY*vD->m_fX;
                float fS = 1.0/sqrt(fNX*fNX+fNY*fNY+fNZ*fNZ);
                glNormal3f(fS*fNX, fS*fNY, fS*fNZ);
                glVertex3f(0, 0, 0);
                glVertex3f(fScale*vC->m_fX,fScale*vC->m_fY,fScale*vC->m_fZ);
                glVertex3f(fScale*vD->m_fX,fScale*vD->m_fY,fScale*vD->m_fZ);
                vD = vC;
            }
            glEnd();    
        }
    }

    // backsides for partial icos
    if (m_pIco->getPreSel()) {
        glCullFace(GL_FRONT);
        fCol[0] = 0.0;
        fCol[1] = 0.2;
        fCol[2] = 0.3;
        fCol[3] = 0.2;
        glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
        for (it = pVL->m_mI2H.begin(); it != pVL->m_mI2H.end(); ++it) {
            Vec3D *vNode = pVL->getVertex(it->first);

            const std::set<std::pair< double, gridtype> > &s = it->second;

            glBegin(GL_TRIANGLE_FAN);
            // vertex first
            glVertex3f(vNode->m_fX,vNode->m_fY,vNode->m_fZ);
        
            std::set<std::pair< double, gridtype> >::const_iterator  it2;
            
            for (it2 = s.begin(); it2 != s.end(); it2++) {
                Vec3D *vC = pVL->getCenter(it2->second);
                glVertex3f(vC->m_fX,vC->m_fY,vC->m_fZ);
            }
            Vec3D *vC = pVL->getCenter(s.begin()->second);
            glVertex3f(vC->m_fX,vC->m_fY,vC->m_fZ);

            glEnd();
        }
        glCullFace(GL_BACK);
    }

    glDisable(GL_LIGHTING);

}
