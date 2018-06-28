#include <string.h>

#include <GL/gl.h>
#include <GL/glut.h>

#include "utils.h"
#include "IQSurface_OGL.h"
#include "IQFlat_OGL.h"
#include "GridProjection.h"
#include "PolyFace.h"
#include "IQOverlay.h"
#include "Lattice.h"





//----------------------------------------------------------------------------
// 
//
IQFlat_OGL::IQFlat_OGL(Lattice *pLattice,IQOverlay *pOverlay) 
    : IQSurface_OGL(pOverlay),
      m_pLattice(pLattice)  {

}

/*
//----------------------------------------------------------------------------
// getCol
//
void IQFlat_OGL::getCol(gridtype lNode, float fCol[4], float *pfScale) {
    double dVal = dNaN; 
    if (m_pVP != NULL) {
        dVal = m_pVP->getValue(lNode);
    
        
        if (!isnan(dVal) && m_bUseAlt) {
            *pfScale = 1+100*m_fAltFactor*(dVal - m_fMinLevel)/(m_fMaxLevel-m_fMinLevel);
        } else {
            *pfScale = 1;
        }

        m_pCol->getCol(dVal, fCol);
    } else {
        memcpy(fCol, MAT_COLORS[COL_BLUE], 4*sizeof(float));
    }

}
*/


//----------------------------------------------------------------------------
// drawSurfaceLines
//   draw a line version of the surface
//
void IQFlat_OGL::drawSurfaceLines() {
    int iColIndex = COL_BLUE;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int iCount =0;


    if (m_pLattice->getNumPolys() == 0) {
        glBegin(GL_POLYGON);
        
        glMaterialfv(GL_FRONT,  GL_EMISSION,  MAT_COLORS[iColIndex]);
        glVertex3f(-1, -1, 0);
        glVertex3f( 1, -1, 0);
        glVertex3f( 1,  1, 0);
        glVertex3f(-1,  1, 0);
        glEnd();
    } else {   
        //        VertexLinkage *pVL = m_pLattice->getLinkage();

        double dOffsX = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsX;
        double dOffsY = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsY;
        
        
        for (int i = 0; i < m_pLattice->getNumPolys(); i++) {
            glBegin(GL_POLYGON);

            glLoadName(99);
            float fCol0[4];
            float fScale0;
            PolyFace *pPF =  m_pLattice->getFaces()[i];
            for (int j = 0; j < pPF->getNumVertices(); j++) {
                
                Vec3D *pV = pPF->getVertex(j);
                //                m_pLattice->getGridProjection()->gridToSphere(pV->m_fX, pV->m_fY, dLon, dLat);
                //                m_pCol->getCol(dLon, dLat,  fCol);
                /*                                
                gridtype lID0 = pVL->getVertexID(pV);
                */
                gridtype lID0 = pPF->getVertexID(j);
                getCol(lID0,  fCol0, &fScale0);


                glMaterialfv(GL_FRONT,  GL_EMISSION, fCol0);

                glVertex3f(pV->m_fX+dOffsX, pV->m_fY+dOffsY, 0);
            }
           
            iCount++;
        
            glEnd();
        }

    }
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
   
}

