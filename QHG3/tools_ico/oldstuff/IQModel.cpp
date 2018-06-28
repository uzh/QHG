#include <gtkmm.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glut.h>


#include "icoutil.h"
#include "IQView.h"
#include "IQModel.h"


#include "trackball.h"
#include "PolyFace.h"
#include "Vec3D.h"
#include "Quat.h"
#include "ValReader.h"
#include "QMapUtils.h"
#include "QMapHeader.h"
#include "LookUp.h"
#include "LookUpFactory.h"
#include "LineReader.h"

#include "HeaderBase.h"
#include "SnapHeader.h"
#include "PopHeader.h"

#include "ValueProvider.h"
#include "SurfaceManager.h"
#include "Surface.h"
#include "IQSurface_OGL.h"
#include "ProjInfo.h"
#include "IQColorizer.h"
#include "SnapColorizer.h"

#include "notification_codes.h"
#include "init.h"

#define ICO_SKEL    0
#define NODE_FACES  1
#define NODE_POINTS 2
#define NODE_HEX    3

/*@@
const float IQModel::MAT_SPECULAR[4]  = { 0.5, 0.5, 0.5, 1.0 };
const float IQModel::MAT_SHININESS[1] = { 10.0 };
const float IQModel::MAT_BLACK[4]     = { 0.0, 0.0, 0.0, 1.0 };
*/

#define COL_RED      0
#define COL_GREEN    1
#define COL_BLUE     2
#define COL_YELLOW   3
#define COL_CYAN     4
#define COL_MAGENTA  5
#define COL_WHIE     6
#define COL_ORANGE   7
#define COL_PURPLE   8
#define COL_LEMON    9
#define COL_LILA    10
#define COL_GRAY    11
#define COL_DGRAY   12
#define COL_WINE    13
#define COL_LGREEN  14
#define COL_BUTTER  15
#define COL_BROWN   16
#define COL_DGREEN  17
#define COL_SKIN    18
#define COL_SKY     19


const float IQModel::MAT_COLORS[NUM_COLORS][4] = {
    { 1.0, 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0 },
    { 0.0, 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 0.0, 1.0, 1.0, 1.0 },
    { 1.0, 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0, 1.0 },
    { 1.0, 0.5, 0.0, 1.0 },
    { 0.5, 0.0, 1.0, 1.0 },
    { 0.7, 1.0, 0.0, 1.0 },
    { 0.7, 0.0, 1.0, 1.0 },
    { 0.7, 0.7, 0.7, 1.0 },
    { 0.5, 0.5, 0.5, 1.0 },
    { 0.65, 0.15, 0.5, 1.0 },
    { 0.7, 1.0, 0.6, 1.0 },
    { 1.0, 0.9, 0.2, 1.0 },
    { 0.5, 0.4, 0.1, 1.0 },
    { 0.2, 0.3, 0.1, 1.0 },
    { 0.9, 0.6, 0.6, 1.0 },
    { 0.5, 0.8, 1.0, 1.0 },
};


//-----------------------------------------------------------------------------
// constructor 
//
IQModel::IQModel(SurfaceManager *pSurfaceManager, ProjInfo *pPI)
    : m_pSurfaceManager(pSurfaceManager),
      m_Mode(0), 
      m_bColor(false), 
      m_bInitialized(false),
      m_bHaveObject(false),
      /*
      m_pIcosahedron(Icosahedron::create(1, POLY_TYPE_ICO)),
      m_pLattice(NULL),
      */
      m_pSurface(NULL),
      m_pSurfaceOGL(NULL), 
      m_pVR(NULL),
      m_pInitLookUp(NULL),
      m_iWF(GL_FILL),
      m_bBox(false),
      m_pFSpecial(NULL),
      m_pvHitNode(NULL),
      m_pValueProvider(NULL),
      m_iCurDataType(DATA_FILE_TYPE_NONE),
      m_pCol(NULL),
      m_iDisp(MODE_PLANES),
      m_pPI(pPI),
      m_bDrawing(false),
      m_bHaveList(false),
      m_bHex(INIT_HEX),
      m_bAxis(INIT_AXIS),
      m_bLoading(false),
      m_iFirst(1) {    // glList wont work when created before GLContext exists; therefore we do the first draw directly

    
    m_pPI->addObserver(this);

   
    // create a standard geo lookup
    double dLParams[3];
    dLParams[0] = -1000;//m_pVR->getMin();
    dLParams[1] = 7000;//m_pVR->getMax();
    dLParams[2] = 0;
    m_pInitLookUp = LookUpFactory::instance()->getLookUp(LOOKUP_GEO, dLParams, 3);
    
    m_pCol = new SnapColorizer();
    m_pCol->setLookUp(m_pInitLookUp);

}

