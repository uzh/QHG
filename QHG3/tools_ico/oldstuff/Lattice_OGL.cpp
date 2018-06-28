#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "IcoSurface_OGL.h"
#include "Lattice_OGL.h"
#include "Lattice.h"
#include "PolyFace.h"
#include "GridProjection.h"

Lattice_OGL::Lattice_OGL(Lattice *pLattice) 
    : m_pLattice(pLattice) {

}

void Lattice_OGL::getCol(Vec3D *pV,  float fCol[4]) {
    double theta;
    double phi;
    cart2Sphere(pV, &theta, &phi);    
    m_pCol->getCol(theta, phi, fCol);
}


void Lattice_OGL::drawFaces() {
    printf("[Lattice_OGL::drawFaces] numPolys:%d\n", m_pLattice->getNumPolys());
    int iCount =0;
    float fCol[4];
    
    // no lattice: draw green square
    if (m_pLattice->getNumPolys() == 0) {
        fCol[0] = 0.2;
        fCol[1] = 0.8;
        fCol[2] = 0.1;
        fCol[3] = 1.0;
        glBegin(GL_POLYGON);

        glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);
        glVertex3f(-1, -1, 0);
        glVertex3f( 1, -1, 0);
        glVertex3f( 1,  1, 0);
        glVertex3f(-1,  1, 0);
        glEnd();
    } else {
        double dOffsX = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsX;
        double dOffsY = m_pLattice->getGridProjection()->getProjGrid()->m_dOffsY;
        
        printf("Use GP %p Grid =  %s\n", m_pLattice->getGridProjection(), m_pLattice->getGridProjection()->getProjGrid()->toString());
        double dMinLat = 1e3;
        double dMinLatY = 0;
        double dMaxLat = -1e3;
        double dMaxLatY = 0;
        
        for (int i = 0; i < m_pLattice->getNumPolys(); i++) {
            glBegin(GL_POLYGON);
            //    printf("displevel: %d\n", m_iDispLevel);
        

            glLoadName(99);
            double dLon;
            double dLat;
            PolyFace *pPF =  m_pLattice->getFaces()[i];
            for (int j = 0; j < pPF->getNumVertices(); j++) {
                Vec3D *pV = pPF->getVertex(j);
                m_pLattice->getGridProjection()->gridToSphere(pV->m_fX, pV->m_fY, dLon, dLat);
                                         
                if (dLat < dMinLat) {
                    dMinLat = dLat;
                    dMinLatY = pV->m_fY;
                }
                if (dLat > dMaxLat) {
                    dMaxLat = dLat;
                    dMaxLatY = pV->m_fY;
                }

                m_pCol->getCol(dLon, dLat,  fCol);

                glMaterialfv(GL_FRONT,  GL_EMISSION, fCol);

                glVertex3f(pV->m_fX+dOffsX, pV->m_fY+dOffsY, pV->m_fZ);
            }
           
            iCount++;
        
            glEnd();
        }
        printf("MinLat: %f for Y=%f\n", dMinLat*180/M_PI, dMinLatY);
        printf("MaxLat: %f for Y=%f\n", dMaxLat*180/M_PI, dMaxLatY);
    }
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}
