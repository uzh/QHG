#include <gtkmm.h>

#include <GL/gl.h>
#include <GL/glut.h>

#include "IcoView.h"
#include "IcoModel.h"

#include "TrivialSplitter.h"
#include "IcoGridCreator.h"
#include "IcoGridNodes.h"

#include "GridProjection.h"


#include "trackball.h"

#include "VertexLinkage.h"
#include "Surface.h"
#include "IcoSurface_OGL.h"
#include "PolyFace.h"
#include "Vec3D.h"
#include "Quat.h"
#include "ValReader.h"
#include "QMapUtils.h"
#include "LookUp.h"
#include "LookUpFactory.h"
#include "IcoColorizer.h"
#include "VRColorizer.h"


#define ICO 1

const float IcoModel::MAT_SPECULAR[4]  = { 0.5, 0.5, 0.5, 1.0 };
const float IcoModel::MAT_SHININESS[1] = { 10.0 };
const float IcoModel::MAT_BLACK[4]     = { 0.0, 0.0, 0.0, 1.0 };


#define NODE_FACES  0
#define FLAT_FACES  1



//-----------------------------------------------------------------------------
// constructor 
//
IcoModel::IcoModel(const char *pFile)
    : m_bColor(true), 
      m_bInitialized(false),
      m_bHaveObject(false),
      m_pSurface(NULL),
      m_pSurfaceOGL(NULL),
      m_pVR(NULL),
      m_pLookUp(NULL),
      m_iWF(GL_FILL),
      m_iDispLevel(0xffffffff),
      m_bBox(false),
      m_pFSpecial(NULL),
      m_bHaveList(false),
      m_iCurSubDivLevel(0),
      m_bDual(false),
      m_pCol(NULL) ,
      m_pqobj(NULL) {



    //   m_pIcosahedron->relink();


    m_pVR = QMapUtils::createValReader(pFile, true);
    if (m_pVR != NULL) {
        m_pVR->extractData();

        // create a standard geo lookup
        double dLParams[3];
        dLParams[0] = -1000;//m_pVR->getMin();
        dLParams[1] = 7000;//m_pVR->getMax();
        dLParams[2] = 0;
        m_pLookUp = LookUpFactory::instance()->getLookUp(LOOKUP_GEO, dLParams, 3);
        
        m_pCol = new VRColorizer(m_pVR);
        m_pCol->setLookUp(m_pLookUp);

        LookUpFactory::instance()->free();
    } else {
        printf("Couldn't open ValReader [%s]\n", pFile);
    }
}

//-----------------------------------------------------------------------------
//  destructor
//
IcoModel::~IcoModel() {
    if (m_pVR != NULL) {
        delete m_pVR;
    }
    if (m_pqobj != NULL) {
        gluDeleteQuadric(m_pqobj);
    }

}

//-----------------------------------------------------------------------------
//  init_gl
//
void IcoModel::init_gl() {
    printf("init_gl\n");

    glEnable(GL_CULL_FACE);
    
    glPushMatrix();
    
    //    glMaterialfv(GL_FRONT, GL_SPECULAR, MAT_SPECULAR);
    //    glMaterialfv(GL_FRONT, GL_SHININESS, MAT_SHININESS);
 
    glNormal3f(0.0, 0.0, 1.0);


    glPopMatrix();

    glEnable(GL_NORMALIZE);

    m_iListBase = glGenLists(1);
    m_bHaveList = false;
    buildObject();
}


//-----------------------------------------------------------------------------
//  toggle_color
//
void IcoModel::toggle_color() {
    m_bColor = !m_bColor;
    m_bHaveList = false;
    buildObject();
    printf("clor setting: %d\n", m_bColor);
}