//-----------------------------------------------------------------------------
//  destructor
//
IQModel::~IQModel() {
    if (m_pInitLookUp != NULL) {
        delete m_pInitLookUp;
    }

    if (m_pCol != NULL) {
        delete m_pCol;
    }
}

//-----------------------------------------------------------------------------
//  init_gl
//
void IQModel::init_gl() {
    printf("init_gl\n");

    glEnable(GL_CULL_FACE);
    
    glPushMatrix();
    
 
    glNormal3f(0.0, 0.0, 1.0);

    /* draw object */

    glPopMatrix();

    glEnable(GL_NORMALIZE);

    m_iListBase = glGenLists(5);

}

//-----------------------------------------------------------------------------
//  toggle_color
//
void IQModel::toggle_color() {
    m_bColor = !m_bColor;
    printf("clor setting: %d\n", m_bColor);
}


//-----------------------------------------------------------------------------
//  outlineTriangle
//
void IQModel::outlineTriangle(PolyFace *pF) {
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

//-----------------------------------------------------------------------------
//  drawHitLine
// 
void IQModel::drawHitLine() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    glLineWidth(2);
    glBegin(GL_LINES);
    glColor4f(1.0, 0.5, 0.3, 1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(1.1*m_pvHitNode->m_fX, 1.1*m_pvHitNode->m_fY, 1.1*m_pvHitNode->m_fZ);
    //    glEnd();

    std::set<gridtype>::const_iterator it;
    //    glLineWidth(0.5);
    //    glBegin(GL_LINES);
    glColor4f(1.0, 1.0, 0.8, 0.7);
    for (it = m_sNeighbors.begin(); it != m_sNeighbors.end(); it++) {
        //        Vec3D *pv = pVL->getVertex(*it);
        Vec3D *pv = m_pSurface->getVertex(*it);
        glVertex3f(0, 0, 0);
        glVertex3f(1.05*pv->m_fX, 1.05*pv->m_fY, 1.05*pv->m_fZ);
    }
    glEnd();
    glLineWidth(1);


}

//-----------------------------------------------------------------------------
//  drawAxis
//    draw faces colored by QMap
// 
void IQModel::drawAxis() {
    //    glPolygonMode(GL_FRONT_AND_BACK, m_iWF);
    glBegin(GL_LINES);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    glVertex3f(0, 0, -1.5);
    glVertex3f(0, 0, 1.5);
    glEnd();
}




//-----------------------------------------------------------------------------
//  buildNodeFaces
//    draw faces colored by nodelist
//
void IQModel::buildNodeFaces() {
    if (!m_bHaveList) {
        glDeleteLists(m_iListBase+NODE_FACES, 1);
        glNewList(m_iListBase+NODE_FACES, GL_COMPILE);

        if (m_pSurfaceOGL != NULL) {
            m_pSurfaceOGL->setColorizer(m_pCol);
            m_pSurfaceOGL->drawNodeFaces();
        }
        glEndList();

        m_bHaveList = true;
    }
}

//-----------------------------------------------------------------------------
//  buildNodeHex
//    draw faces colored by nodelist
//IQFlat_OGL.cpp
void IQModel::buildNodeHex() {
    if (!m_bHaveList) {
        glDeleteLists(m_iListBase+NODE_HEX, 1);
        glNewList(m_iListBase+NODE_HEX, GL_COMPILE);

        if (m_pSurfaceOGL != NULL) {
            m_pSurfaceOGL->setColorizer(m_pCol);
            m_pSurfaceOGL->drawNodeHex();
        }

        glEndList();

        m_bHaveList = true;
    }
}

//-----------------------------------------------------------------------------
//  select
//
void IQModel::select(int x, int y) {
    printf("Checking point %d,%d\n", x, y);


    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    GLdouble pr[16];
    glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLdouble mv[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mv);

    float fX = (float) x;
    float fY = (float)view[3] - (float) y;
    float fZ;
    glReadPixels(fX, fY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fZ);
    Vec3D vEnd(fX,fY,fZ);
    //    printf(" end vector (%f,%f,%f)\n", vEnd.m_fX,vEnd.m_fY,vEnd.m_fZ);


    Vec3D vEnd2;
    Vec3D vEye(mv[12], mv[13], mv[14]);

    gluUnProject(vEnd.m_fX, vEnd.m_fY, vEnd.m_fZ,  
                 mv, pr, view,
                 &vEnd2.m_fX, &vEnd2.m_fY, &vEnd2.m_fZ);
    //    printf(" end vector trans (%f,%f,%f)\n", vEnd2.m_fX,vEnd2.m_fY,vEnd2.m_fZ);


    if  (vEnd2.calcNorm()  < 2.01) {
        double dLon;
        double dLat;
        gridtype lNode;
        vEnd2.normalize();
        cart2Sphere(&vEnd2, &dLon, &dLat);
        locate(dLon, dLat, &lNode);
        

        Vec3D *p = m_pSurface->getVertex(lNode);
        cart2Sphere(p, &dLon, &dLat);
        printf(" %d -> %f,%f\n", lNode, dLon ,dLat);
        m_pvHitNode = p;
        double dVal = dNaN;

        ValReader *pVR = m_pPI->getValReader();
        if (pVR != NULL) {
            dVal = m_pVR->getDValue(dLon*180/M_PI, dLat*180/M_PI);
        } else {
            dVal = m_pValueProvider->getValue(lNode);
        }


        SelectData sd;
        sd.x = x;
        sd.y = y;
        sd.lNodeID = lNode;
        sd.dLon = dLon;
        sd.dLat = dLat;
        sd.dVal = dVal;
        notifyObservers(NOTIFY_SET_SELECTED, &sd);


        char sHit[512];
        char sVal[64];
        if ((fabs(dVal) > 1e3) ||  (fabs(dVal) < 1e-3))  {
            sprintf(sVal, "%e", dVal);
        } else {
            sprintf(sVal, "%f", dVal);
        }
        sprintf(sHit, "[%d,%d] node %d |  coords (%5.2f, %5.2f) | val %s", x, y,lNode, dLon*180/M_PI, dLat*180/M_PI, sVal);
        notifyObservers(NOTIFY_MESSAGE, sHit);
        
        std::set<gridtype> sNeighbors;
        for (int iDist = 0; iDist < 4; iDist++) {
            sNeighbors.clear();
            /*int iNumNeighs=*/ m_pSurface->collectNeighborIDs(lNode, iDist, sNeighbors);
            /*
            printf("NumNeighbors (dist=%d): %d:", iDist, iNumNeighs);
            for (std::set<gridtype>::const_iterator it = sNeighbors.begin(); it != sNeighbors.end(); it++) {
                printf("%lld ", *it);
            }
            printf("\n");
            */
        }


        m_sNeighbors.clear();
        m_sNeighbors.insert(sNeighbors.begin(), sNeighbors.end());
    } else {
        clearMarkers();
        char sMiss[128];
        printf("Miss\n");
        sprintf(sMiss, "[%d,%d] does not correspond to a node", x, y);
        notifyObservers(NOTIFY_ERROR, sMiss);
    }

}




//-----------------------------------------------------------------------------
//  draw
//
void IQModel::draw() {
   
    // Init GL context.
    if (!m_bInitialized) {
        init_gl();
        m_bInitialized = true;
    }

    // Draw logo model.
    glPushMatrix();
    
    glEnable(GL_CULL_FACE);
    if ((m_pSurfaceOGL != NULL) && (m_iCurDataType == DATA_FILE_TYPE_NONE)) {
        m_pSurfaceOGL->drawSurfaceLines();
    }

    // do nothing if no surface is here
    if (m_pSurfaceOGL != NULL) {
        // draw the main object
        if (m_iDisp == MODE_POINTS) {
            if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
                if (m_pSurfaceOGL != NULL) {
                    m_pSurfaceOGL->setColorizer(m_pCol);
                    m_pSurfaceOGL->drawNodePoints();
                }
            } else {
                // currently no other file types
            }
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, m_iWF);
       
            if (m_iCurDataType == DATA_FILE_TYPE_LIST) {

                if (m_bHex) {
                    if (m_iFirst>0) {
                        // found no other way to paint object at start (lists won't show)
                        m_pSurfaceOGL->setColorizer(m_pCol);
                        m_pSurfaceOGL->drawNodeHex();
                        m_iFirst--;
                        m_bHaveList = false;
                        buildNodeHex();
                    } else {
                        glCallList(m_iListBase+NODE_HEX);
                    }
                } else {
                    if (m_iFirst>0) {
                        // found no other way to paint object at start (lists won't show)
                        m_pSurfaceOGL->setColorizer(m_pCol);
                        m_pSurfaceOGL->drawNodeFaces();
                        m_iFirst--;
                        m_bHaveList = false;
                        buildNodeFaces();
                    } else {
                        glCallList(m_iListBase+NODE_FACES);
                    }
                }
            } else {
                // currently no other file types
            }
        
        }
    } else {
        printf("need a surface!!!!\n");
    }
    // decorations
    if (m_pvHitNode != NULL) {
        drawHitLine();
    }
     
    if (m_bAxis) {
        drawAxis();
    }
   
   
    glPopMatrix();
    //    m_bDrawing = false;

    notifyObservers(0, (void *)NOTIFY_MODEL_LOADED);
}


