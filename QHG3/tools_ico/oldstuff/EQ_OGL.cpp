#include <GL/gl.h>
#include <GL/glut.h>

#include "IcoSurface_OGL.h"
#include "EQ_OGL.h"
#include "EQsahedron.h"

EQ_OGL::EQ_OGL(EQsahedron *pEQ) 
    : m_pEQ(pEQ) {

}

//----------------------------------------------------------------------------
// getCol
//   for vertex colors (
void EQ_OGL::getCol(Vec3D *pV,  float fCol[4]) {
    double theta;
    double phi;
    cart2Sphere(pV, &theta, &phi);    
    m_pCol->getCol(theta, phi, fCol);
}

void EQ_OGL::drawFaces() {
    printf("drawFaces by Ico_OGL\n");
    int iCount =0;
    printf("Doing some faces\n");
    glBegin(GL_TRIANGLES);
    //    printf("displevel: %d\n", m_iDispLevel);
    float fCol[4];
        
    PolyFace *pF = m_pEQ->getFirstFace();

    while (pF != NULL) {
        if  ((0xffffffff & (1<< pF->getLevel())) != 0) {
            // find worldmap value
                
            glLoadName(99);
            getCol(pF->getVertex(0),  fCol);
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ);
                
            getCol(pF->getVertex(1),  fCol);
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ);
                
            getCol(pF->getVertex(2),  fCol);
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
                
            iCount++;
        }
        pF = m_pEQ->getNextFace();
    }
    printf("-- did %d faces\n", iCount);
    glEnd();
    
    // do back colors if necessary
          if (m_pEQ->isPartial()) {
            glCullFace(GL_FRONT);
            float fCol[4];
            fCol[0] = 0.0;
            fCol[1] = 0.2;
            fCol[2] = 0.3;
            fCol[3] = 0.2;
            glBegin(GL_TRIANGLES);
            PolyFace *pF = m_pEQ->getFirstFace();
            
            while (pF != NULL) {
                glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
                glVertex3f(pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ);
                
                glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
                glVertex3f(pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ);
                
                glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
                glVertex3f(pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
                
                pF = m_pEQ->getNextFace();
            
            }

            glEnd();
        }
        glCullFace(GL_BACK);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);

}
