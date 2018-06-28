#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Vec3D.h"
#include "geomutils.h"
#include "ValReader.h"
#include "IcoFace.h"

const double EPS = 1e-8;


//-----------------------------------------------------------------------------
// createIcoFace 
//
IcoFace *IcoFace::createFace(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, bool bCheckLength) {
    IcoFace *pF = new IcoFace();
    int iResult = pF->init(pV1, pV2, pV3, bCheckLength);
    if (iResult == 0) {
        // other stuff?

    } else {
        printf("Have result %d: ", iResult);
        pF->display();
        delete pF;
        pF = NULL;
    }
    return pF;
}


//-----------------------------------------------------------------------------
// createIcoFace
//
IcoFace *IcoFace::createFace(Vec3D **ppV) {
    return createFace(ppV[0], ppV[1], ppV[2]);
}


//-----------------------------------------------------------------------------
// constructor
//
IcoFace::IcoFace() 
    : m_pNormal(NULL),
      m_bSubdivided(false),
      m_pNextSubFace(NULL),
      m_pFirstSubFace(NULL),
      m_iLevel(0),
      m_bKeep(false),
      m_iCompletionLevels(-1),
      m_dArea(-1),
      m_dOrigDist(-1) {

    bzero(m_apSubFaces, 4*sizeof(IcoFace*));
    bzero(m_apBorderFaces, 4*sizeof(Vec3D*));
}


//-----------------------------------------------------------------------------
// destructor
//
IcoFace::~IcoFace() {
    delete m_pNormal;
    for (int i = 0; i < 3; i++) {
        delete m_apBorderFaces[i];
    }

    if (m_bSubdivided) {
        for (int i = 0; i < 3; i++) {
            delete m_apSubFaces[0]->m_apVerts[i];
        }
        for (int i = 0; i < 4; i++) {
            if (m_apSubFaces[i] != NULL) {
                delete m_apSubFaces[i];
            }
        }
    }
}