//-----------------------------------------------------------------------------
// locate
//
void IQModel::locate(double dLon, double dLat, gridtype *piNode) {
    *piNode = m_pSurface->findNode(dLon, dLat);
    printf(" %f,%f -> %d\n", dLon ,dLat, *piNode);
    m_pFSpecial = m_pSurface->findFace(dLon, dLat);
}
/*
void areacheck(Icosahedron *pIco) {
    std::vector<double> vA;
    double dSum = 0;
    PolyFace *pF = pIco->getFirstFace();
    while (pF != NULL) {
        dSum += pF->getArea();
        vA.push_back(pF->getArea());
        pF = pIco->getNextFace();
    }
    std::vector<double>::iterator it;

    printf("Number of faces: %zd (sum %f)\n", vA.size(), dSum);
    double dAvg =  dSum/vA.size();
    printf("Avg: %e\n", dAvg);
    double dS2 = 0;
    for (unsigned int i = 0; i < vA.size(); i++) {
        dS2 += vA[i]*vA[i];
    }
    printf("dev: %e\n", sqrt(dS2-dAvg*dAvg)/(vA.size()));

    it = std::min_element(vA.begin(), vA.end());
    printf("Min: %e\n", *it);
    it = std::max_element(vA.begin(), vA.end());
    printf("Max: %e\n", *it);
}
*/
//-----------------------------------------------------------------------------
//  setSurface
//
void IQModel::setSurface() {
    m_pSurface    = m_pSurfaceManager->getSurface();
    m_pSurfaceOGL = m_pSurfaceManager->getSurfaceOGL();
    m_pSurfaceOGL->setColorizer(m_pCol);
    m_bInitialized = false;

    m_pPI->setSurface(m_pSurface);
    
    m_pPI->setFlat(m_pSurfaceManager->getType() == STYPE_FLAT);
    clearMarkers();

    m_iCurDataType = (m_pValueProvider!=NULL)?m_pValueProvider->getDataType():DATA_FILE_TYPE_NONE;
    if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        m_bHaveList = false;
        if (m_bHex) {
            buildNodeHex();
        } else {
            buildNodeFaces();
        }
        
    } else {
        char sMess[128];
        sprintf(sMess, "Can't visualize datatype %d", m_iCurDataType);
        notifyObservers(NOTIFY_ERROR, sMess);
    }

    notifyObservers(0, (void*)NOTIFY_MODEL_REPAINT);
    
}

