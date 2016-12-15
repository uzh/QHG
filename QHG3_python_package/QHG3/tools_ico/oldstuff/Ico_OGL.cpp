#include <GL/gl.h>
#include <GL/glut.h>

#include "types.h"
#include "IcoSurface_OGL.h"
#include "Ico_OGL.h"
#include "Icosahedron.h"

Ico_OGL::Ico_OGL(Icosahedron *pIco) 
    : m_pIco(pIco) {

}

//----------------------------------------------------------------------------
// getCol
//   for vertex colors (
void Ico_OGL::getCol(Vec3D *pV,  float fCol[4]) {
    double theta;
    double phi;
    cart2Sphere(pV, &theta, &phi);    
    m_pCol->getCol(theta, phi, fCol);
}

void Ico_OGL::drawFaces() {
    printf("drawFaces by Ico_OGL\n");
    int iCount =0;
    printf("Doing some faces\n");
    glBegin(GL_TRIANGLES);
    //    printf("displevel: %d\n", m_iDispLevel);
    float fCol[4];
        
    PolyFace *pF = m_pIco->getFirstFace();

    while (pF != NULL) {
        if  ((/*m_pIco->m_iDispLevel*/0xffffffff & (1<< pF->getLevel())) != 0) {
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
        pF = m_pIco->getNextFace();
    }
    printf("-- did %d faces\n", iCount);
    glEnd();
        
    if (m_pIco->getPreSel()) {
        glCullFace(GL_FRONT);
        float fCol[4];
        fCol[0] = 0.0;
        fCol[1] = 0.2;
        fCol[2] = 0.3;
        fCol[3] = 0.2;
        glBegin(GL_TRIANGLES);
        PolyFace *pF = m_pIco->getFirstFace();
            
        while (pF != NULL) {
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(0)->m_fX,pF->getVertex(0)->m_fY,pF->getVertex(0)->m_fZ);
                
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(1)->m_fX,pF->getVertex(1)->m_fY,pF->getVertex(1)->m_fZ);
                
            glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
            glVertex3f(pF->getVertex(2)->m_fX,pF->getVertex(2)->m_fY,pF->getVertex(2)->m_fZ);
                
            pF = m_pIco->getNextFace();
            
        }

        glEnd();
    }
    glCullFace(GL_BACK);
    

    
        
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);

}