//-----------------------------------------------------------------------------
// init
//
int IcoFace::init(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, bool bCheckLength) {
    int iResult = 0;
    setVerts(pV1, pV2, pV3);

    if (bCheckLength) {
        for (int i = 0; (iResult == 0) && (i < 3); i++) {
            double r = m_apVerts[i]->calcNorm();
            
            if (r >= -EPS) {
                m_apVerts[i]->scale(1/r);
                cart2Sphere(m_apVerts[i], &( m_adLons[i]), &(m_adLats[i]));
            } else {
                iResult = -1;
            }
            
        }
    }
    
    for (int i = 0; (iResult == 0) && (i < 3); i++) {
        m_apBorderFaces[i] = m_apVerts[i]->crossProduct(m_apVerts[(i+1)%3]);
        m_apBorderFaces[i]->normalize();
    }

    Vec3D vEdge0(m_apVerts[0]);
    vEdge0.subtract(m_apVerts[2]);
    Vec3D vEdge1(m_apVerts[1]);
    vEdge1.subtract(m_apVerts[0]);

    m_pNormal = vEdge0.crossProduct(&vEdge1);
    m_dArea = m_pNormal->calcNorm()/2;

    // face normal
    m_pNormal->normalize();
    // distance of face plane to origin
    m_dOrigDist = m_pNormal->dotProduct(m_apVerts[0]);

    /*
    //    printf("Borderface sanity checks:\n");
    for (int i = 0; (iResult == 0) && (i < 3); i++) {
        double d = m_apBorderFaces[i]->dotProduct(m_pNormal);
        if (d < -EPS) {
            printf("  borderface %d dot normal: %e\n", i, d);

        }
    }
    for (int i = 0; (iResult == 0) && (i < 3); i++) {
        for (int j = 0; (iResult == 0) && (j < 3); j++) {
            double d = m_apBorderFaces[i]->dotProduct(m_apVerts[j]);
            if (d < -EPS) {
                printf("  borderface %d dot vertex %d: %e\n", i, j, d);
            }
        }
    }
    */

    // find the largest angle between normal and any of the vertices
    m_fCosMaxAng = 2;
    for (int i = 0; (iResult == 0) && (i < 3); i++) {
        float f = (float)m_apVerts[i]->dotProduct(m_pNormal);
        if (f < m_fCosMaxAng) {
            m_fCosMaxAng = f;
        }
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// inBox
//   is the entire face inside the box given by min/max Lon/Lat?
//
bool IcoFace::inBox(double dThetaMin, double dThetaMax,double dPhiMin, double dPhiMax) {
    bool bIn = true;
    for (int i = 0;  bIn && (i < 3); i++) {
        bIn = (dThetaMin <= m_adLons[i]) && (m_adLons[i] <= dThetaMax) &&
            (dPhiMin <= m_adLats[i]) && (m_adLats[i] <= dPhiMax);
    }
    return bIn;
    
}
//-----------------------------------------------------------------------------
// inBox
//   is the entire face inside the box
//
bool IcoFace::inBox(tbox &tBox) {
    bool bIn = true;
    for (int i = 0;  bIn && (i < 3); i++) {
        bIn = (tBox.dLonMin <= m_adLons[i]) && (m_adLons[i] <= tBox.dLonMax) &&
            (tBox.dLatMin <= m_adLats[i]) && (m_adLats[i] <= tBox.dLatMax);
    }
    return bIn;
    
}

bool IcoFace::vertexInBox(tbox &tBox) {
    bool bIn = false;
    for (int i = 0;  !bIn && (i < 3); i++) {
        bIn = (tBox.dLonMin <= m_adLons[i]) && (m_adLons[i] <= tBox.dLonMax) &&
            (tBox.dLatMin <= m_adLats[i]) && (m_adLats[i] <= tBox.dLatMax);
    }
    return bIn;
    
}

//-----------------------------------------------------------------------------
// merge
//
void IcoFace::merge(int iNumLevels) {
    if (iNumLevels == 0) {
        if (m_bSubdivided) {
            // only delete the "internal" vertices, not the ones of the parent!
            for (int i = 0; i < 3; i++) {
                delete m_apSubFaces[0]->m_apVerts[i];
            }
            
            for (int i = 0; i < 4; i++) {
                delete m_apSubFaces[i];
                m_apSubFaces[i]=NULL;
            }
            m_bSubdivided = false;
        }
    } else if (m_bSubdivided)  {
        for (int i = 0; i < 4; i++) {
            m_apSubFaces[i]->merge(iNumLevels-1);
        }

    }

}


//-----------------------------------------------------------------------------
// subdivide
//
void IcoFace::subdivide() {
    // actual subdivision & filling of subFAces
    // create new vertexes
    Vec3D *pNew[3];
    for (int i = 0; i < 3; i++) {
        pNew[i] = new Vec3D(m_apVerts[i]);
        pNew[i]->add(m_apVerts[(i+1)%3]);
        pNew[i]->scale(0.5);
        // "round" it to float to avoid fuzz errors
        pNew[i]->trunc();
    }

    m_apSubFaces[0] = IcoFace::createFace(pNew[0], pNew[1], pNew[2]);
    m_apSubFaces[0]->setLevel(m_iLevel+1);
    for (int i = 0; i < 3; i++) {
        m_apSubFaces[i+1] = IcoFace::createFace(m_apVerts[i], pNew[i], pNew[(i+2)%3]);
        m_apSubFaces[i+1]->setLevel(m_iLevel+1);

    }

    m_bSubdivided = true;
}


//-----------------------------------------------------------------------------
// subdivide
//
void IcoFace::subdivide(int iNumLevels) {
    if (iNumLevels > 0) {
        if (!m_bSubdivided) {
            subdivide();
        }
        for (int i = 0; i < 4; i++) {
            m_apSubFaces[i]->subdivide(iNumLevels-1);
        }

        for (int i = 0; i < 3; i++) {
            m_apSubFaces[i]->setNext(m_apSubFaces[i+1]);
        }
        m_pFirstSubFace = m_apSubFaces[0];
    }
}

//-----------------------------------------------------------------------------
// subdivideLand
//
bool IcoFace::subdivideLand(ValReader *pVR, float fMinAlt, int iNumLevels) {
    bool bKeep = false;
    char s[32];
    memset(s, ' ', 2*m_iLevel);
    s[2*m_iLevel] = '\0';

    if (iNumLevels > 0) {
        if (!m_bSubdivided) {
            subdivide();
        }
        for (int i = 0; i < 4; i++) {
            bKeep = m_apSubFaces[i]->subdivideLand(pVR, fMinAlt, iNumLevels-1) || bKeep;
        }
        
        if (bKeep) {
            for (int i = 0; i < 3; i++) {
                m_apSubFaces[i]->setNext(m_apSubFaces[i+1]);
            }
            m_pFirstSubFace = m_apSubFaces[0];
        } else {
            merge(0);
        }

    } else {
        // last level : check nodes
        for (int i = 0;  (i < 3); i++) {
            double dLat = asin(m_apVerts[i]->m_fZ)*180/M_PI;
            double dLon = atan2(m_apVerts[i]->m_fY,m_apVerts[i]->m_fX)*180/M_PI;

            bKeep = (pVR->getDValue(dLon, dLat) > fMinAlt) || bKeep;
        }
       
    }
    return bKeep;
}


//-----------------------------------------------------------------------------
// setNext
//
void IcoFace::setNext(IcoFace *pF) {
    m_pNextSubFace = pF;
    /*    
    if (m_bSubdivided) {
        m_apSubFaces[3]->setNext(pF->m_apSubFaces[0]);
    }
    */
}

//-----------------------------------------------------------------------------
// getNextSubFace
//
IcoFace *IcoFace::getNextSubFace() {
    return m_pNextSubFace;
}

//-----------------------------------------------------------------------------
// getFirstSubFace
//
IcoFace *IcoFace::getFirstSubFace() {
    if (m_pFirstSubFace == NULL) {
        if (m_bSubdivided) {
            m_pFirstSubFace = m_apSubFaces[0]->getFirstSubFace();
        }
    }
    return m_pFirstSubFace;
}



//-----------------------------------------------------------------------------
// contains
//
PolyFace *IcoFace::contains(Vec3D *pP) {
    return contains(pP, EPS);
}


//-----------------------------------------------------------------------------
// contains
//
PolyFace *IcoFace::contains(Vec3D *pP, double dEps) {
    PolyFace *pHit = NULL;
    pP->normalize();
    // 1st test: point within circle passing through most 
    //           distant vertex of triangle
    // 
    //        printf("cosmaxang %f, this cosang %f, eps(%f)\n", (float)m_fCosMaxAng, (float)m_pNormal->dotProduct(pP), dEps);
    if ((float)m_pNormal->dotProduct(pP) > (float)m_fCosMaxAng - dEps) {
        
        bool bGoOn = true;
        // 2nd test: point on the inside halfspace of every pyramid side
        // (imagine pyramid going through sphere center and triangle points)
        for (int i = 0; bGoOn &&(i < 3); i++) {
            //                        printf("borderfaces %d: %15.13f ; EPS:%15.13f\n", i, (float)m_apBorderFaces[i]->dotProduct(pP), dEps);
            bGoOn = ((float)m_apBorderFaces[i]->dotProduct(pP) > -dEps);
        }
        // ok point is inside, let's see if we find it in a subtriangle
        if (bGoOn) {
            if (m_bSubdivided) {
                
                // we know it must be inside a subface
                for (int i = 0; (pHit == NULL) && (i < 4); i++) {
                    pHit = m_apSubFaces[i]->contains(pP, dEps);
                }
                
                double dEps1 = dEps;
                while (pHit == NULL) {
                    // no hit: point too close to border -> coarser epsilon
                    // repeat until you get a hit
                    dEps1*=2;
                    for (int i = 0; (pHit == NULL) && (i < 4); i++) {
                        pHit = m_apSubFaces[i]->contains(pP, dEps1);
                    }
                }
            } else {

                pHit = this;
            }
        }
    } // angle test
    return pHit;
}

//-----------------------------------------------------------------------------
// closestVertex
//
Vec3D *IcoFace::closestVertex(Vec3D *pP) {
    int iMin = -1;
    double dMin = 1e99;

    
    for (int i = 0; i < 3; i++) {
        //double d = spherdist(pP, m_apVerts[i], 1);
        double d = m_apVerts[i]->dist(pP);
        if (d < dMin) {
            iMin = i;
            dMin = d;
        }
    }
    return m_apVerts[iMin];
}

//-----------------------------------------------------------------------------
// closestVertex
//
gridtype IcoFace::closestVertexID(Vec3D *pP) {
    int iMin = -1;
    double dMin = 1e99;

    
    for (int i = 0; i < 3; i++) {
        //double d = spherdist(pP, m_apVerts[i], 1);
        double d = m_apVerts[i]->dist(pP);
        if (d < dMin) {
            iMin = i;
            dMin = d;
        }
    }
    return m_alIDs[iMin];
}


//-----------------------------------------------------------------------------
// display
//
void IcoFace::display() {
    printf("[");
    for (int i = 0; i < 3; i++) {
        printf("(%15.13f %15.13f %15.13f)", 
               m_apVerts[i]->m_fX, 
               m_apVerts[i]->m_fY, 
               m_apVerts[i]->m_fZ);
    }
    printf("]\n"); 
}
//-----------------------------------------------------------------------------
// displayLL
//
void IcoFace::displayLL() {
    printf("[");
    for (int i = 0; i < 3; i++) {
        printf("(%f %f)", 
               m_adLons[i]*180/M_PI, 
               m_adLats[i]*180/M_PI);
    }
    printf("]\n"); 
}

//-----------------------------------------------------------------------------
// displayRec
//
void IcoFace::displayRec(const char *pIndent) {
    printf("%s", pIndent);
    display();
    if (m_bSubdivided) {
        char s[256];
        sprintf(s, "  %s", pIndent);
        for (int i = 0; i < 4; i++) {
            m_apSubFaces[i]->displayRec(s);
        }
    }
}

//-----------------------------------------------------------------------------
// getNumIcoFaces
//
int IcoFace::getNumFaces() {
    int iFaces = 0;
    IcoFace *pF = getFirstSubFace();
    while (pF != NULL) {
        iFaces++;
        pF = pF->getNextSubFace();
    }
    return iFaces;
}


//-----------------------------------------------------------------------------
// calcCompletion
//  if the face has no subfaces it has completion level 0
//  if all subfaces have the same completion level CL, this face has completion level CL+1
//  otherwise, completion level is -1
// 
int IcoFace::calcCompletion() {
    bool bEqual = true;
    if (m_bSubdivided) {
        int iCL = m_apSubFaces[0]->calcCompletion();
        
        for (int i = 1; bEqual && (i < 4); i++) {
            int iC1 = m_apSubFaces[i]->calcCompletion();
            if (iCL != iC1) {
                bEqual = false;
            }
        }
        if (bEqual) {
            m_iCompletionLevels = iCL+1;
        } else {
            m_iCompletionLevels = -1;
        }
    } else {
        m_iCompletionLevels = 0;
    }
    return m_iCompletionLevels;
}

void IcoFace::planify(Vec3D *pV) {
    double dScale = m_dOrigDist/m_pNormal->dotProduct(pV);
    pV->scale(dScale);
}

//-----------------------------------------------------------------------------
// calcBary
//  calc barycentric coordinates for v in the triangle described by face
//  first, v is scaled so that it lies in the plane of face
//
void IcoFace::calcBary(Vec3D v, double *pdL1, double *pdL2) {
    // first make sure v lies in the face
    planify(&v);

    // calc bary coords
    Vec3D A00(m_apVerts[2]);
    A00.subtract(m_apVerts[1]);
    Vec3D A01(v);
    A01.subtract(m_apVerts[1]);
    *pdL1 = A00.getCrossSize(&A01)/(2*m_dArea);

    Vec3D A10(m_apVerts[0]);
    A10.subtract(m_apVerts[2]);
    Vec3D A11(v);
    A11.subtract(m_apVerts[2]);
    *pdL2 = A10.getCrossSize(&A11)/(2*m_dArea);
}