//-----------------------------------------------------------------------------
//  setValueProvider
//
void IQModel::setValueProvider(bool bForceCol) {
 
    m_pValueProvider = m_pSurfaceManager->getValueProvider();

    if (m_pSurfaceOGL != NULL) {
        m_pSurfaceOGL->setValueProvider(m_pValueProvider);
    }
    m_iCurDataType = m_pValueProvider->getDataType();
    m_pPI->setData(m_pValueProvider, bForceCol);
     
    //    notifyObservers(0, (void *)NOTIFY_MODEL_REPAINT);
    notifyObservers(0, (void *)NOTIFY_DATA_LOADED);
    m_pVR = m_pPI->getValReader();
    m_bColor = true;
    printf("[IQModel::setValueProvider] data type %d\n", m_iCurDataType);

    if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        if (m_iFirst == 0) {
            m_bHaveList = false;
            if (m_bHex) {
                buildNodeHex();
            } else {
                buildNodeFaces();
            }
        }
    } else {
        char sMess[128];
        sprintf(sMess, "Can't visualize datatype %d", m_iCurDataType);
        notifyObservers(2, sMess);
    }
}

//-----------------------------------------------------------------------------
//  setSurfaceValue
//
void IQModel::setSurfaceAndValues(bool bForceCol) {
    // set surface
    m_pSurface    = m_pSurfaceManager->getSurface();
    m_pSurfaceOGL = m_pSurfaceManager->getSurfaceOGL();
    m_pSurfaceOGL->setColorizer(m_pCol);
    m_bInitialized = false;

    m_pPI->setSurface(m_pSurface);
    
    m_pPI->setFlat(m_pSurfaceManager->getType() == STYPE_FLAT);

    // set value
    m_pValueProvider = m_pSurfaceManager->getValueProvider();

    if (m_pSurfaceOGL != NULL) {
        m_pSurfaceOGL->setValueProvider(m_pValueProvider);
    }
    m_iCurDataType = m_pValueProvider->getDataType();
    m_pPI->setData(m_pValueProvider, bForceCol);
     
    m_pVR = m_pPI->getValReader();
    m_bColor = true;

    clearMarkers();

    if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        if (m_iFirst == 0) {
            m_bHaveList = false;
            if (m_bHex) {
                buildNodeHex();
            } else {
                buildNodeFaces();
            }
        }
    } else {
        char sMess[128];
        sprintf(sMess, "Can't visualize datatype %d", m_iCurDataType);
        notifyObservers(NOTIFY_ERROR, sMess);
    }

    notifyObservers(0, (void*)NOTIFY_MODEL_REPAINT);
    
}