//----------------------------------------------------------------------------
// drawNodePoints
//
void IQFlat_OGL::drawNodePoints() {
    glBegin(GL_POINTS);
    float fCol[4];
   
    VertexLinkage *pVL = m_pLattice->getLinkage();
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
// drawNodeFaces
//
void IQFlat_OGL::drawNodeFaces() {
 
    glEnable(GL_LIGHTING);
    glCullFace(GL_BACK);
    
    int iCount =0;

    //    VertexLinkage *pVL = m_pLattice->getLinkage();

    double dOffsX = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsX;
    double dOffsY = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsY;


    for (int i = 0; i < m_pLattice->getNumPolys(); i++) {
        glBegin(GL_POLYGON);

        glLoadName(99);       
        float fCol0[4];
        float fScale0 = 0.0;

            
        PolyFace *pPF =  m_pLattice->getFaces()[i];
        for (int j = 0; j < pPF->getNumVertices(); j++) {
            
            Vec3D *pV = pPF->getVertex(j);
            /*
            //                m_pLattice->getGridProjection()->gridToSphere(pV->m_fX, pV->m_fY, dLon, dLat);
            
            gridtype lID0 = pVL->getVertexID(pV);
            */
            gridtype lID0 = pPF->getVertexID(j);

            getCol(lID0,  fCol0, &fScale0);
            
            float fZ = 0;
            if (m_bUseAlt) {
                fZ = fScale0; 
            }

            glMaterialfv(GL_FRONT,  m_iMatType, fCol0);
            glNormal3f(0, 0, 1);
            glVertex3f(pV->m_fX+dOffsX, pV->m_fY+dOffsY, fZ);
        }
        
        iCount++;
        
        glEnd();
    }
   
    glCullFace(GL_FRONT);
    glMaterialfv(GL_FRONT, m_iMatType, MAT_COLORS[COL_DGRAY]);


    // backsides
    for (int i = 0; i < m_pLattice->getNumPolys(); i++) {
        glBegin(GL_POLYGON);

        glLoadName(99);       
        float fCol0[4];
        float fScale0 = 0.0;

            
        PolyFace *pPF =  m_pLattice->getFaces()[i];
        for (int j = 0; j < pPF->getNumVertices(); j++) {
            
            Vec3D *pV = pPF->getVertex(j);
            /*
            //                m_pLattice->getGridProjection()->gridToSphere(pV->m_fX, pV->m_fY, dLon, dLat);
            

            gridtype lID0 = pVL->getVertexID(pV);
            */
            gridtype lID0 = pPF->getVertexID(j);
            getCol(lID0,  fCol0, &fScale0);
            
            float fZ = 0;
            if (m_bUseAlt) {
                fZ = fScale0; 
            }

            glVertex3f(pV->m_fX+dOffsX, pV->m_fY+dOffsY, fZ);
        }
        
        iCount++;
        
        glEnd();

        
    }


    glCullFace(GL_BACK);
    glDisable(GL_LIGHTING);
 

}

//----------------------------------------------------------------------------
// drawNodeHexHoriPlane
//  draw top and bottom of prisms
//
void IQFlat_OGL::drawNodeHexHoriPlane(VertexLinkage *pVL, double dOffsX, double dOffsY, bool bTop) {
    float fCol[4];

    glCullFace(bTop?GL_BACK:GL_FRONT);

    std::map<gridtype, std::set<std::pair< double, gridtype> > >::const_iterator it;
    for (it = pVL->m_mI2H.begin(); it != pVL->m_mI2H.end(); ++it) {
        Vec3D *vNode = pVL->getVertex(it->first);
        float fScale0 = 0;
        float fScale  = 0;
        
        const std::set<std::pair< double, gridtype> > &s = it->second;
        getCol(it->first, fCol, &fScale0);
        
        fScale = (bTop)?fScale0:0;
        glMaterialfv(GL_FRONT, m_iMatType, fCol);
        if (s.size()>2) {
            
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, 1);
            // vertex first
            glVertex3f(vNode->m_fX+dOffsX,vNode->m_fY+dOffsY, fScale);
            
            std::set<std::pair< double, gridtype> >::const_iterator  it2; 
            double dAngPrev =  s.rbegin()->first-2*M_PI;
            for (it2 = s.begin(); it2 != s.end(); it2++) {
                
                // we only draw this triangle if the angle 
                // to the previous vertex is acute
                double dAngCur = it2->first;
                if (abs(dAngCur - dAngPrev) < M_PI) {
                    Vec3D *vC = pVL->getCenter(it2->second);
                    glNormal3f(0, 0, 1);
                    glVertex3f(vC->m_fX+dOffsX,vC->m_fY+dOffsY,fScale);
                } else {
                    // otherwise, we start a new fan
                    glEnd();
                    glBegin(GL_TRIANGLE_FAN);
                    // vertex first
                    glNormal3f(0, 0, 1);
                    glVertex3f(vNode->m_fX+dOffsX,vNode->m_fY+dOffsY,fScale);
                    Vec3D *vC = pVL->getCenter(it2->second);
                    glNormal3f(0, 0, 1);
                    glVertex3f(vC->m_fX+dOffsX,vC->m_fY+dOffsY,fScale);
                }
                dAngPrev = dAngCur;
            }
            if ((2*M_PI-dAngPrev) < M_PI) {
                    Vec3D *vC = pVL->getCenter(s.begin()->second);
                    glNormal3f(0, 0, 1);
                    glVertex3f(vC->m_fX+dOffsX,vC->m_fY+dOffsY,fScale);
            }
            
            
            glEnd();
            
        }
    }
}

//----------------------------------------------------------------------------
// drawNodeHexSides
//  draw sides of prisms
//
void IQFlat_OGL::drawNodeHexSides(VertexLinkage *pVL, double dOffsX, double dOffsY) {
    float fCol[4];

    glCullFace(GL_FRONT);
        
    std::map<gridtype, std::set<std::pair< double, gridtype> > >::const_iterator it;
    for (it = pVL->m_mI2H.begin(); it != pVL->m_mI2H.end(); ++it) {
        float fScale0 = 0;
        const std::set<std::pair< double, gridtype> > &s = it->second;
        
        getCol(it->first, fCol, &fScale0);
        glMaterialfv(GL_FRONT, m_iMatType, fCol);
        
        // now do the prism sides: new.bottom old.bottom old.top new.top
        if (m_bUseAlt) {
            if (s.size()>2) {
                glCullFace(GL_BACK);
                Vec3D *vD = pVL->getCenter(s.begin()->second);
                std::set<std::pair< double, gridtype> >::const_reverse_iterator  it3;
                for (it3 = s.rbegin(); it3 != s.rend(); it3++) {
                    Vec3D *vC = pVL->getCenter(it3->second);
                    
                    float fNX=vD->m_fY - vC->m_fY;
                    float fNY=vD->m_fX - vC->m_fX;
                    
                    float fS = 1.0/sqrt(fNX*fNX+fNY*fNY);
                    glBegin(GL_POLYGON);
                    glNormal3f(fS*fNX, fS*fNY, 0);
                    glVertex3f(vC->m_fX+dOffsX,vC->m_fY+dOffsY,0);
                    glVertex3f(vD->m_fX+dOffsX,vD->m_fY+dOffsY,0);
                    glVertex3f(vD->m_fX+dOffsX,vD->m_fY+dOffsY,fScale0);
                    glVertex3f(vC->m_fX+dOffsX,vC->m_fY+dOffsY,fScale0);
                    glEnd();    
                    vD = vC;
                    
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
// drawNodeHex
//
void IQFlat_OGL::drawNodeHex() {
   
    glEnable(GL_LIGHTING);

    double dOffsX = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsX;
    double dOffsY = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsY;
  
    VertexLinkage *pVL = m_pLattice->getLinkage();

    drawNodeHexHoriPlane(pVL, dOffsX, dOffsY, true);            
    drawNodeHexHoriPlane(pVL, dOffsX, dOffsY, false);   
    drawNodeHexSides(pVL, dOffsX, dOffsY);   

    glDisable(GL_LIGHTING);

}