//-----------------------------------------------------------------------------
//  setDisplayLevel
//
void IcoModel::setDisplayLevel(int iDispLevel) {
    if (iDispLevel >= 0) {
        m_iDispLevel = (1 << iDispLevel);
    } else { 
        m_iDispLevel = 0xffffffff;
    }
    m_bHaveList = false;
    buildObject();
    //    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
//  
//
void IcoModel::toggleDisplayLevel(int iLevel) {
    iLevel = 1 << iLevel; 
    if ((m_iDispLevel & iLevel) == 0) {
        m_iDispLevel |= iLevel;
    } else {
        m_iDispLevel &= ~iLevel;
    }
    m_bHaveList = false;
    buildObject();
}

//-----------------------------------------------------------------------------
//  buildNodeFaces
//    draw faces colored by nodelist
//
void IcoModel::buildNodeFaces() {
    if (!m_bHaveList) {
        glDeleteLists(m_iListBase+NODE_FACES, 1);
        glNewList(m_iListBase+NODE_FACES, GL_COMPILE);
        printf("buildNodeFace\n");
        drawFaces();
        
        glEndList();

        m_bHaveList = true;
    }
}


//-----------------------------------------------------------------------------
//  buildObject
//    draw faces colored by nodelist
//
void IcoModel::buildObject() {
    printf("buildObject havelist %s\n", m_bHaveList?"yes":"no");
    buildNodeFaces();
    
}

//-----------------------------------------------------------------------------
//  drawFan
//
void IcoModel::drawFan(Vec3D &A, Vec3D &B) {
    Vec3D *vAxis =A.crossProduct(&B);
    vAxis->normalize();
    double dAlpha = A.getAngle(&B);


    glBegin(GL_QUAD_STRIP);

    //    glColor4f(1.0, 1.0, 1.0, 0.0);
    //    glVertex3f(0, 0, 0);
    double dR1 = 1.05;
    double dR2 = 0.75;

    for (int i = 0; i <= 20; i++) {
        double d = i*dAlpha/20;
        Quat *pQ = Quat::makeRotation(d, *vAxis);
        Vec3D *pC = pQ->apply(&A);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glVertex3f(dR1*pC->m_fX, dR1*pC->m_fY, dR1*pC->m_fZ);
        glColor4f(1.0, 1.0, 1.0, 0.0);
        glVertex3f(dR2*pC->m_fX, dR2*pC->m_fY, dR2*pC->m_fZ);
        
        
        delete pQ;
        delete pC;
        }
    delete vAxis;
    glEnd();
}

//-----------------------------------------------------------------------------
//  drawDisc
//
void IcoModel::drawDisc(double dLonMin, double dLonMax, double dLat) {
    
    glBegin(GL_QUAD_STRIP);
    double dR1 = 1.05;
    double dR2 = 0.75;
    for (int i = 0; i <= 20; i++) {
        double dV = dLonMin+i*(dLonMax -dLonMin)/20;
        glColor4f(1.0, 0.0, 0.0, 1.0);
        glVertex3f(dR1*cos(dLat)*cos(dV), dR1*cos(dLat)*sin(dV), sin(dLat));
        glColor4f(1.0, 0.0, 0.0, 0.0);
        glVertex3f(dR2*cos(dLat)*cos(dV), dR2*cos(dLat)*sin(dV), sin(dLat));
    }        
    
    glEnd();
}

//-----------------------------------------------------------------------------
//  outlineTriangle
//
void IcoModel::outlineTriangle(PolyFace *pF) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);


    glBegin(GL_TRIANGLES);
    
    glColor4f(1.0, 0.0, 0.5, 0.4);
    Vec3D vM;
    for (int i = 0; i < 3; i++) {
        Vec3D *pV = pF->getVertex(i);
        vM.add(pV);
        glVertex3f(pV->m_fX*1.01, pV->m_fY*1.01, pV->m_fZ*1.01);
    }
    vM.scale(0.4);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, m_iWF);
    glBegin(GL_LINES);
    glColor4f(1.0, 0.0, 0.0, 1.0);
   glVertex3f(0, 0, 0);
    glVertex3f(vM.m_fX, vM.m_fY, vM.m_fZ);
    glEnd();
}

void IcoModel::refreshImage() {
    m_bHaveList = false;
    buildObject();
}

//-----------------------------------------------------------------------------
//  drawFaces
//
void IcoModel::drawFaces() {
    printf("[IcoModel::drawFaces]surface: %p\n", m_pSurfaceOGL);
    if (m_pSurfaceOGL != NULL) {
        m_pSurfaceOGL->setColorizer(m_pCol);
        m_pSurfaceOGL->drawFaces();
    }
}

//-----------------------------------------------------------------------------
//  setSurface
//
void IcoModel::setSurface(Surface *pSurface, IcoSurface_OGL *pSurfaceOGL) {

    m_pSurface    = pSurface;
    m_pSurfaceOGL = pSurfaceOGL;
 
    m_bInitialized = false;
}


//-----------------------------------------------------------------------------
//  draw
//
void IcoModel::draw() {
    // Init GL context.
    if (!m_bInitialized) {
        init_gl();
        m_bInitialized = true;
    }

    // draw icosahedron
    glPushMatrix();
    
    glPolygonMode(GL_FRONT_AND_BACK, m_iWF);
    glCallList(m_iListBase+NODE_FACES);
    

    // draw decoraitions

    if (m_bBox) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        /*
        tbox *pB = m_pIcosahedron->getBox();
        
        Vec3D LL(cos(pB->dLatMin)*cos(pB->dLonMin), cos(pB->dLatMin)*sin(pB->dLonMin),sin(pB->dLatMin)); 
        Vec3D LR(cos(pB->dLatMin)*cos(pB->dLonMax), cos(pB->dLatMin)*sin(pB->dLonMax),sin(pB->dLatMin)); 
        Vec3D UL(cos(pB->dLatMax)*cos(pB->dLonMin), cos(pB->dLatMax)*sin(pB->dLonMin),sin(pB->dLatMax)); 
        Vec3D UR(cos(pB->dLatMax)*cos(pB->dLonMax), cos(pB->dLatMax)*sin(pB->dLonMax),sin(pB->dLatMax)); 
        drawFan(LL, UL);
        drawFan(LR, UR);
        drawDisc(pB->dLonMin, pB->dLonMax, pB->dLatMin);
        drawDisc(pB->dLonMin, pB->dLonMax, pB->dLatMax);
        */
    }
    
    if (m_pFSpecial != NULL) {
        outlineTriangle(m_pFSpecial);
    }
    
    if (m_pqobj == NULL) {
        m_pqobj=gluNewQuadric();
    }
    gluQuadricDrawStyle(m_pqobj, GLU_FILL);
    gluQuadricNormals(m_pqobj, GLU_SMOOTH);
    
    glPushMatrix();
    glTranslatef(0.0,.0, 1.0);
    gluSphere(m_pqobj, 0.01, 10,10);
    glPopMatrix();
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);


    glPopMatrix();
}


void IcoModel::locate(double dLon, double dLat, gridtype *piNode) {
    *piNode = m_pSurface->findNode(dLon, dLat);
    printf(" %f,%f -> %d\n", dLon ,dLat, *piNode);
    m_pFSpecial = m_pSurface->findFace(dLon, dLat);
    /* debug: show all vertices
    VertexLinkage *pVL= m_pIcosahedron->getLinkage();
    std::map<gridtype, Vec3D *>::const_iterator it;
    for (it = pVL->m_mI2V.begin; it != pVL->m_mI2V.end(); it++) { 
        double dLon;
        double dLat;
        cart2Sphere(it->second, &dLon, &dLat);
        printf("%d  %f  %f\n", it->frst, dLon, dLat);
    }
    */
}