//-----------------------------------------------------------------------------
//  setPaintMode
//   set paint mode (WIRE or FILL)
//
void IQModel::setPaintMode(int iDisp) {
    m_iDisp = iDisp;

    if (m_iDisp == MODE_PLANES) {
        m_iWF = GL_FILL;
    } else {
        m_iWF = GL_LINE;
    }
    printf("Rebuilding %s\n", m_bHex?"Hex":"Face");
    if  (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        if (m_bHex) {
            buildNodeHex();
        } else {
            buildNodeFaces();
        }
    } 
    
    notifyObservers(NOTIFY_INVALIDATE, &m_iDisp);
};

//-----------------------------------------------------------------------------
//  toggleHex
//
void IQModel::toggleHex() {
    m_bHex = !m_bHex;
    //    printf("set Hex to %s\n", m_bHex?"true":"false");
    if  (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        m_bHaveList=false;
        if (m_bHex) {
            buildNodeHex();
        } else {
            buildNodeFaces();
        }
        
    } else {
        printf("No list building\n");
    }
    m_bHaveList = false;
}
//-----------------------------------------------------------------------------
//  toggleAlt
//
void IQModel::toggleAlt() {
    m_pSurfaceOGL->toggleAlt();
}


//-----------------------------------------------------------------------------
//  setUseAlt
//
void IQModel::setUseAlt(bool bUseAlt) {

    m_pSurfaceOGL->setUseAlt(bUseAlt);

    if  (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        m_bHaveList=false;
        if (m_bHex) {
            buildNodeHex();
        } else {
            buildNodeFaces();
        }
        notifyObservers(NOTIFY_USE_ALT_SET, &bUseAlt);
    } else {
        printf("No list building\n");
    }
    m_bHaveList = false;

}

//-----------------------------------------------------------------------------
//  toggle_lighting
//
void IQModel::toggleLighting() {
    m_pSurfaceOGL->toggleLighting();
}

//-----------------------------------------------------------------------------
//  toggle_lighting
//
void IQModel::setUseLight(bool bUseLight) {
    
       m_pSurfaceOGL->setUseLight(bUseLight);

    if  (m_iCurDataType == DATA_FILE_TYPE_LIST) {
        m_bHaveList=false;
        if (m_bHex) {
            buildNodeHex();
        } else {
            buildNodeFaces();
        }
        notifyObservers(NOTIFY_USE_LIGHT_SET, &bUseLight);
    } else {
        printf("No list building\n");
    }
    m_bHaveList = false;
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQModel::notify(Observable *pObs, int iType, const void *pCom) {
    /*
    if (pObs == m_pPI) {
        long iCom= (long)(pCom);
        switch (iCom) {
        case NOTIFY_LU_CHANGE: {
            printf("Must redraw\n");
            LookUp *pCurLookUp = m_pPI->getLU();  
            m_pCol->setLookUp(pCurLookUp);      

            if (!m_bLoading) {
 
                m_bColor = true;
                if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
                    m_bHaveList=false;
                    if (m_bHex) {
                        buildNodeHex();
                    } else {
                        buildNodeFaces();
                    }
                } 
                notifyObservers(0, (void*) NOTIFY_MODEL_REPAINT);
            }
        }
                break;
            
        default:
            break;
        }
    
    } else {
*/
        switch (iType) {
        case NOTIFY_LU_CHANGE: {
            printf("Must redraw\n");
            LookUp *pCurLookUp = m_pPI->getLU();  
            m_pCol->setLookUp(pCurLookUp);      
            
            if (!m_bLoading) {
                
                m_bColor = true;
                if (m_iCurDataType == DATA_FILE_TYPE_LIST) {
                    m_bHaveList=false;
                    if (m_bHex) {
                        buildNodeHex();
                    } else {
                        buildNodeFaces();
                    }
                } 
                notifyObservers(0, (void*) NOTIFY_MODEL_REPAINT);
            }
        }
                break;
        case NOTIFY_NEW_GRID: {
            setSurface();
        }
            break;
        
        case NOTIFY_NEW_DATA: {
            bool bForceCol = *((bool *)pCom);
            setValueProvider(bForceCol);

        }
            break;

        case NOTIFY_NEW_GRIDDATA: {
            // have a new grid and data
            bool bForceCol = true;
            setSurfaceAndValues(bForceCol);

        }
            break;
        
        case NOTIFY_CLEAR_DATA: {
            m_iCurDataType = DATA_FILE_TYPE_NONE;
        }
            break;

        case NOTIFY_DISPLAY_MODE:
            //  int iDisp = *((int *) pCom);
            setPaintMode( *((int *) pCom));
            break;

        case NOTIFY_SET_USE_ALT: {
            AltData ad = *((AltData*) pCom);
            float fAltFactor = ad.fAltFactor/1000;

            double dMin;
            double dMax;
            m_pPI->getLURange(&dMin, &dMax);
            m_pSurfaceOGL->setAltData(fAltFactor, dMin, dMax);

            setUseAlt(ad.bUseAlt);
            notifyObservers(0, (void*) NOTIFY_MODEL_REPAINT);
            }
            break;

        case NOTIFY_SET_USE_LIGHT:
            setUseLight(*((bool *) pCom));
            notifyObservers(0, (void*) NOTIFY_MODEL_REPAINT);
            break;
        }
        //    }
}


//-----------------------------------------------------------------------------
// clearMarkers
//
void IQModel::clearMarkers() {
    m_pFSpecial = NULL;
    m_pvHitNode= NULL;
    m_sNeighbors.clear();
}


