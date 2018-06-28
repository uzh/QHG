/*****************************************************************************
* EQTriangle
* 
*   math based on
*   Max Tegmark (1996), 
*   "An Icosahedron-based Method for Pixelizing the Celestial Sphere"
*   (ApJ Letters, 470)
*
*---------
| Base triangle is used to define nodes and triangles
|
|  6----7----8----9
|  | 4 /| 6 /| 8 /
|  |  / |  / |  /
|  | /  | /  | /
|  |/ 5 |/ 7 |/
|  3----4----5
|  | 1 /| 3 / 
|  |  / |  /  
|  | /  | /   
|  |/ 2 |/    
|  1----2
|  | 0 /
|  |  /
|  | / 
|  |/  
|  0
* ***************************************************************************/ 

#include <stdio.h>
#include <math.h>
#include <string.h>



#include "strutils.h"
#include "LineReader.h"
#include "Vec3D.h"
#include "Quat.h"
#include "Tegmark.h"
#include "EQTriangle.h"

const double D0  = (1.0/(2*sqrt(3)));

const double S3  = sqrt(3);
const double T0  = tan(M_PI/5);
// side of projected ico-face (inscribed in unit sphere) 
const double A   = sqrt(9*T0*T0-3);
/*
// proportionality factor
const double ETA = A*sqrt(5*S3/(4*M_PI)); // 1.09843845
*/
const double EPS = 1e-6;

const double C2PI3 = cos(2*M_PI/3);
const double S2PI3 = sin(2*M_PI/3);

#define KEY_START "IEQ3"
#define KEY_LEVEL "LEVEL:"
#define KEY_VERT  "VERTICES:"
#define KEY_TRI   "TRIANGLES:"

#define NUM_ITEMS 10000

//--------------------------------------------------------------
// createInstance
//
EQTriangle *EQTriangle::createInstance(int iSubDiv, bool bTegmark) {
    EQTriangle *pEQ = new EQTriangle();
    int iResult = pEQ->init(iSubDiv, bTegmark);
    if (iResult != 0) {
        delete pEQ;
        pEQ = NULL;
    }
    return pEQ;
}

//--------------------------------------------------------------
// createEmpty
//
EQTriangle *EQTriangle::createEmpty() {
    return new EQTriangle();
}


//--------------------------------------------------------------
// destructor
//
EQTriangle::~EQTriangle() {
    if (m_pNodes != NULL) {
        delete[] m_pNodes;
    }
    if (m_pTriangles != NULL) {
        delete[] m_pTriangles;
    }
}

//--------------------------------------------------------------
// constructor
//
EQTriangle::EQTriangle()
    : m_iN(0),
      m_iNumNodes(0),
      m_pNodes(NULL),
      m_iNumTris(0),
      m_pTriangles(NULL),
      m_bMirr(false),
      m_bTegmark(true)  {

}

//--------------------------------------------------------------
// copy
//
EQTriangle *EQTriangle::copy() {
    EQTriangle *pEQ = new EQTriangle();
    pEQ->m_iN = m_iN;
    pEQ->m_iNumNodes = m_iNumNodes;
    pEQ->m_pNodes = new node[m_iNumNodes];
    memcpy(pEQ->m_pNodes, m_pNodes, m_iNumNodes*sizeof(node));

    pEQ->m_iNumTris = m_iNumTris;
    pEQ->m_pTriangles = new triangle[m_iNumTris];
    memcpy(pEQ->m_pTriangles, m_pTriangles, m_iNumTris*sizeof(triangle));
    pEQ->m_bTegmark = m_bTegmark;
    return pEQ;
}

//--------------------------------------------------------------
// init
//
int EQTriangle::init(int iSubDiv, bool bTegmark) {
    m_bTegmark = bTegmark;
    // A is the side of a icosahedron face radially projected 
    // to the tangent space of the containing sphere 
    int iResult = createSubdividedTriangle(A, iSubDiv);
    if (iResult == 0) {
//        saveNodes("normaltri.dat", false);
        if (m_bTegmark) {
            transformTriangle();
        }
        
    }
    return iResult;
}

//--------------------------------------------------------------
// save
//
void EQTriangle::deleteTriangles() {
    if (m_pTriangles != NULL) {
        delete[] m_pTriangles;
    }
    m_pTriangles = NULL;
}

//--------------------------------------------------------------
// createSubdividedTriangle
// 
//  creates a 0-centered equilateral triangle        ----
//  with horizontal top side and subdivide it,       \  /
//  keeping track of neighborship and subtriangles    \/ 
//  
//  This is done by starting from the point set      +--
//  {(x,y): 0 <= y <= N, 0 <= x <= y} (which forms   | /
//  a right triangle). In this setup neighborhood    |/
//  relations are easily determined.
//  At the same time the nodes' coordinates are calculated
//  to form the equilateral triangle.
//                                         
int EQTriangle::createSubdividedTriangle(double dSide, int iSubDiv) {
    m_iN = iSubDiv+1;
    m_iNumNodes = (m_iN+1)*(m_iN+2)/2;
    m_iNumTris = m_iN*m_iN;
    int i = 0;
    int j = 0;
 
    m_pNodes = new node[m_iNumNodes];
    m_pTriangles = new triangle[m_iNumTris];
    memset(m_pNodes, 0, m_iNumNodes*sizeof(node));
    memset(m_pTriangles, 0, m_iNumTris*sizeof(triangle));

    for (int y = 0; y <= m_iN; y++) {
        for (int x = 0; x <= y; x++) {
            node &nCur = (m_pNodes)[i]; 
            // calculate coordinates in equilateral triangle
            shearScale(x, y, &nCur.v);

       
            // define downward pointing triangle at current point (*)
            //  ___
            // |  /
            // | /
            // |/
            // *
            // (if exists)
            if (y < m_iN) {
                triangle &t = m_pTriangles[j];
                t.aiIdx[0] = i;
                t.aiIdx[1] = i+y+2;
                t.aiIdx[2] = i+y+1;
                j++;
            }
            

            // define upward pointing triangle at point (*)
            //          .
            //    /|    .
            //   / |    .  
            //  /  |    .
            // *---+    .
            // (if exists)
            if ((x < y) && (y < m_iN)) {
                triangle &t = m_pTriangles[j];
                t.aiIdx[0] = i;
                t.aiIdx[1] = i+1;
                t.aiIdx[2] = i+y+2;
                j++;
            }

            i++;
        }
        
    }

    /*
    int iCur = 0;
    for (int y = 0; y <= m_iN; y++) {
        for (int x = 0; x <= y; x++) {
            node &nCur = (m_pNodes)[iCur]; 
            Vec3D test;
            invShearScale(nCur.v.m_fX, nCur.v.m_fY, &test);
            printf("%d,%d --- %f,%f --- %f,%f\n", x, y, nCur.v.m_fX, nCur.v.m_fY,test.m_fX, test.m_fY);
            iCur++;
        }
    }
    */

    return 0;
}


//--------------------------------------------------------------
// shearScale
//   base triangle to equilateral
//
void EQTriangle::shearScale(double dX, double dY, Vec3D *pV) {
    pV->m_fX = A*((dX-dY/2)/m_iN);
    pV->m_fY = A*(D0-(m_iN-dY)*S3/(2*m_iN));
    pV->m_fZ = 0;
}


//--------------------------------------------------------------
// invShearScale
//   equilateral to base triangle
//
void EQTriangle::invShearScale(double dX, double dY, Vec3D *pV) {
    double dY1 = (dY/A-D0)*2*m_iN/S3 + m_iN;
    pV->m_fX = round((1.0*m_iN)*dX/A +dY1/2);
    pV->m_fY = round(dY1);
    pV->m_fZ = 0;    
}


//--------------------------------------------------------------
// transformSeedNodes
//   transform all nodes in the seed triangle:
//   the upper right subtriangle bordered by
//     x >= 0,  y >= x/sqrt(3)
//
void  EQTriangle::transformSeedNodes() {
    gridtype iSelected = 0;
    gridtype *piSelected = nodesInSeedTriangle(&iSelected);

    for (gridtype k = 0; k < iSelected; k++) {
        double dX1;
        double dY1;
        double dX = m_pNodes[piSelected[k]].v.m_fX;
        double dY = m_pNodes[piSelected[k]].v.m_fY;
        Tegmark::distortLocal(dX, dY, &dX1, &dY1);
        m_pNodes[piSelected[k]].v.m_fX = dX1;
        m_pNodes[piSelected[k]].v.m_fY = dY1;
    }
    delete[] piSelected;
}


//--------------------------------------------------------------
// nodesInSeedTriangle
//   select nodes in upper right subrectangle bordered by
//   x >= 0,  y >= x/sqrt(3)
//   must block equality in mirrored case to prevent points 
//   being transformed twice
//
gridtype *EQTriangle::nodesInSeedTriangle(gridtype *piSelected) {
    int iSel = 0;
    // some magic to determine maximum number of selected nodes
    //    the seed triangle contains one sixth of all nodes,
    //    but the m_iN/2 "spartial "nodes (shared with other triangles)
    //    have to be completed (m_iN/4)
    //
    //  in a mirrored situation the number may be smaller
    gridtype *aiSelected = new gridtype[1+int(1.0*m_iNumNodes/6.0+m_iN/4.0)];
  
    for (gridtype i = 0; i < m_iNumNodes; i++) {
        if ((m_pNodes[i].v.m_fX + (m_bMirr?-1:1)*EPS >= 0) && 
            (m_pNodes[i].v.m_fY + (m_bMirr?-1:1)*EPS >= m_pNodes[i].v.m_fX/sqrt(3))) {
            aiSelected[iSel++] = i;
        }
    }
    *piSelected = iSel;
    return aiSelected;
}

//--------------------------------------------------------------
// transformTriangle
//   mirror and rotate the initial seed triangle so that the
//   entire base triangle is covered. transform the nodes in 
//   the seed triangle after every symmetry operation
//
//   D3 symmetry:
//   Generators:
//     1  identity
//     m  mirror on y axis 
//     r  rotate 120°
//
//   with
//     m°m = 1
//     r°r°r = 1 
//     m°r = r'°m
//   therefore
//     r' = r°r
//
//   multiplication table
//    °  |  1  |  m  |  r  |  r' | m°r | r°m
//  -----+-----+-----+-----+-----+-----+-----+
//    1  |  1  |  m  |  r  |  r' | m°r | r°m |
//  -----+-----+-----+-----+-----+-----+-----+
//    m  |  m  |  1  | m°r | r°m |  r  |  r' |
//  -----+-----+-----+-----+-----+-----+-----+
//    r  |  r  | r°m |  r' |  1  |  m  | m°r |
//  -----+-----+-----+-----+-----+-----+-----+
//    r' |  r' | m°r |  1  |  r  | r°m |  m  |
//  -----+-----+-----+-----+-----+-----+-----+
//   m°r | m°r |  r' | r°m |  m  |  1  |  r  |
//  -----+-----+-----+-----+-----+-----+-----+
//   r°m | r°m |  r  |  m  | m°r |  r' |  1  |
//  -----+-----+-----+-----+-----+-----+-----+
//
void EQTriangle::transformTriangle() {

    // position 1
    transformSeedNodes();

    // position r
    rotateNodes120();
    transformSeedNodes();

    // position r°r = r'
    rotateNodes120();
    transformSeedNodes();

    // position m°r°r = m°r' = r°m
    mirrorNodesX();
    transformSeedNodes();

    // position r°m°r°r = r°m°r' = r°r°m = r'°m = m°r
    rotateNodes120();
    transformSeedNodes();

    // position r°r°m°r°r = r°r°m°r' = r°r°r°m = m 
    rotateNodes120();
    transformSeedNodes();

    // back to 1
    mirrorNodesX();
}


//--------------------------------------------------------------
// mirrorNodesX
//    mirror all nodes on x = 0 (in plane z=0)
//
void  EQTriangle::mirrorNodesX() {

    for (gridtype k  =0; k < m_iNumNodes; k++) {
        m_pNodes[k].v.m_fX = -m_pNodes[k].v.m_fX;
    }
    m_bMirr = !m_bMirr;
}


//--------------------------------------------------------------
// rotateNodes
//    rotate all nodes by given angle (in plane z=0)
//
void  EQTriangle::rotateNodes120() {
    for (gridtype k = 0; k < m_iNumNodes; k++) {
        double dX1 = C2PI3*m_pNodes[k].v.m_fX - S2PI3*m_pNodes[k].v.m_fY;
        double dY1 = S2PI3*m_pNodes[k].v.m_fX + C2PI3*m_pNodes[k].v.m_fY;
        m_pNodes[k].v.m_fX = dX1;
        m_pNodes[k].v.m_fY = dY1;
    }
}

//--------------------------------------------------------------
// applyQuat
//  apply rotation given by quaternion to all nodes
//
void EQTriangle::applyQuat(Quat *pQ) {
    Quat qRes;
    Quat qV;
    Quat *qInv = new Quat(pQ);
    qInv->invert();

    for (gridtype i = 0; i < m_iNumNodes; i++) {
        qV.m_fR = 0;
        qV.m_fI = m_pNodes[i].v.m_fX;
        qV.m_fJ = m_pNodes[i].v.m_fY;
        qV.m_fK = m_pNodes[i].v.m_fZ;
        pQ->mult(&qV, &qRes);
        qRes.mult(qInv, &qRes);
        m_pNodes[i].v.m_fX = qRes.m_fI;
        m_pNodes[i].v.m_fY = qRes.m_fJ;
        m_pNodes[i].v.m_fZ = qRes.m_fK;
    }
    delete qInv;
}


//--------------------------------------------------------------
// shift
//   apply translation to al nodes
//
void EQTriangle::shift(Vec3D *pvShift) {

    for (gridtype i = 0; i < m_iNumNodes; i++) {
        m_pNodes[i].v.add(pvShift);
    }
}


//--------------------------------------------------------------
// scale
//   apply scale
//
void EQTriangle::scale(double dScale) {

    for (gridtype i = 0; i < m_iNumNodes; i++) {
        m_pNodes[i].v.scale(dScale);
    }
}


//--------------------------------------------------------------
// normalize
//   normalize (i.e. push all nodes to the sphere)
//
void EQTriangle::normalize() {

    for (gridtype i = 0; i < m_iNumNodes; i++) {
        m_pNodes[i].v.normalize();
    }
}


//--------------------------------------------------------------
// getCorner
//   return coordinates of the (transformed) triangle's i-th corner
//
Vec3D *EQTriangle::getCorner(int i) {
    Vec3D *pRes = NULL;
    int iIndex = -1;
    switch (i) {
    case 0:
        iIndex = 0;
        break;
    case 1:
        iIndex = m_iNumNodes - 1;
        break;
    case 2:
        iIndex = m_iNumNodes - m_iN - 1;
        break;
    }
    if (iIndex >= 0) {
        pRes = &(m_pNodes[iIndex].v);
    }
    return pRes;
}


//--------------------------------------------------------------
// findNeighbors
//  Goal: find IDs of nodes surrounding pV.
//  apply inverse tegmark transform to the point in the 
//  projected ico. Then shear and shift it to the base triangle
//  and get base triangle coordinates r and c.
//  Find neighbor IDs in base trianlgle.
//
int EQTriangle::findNeighbors(Vec3D *pV, gridtype *aiNeighborIDs) {
    int iCount = 0;
 
    // reverse the tegmark transformation
    if (m_bTegmark) {
        pV = Tegmark::straighten(pV);
    }

    double s = 1.0/m_iN;
    double h = s*sqrt(3)/2;

    // transform to rectangular base triangle
    double dY = (pV->m_fY/A-D0)/h+m_iN;
    double dX = pV->m_fX*(m_iN/A)+dY/2;
   
    // get base coordinates
    int r = (int)trunc(dY+EPS);
    int c = (int)trunc(dX+EPS);
 
    // sanity check
    if ((r >= 0) && (r <= m_iN) && (c >= 0) && (c <= r)) {
        // collect neighbors        
        // the node below and to the left of (dX,dY)
        aiNeighborIDs[iCount++] = r*(r+1)/2+c;
        if (r < m_iN) {
            // the node above and to the left of (dX,dY)
            aiNeighborIDs[iCount++] = (r+1)*(r+2)/2+c;
        }
        if (c < r) {
            // the node below and to the right of (dX,dY)
            aiNeighborIDs[iCount++] = r*(r+1)/2+c+1;
        }

        if ((r < m_iN) && (c <= r)) {
            // the node above and to the right of (dX,dY)
            aiNeighborIDs[iCount++] = (r+1)*(r+2)/2+c+1;
        }

    } else {
        // this shouldn't happen
        printf("FN Point (%f %f %f) -> (r %d; c %d) N:%d: not in rectangular base triangle\n", pV->m_fX, pV->m_fY, pV->m_fZ, r, c, m_iN);
    }
    return iCount;
}


//--------------------------------------------------------------
// findFaceID
//
gridtype EQTriangle::findFaceID(Vec3D *pV) {
    gridtype lID = -1;

    // reverse the tegmark transformation
    if (m_bTegmark) {
        pV = Tegmark::straighten(pV);
    }

    double s = 1.0/m_iN;
    double h = s*sqrt(3)/2;

    // transform to rectangular base triangle
    double dY = (pV->m_fY/A-D0)/h+m_iN;
    double dX = pV->m_fX*(m_iN/A)+dY/2;
   
    // get base coordinates
    int r = (int)trunc(dY+EPS);
    int c = (int)trunc(dX+EPS);
 
    // sanity check
    if ((r >= 0) && (r <= m_iN) && (c >= 0) && (c <= r)) {
        double fX = dX - c;
        double fY = dY - r;
 
        int iUpwardTri = (fY > fX)?0:1;

        // the index of the first (down-pointing) triangle in row r is r^2
        // for each c there is an up-pointing and a downpointing triangle
        lID = r*r + 2*c +iUpwardTri;
    } else {
        // this shouldn't happen
        printf("FN Point (%f %f %f) -> (r %d; c %d) N:%d: not in rectangular base triangle\n", pV->m_fX, pV->m_fY, pV->m_fZ, r, c, m_iN);
    }

    return lID;
}


//--------------------------------------------------------------
// displayNodes
//
void EQTriangle::displayNodes(bool bWithTriangles) {
    printf("Have %d nodes\n", m_iNumNodes);
    for (gridtype i = 0; i < m_iNumNodes; i++) {
        printf("%d %d % f % f  ", i, m_pNodes[i].lID, m_pNodes[i].v.m_fX, m_pNodes[i].v.m_fY);
        printf("\n");
            
    }

    if (bWithTriangles) {
        printf("Have %d triangles\n", m_iNumTris);
        for (gridtype i = 0; i < m_iNumTris; i++) {
            printf("%d  %d %d %d\n", i, m_pTriangles[i].aiIdx[0], m_pTriangles[i].aiIdx[1], m_pTriangles[i].aiIdx[2]);
        }
    }

}


//--------------------------------------------------------------
// saveNodes
//
int EQTriangle::saveNodes(const char * pNodeFile, bool bWithTriangles) {
    int iResult = -1;
    FILE *fOut = fopen(pNodeFile, "wt");
    if (fOut != NULL) {
        fprintf(fOut, "# nodes: %d\n", m_iNumNodes);
        for (gridtype i = 0; i < m_iNumNodes; i++) {
            fprintf(fOut, "%d % f % f % f ", m_pNodes[i].lID, m_pNodes[i].v.m_fX, m_pNodes[i].v.m_fY, m_pNodes[i].v.m_fZ);
            fprintf(fOut, "\n");
        }
 
    }
    if (bWithTriangles) {
        fprintf(fOut, "# triangles: %d\n", m_iNumTris);
        for (gridtype i = 0; i < m_iNumTris; i++) {
            fprintf(fOut, "%d  %d %d %d\n", i, m_pTriangles[i].aiIdx[0], m_pTriangles[i].aiIdx[1], m_pTriangles[i].aiIdx[2]);
        }
    }

    fclose(fOut);
    return iResult;
}


//--------------------------------------------------------------
// save
//
int EQTriangle::save(FILE *fOut) {
    int iResult = -1;

    iResult = 0;
    fprintf(fOut, "%s\n",     KEY_START);
    fprintf(fOut, "%s%d\n",   KEY_LEVEL, m_iN);
    fprintf(fOut, "%s%d\n",   KEY_VERT,  m_iNumNodes);
    fprintf(fOut, "%s%d\n",   KEY_TRI,   m_iNumTris);
    node *pDataN = m_pNodes;
    int iOut = 0;
    while ((iResult == 0) && (iOut < m_iNumNodes)) {
        gridtype iNum = m_iNumNodes - (gridtype)(pDataN-m_pNodes);
        if (iNum > NUM_ITEMS) {
            iNum = NUM_ITEMS;
        }
        ulong iWritten = fwrite(pDataN, sizeof(node), iNum, fOut);
        if (iWritten == (ulong)iNum) {
            pDataN += iNum;
            iOut += iNum;
        } else {
                printf("write error\n");
                iResult = -1;
        }
    }
    if (iResult == 0) {
        triangle *pDataT = m_pTriangles;
        iOut = 0;
        while ((iResult == 0) && (iOut < m_iNumTris)) {
            gridtype iNum = m_iNumTris - (gridtype)(pDataT-m_pTriangles);
            if (iNum > NUM_ITEMS) {
                iNum = NUM_ITEMS;
            }
            ulong iWritten = fwrite(pDataT, sizeof(triangle), iNum, fOut);
            if (iWritten == (ulong)iNum) {
                pDataT += iNum;
                iOut += iNum;
            } else {
                printf("write error\n");
                iResult = -1;
            }
        }
    }
    return iResult;
}


//--------------------------------------------------------------
// readHeader 
//
long EQTriangle::readHeader(FILE *fIn, long lStartPos) {
    long lPos = -1;
    LineReader *pLR = new LineReader_std(fIn);
    if (pLR != NULL) {
        //        printf("readHeader:need pos %ld, real pos %ld\n", lStartPos, pLR->tell());
        //        pLR->seek(lStartPos);
            
        char *pLine = pLR->getNextLine();
        if (pLine != NULL) {
            if (strcmp(trim(pLine), KEY_START) == 0) {
                pLine = pLR->getNextLine();
                if (pLine != NULL) {
                    pLine = trim(pLine);
                    if (strstr(pLine, KEY_LEVEL) == pLine) {
                        // extract level
                        char *p = strchr(pLine, ':')+1;
                        if (strToNum(p, &m_iN)) {

                            pLine = pLR->getNextLine();
                            if (pLine != NULL) {
                                pLine = trim(pLine);
                                if (strstr(pLine, KEY_VERT) == pLine) {
                                    // extract numverts
                                    p = strchr(pLine, ':')+1;
                                    if (strToNum(p, &m_iNumNodes)) {
                                    
                                        pLine = pLR->getNextLine();
                                        if (pLine != NULL) {
                                            pLine = trim(pLine);
                                            if (strstr(pLine, KEY_TRI) == pLine) {
                                                // extract numverts
                                                p = strchr(pLine, ':')+1;
                                                if (strToNum(p, &m_iNumTris)) {
                                                   
                                                    lPos= pLR->tell();
                                                } else {
                                                    printf("Invalid number [%s] for [%s]\n", p, KEY_VERT);
                                                }
                                            } else {
                                                printf("Expected [%s] instead of %s\n", KEY_TRI, pLine);
                                            }
                                        } else { 
                                            printf("Incomplete header\n");
                                        }
                                    } else {
                                        printf("Invalid number [%s] for [%s]\n", p, KEY_VERT);
                                    }
                                } else {
                                    printf("Expected [%s] instead of %s\n", KEY_VERT, pLine);
                                }
                            } else { 
                                printf("Incomplete header\n");
                            }
                        } else {
                            printf("Invalid number [%s] for [%s]\n", p, KEY_LEVEL);
                        }
                    } else { 
                        printf("Expected [%s] instead of %s\n", KEY_LEVEL, pLine);
                    }
                } else { 
                    printf("Incomplete header\n");
                }
            } else {
                printf("Bad magic: [%s]\n", KEY_START);
            }
        } else {
            printf("Empty file\n");
        }
        delete pLR;
    } else {
        printf("Couldn't not create LineReader\n");
    }

    return lPos;
}


//--------------------------------------------------------------
// load 
//
long EQTriangle::load(FILE *fIn, long lStartPos) {
    
    int iResult = -1;
    long lPos = readHeader(fIn, lStartPos);
    if (lPos > 0) {
        printf("Triangle reads %d nodes in %d triangles\n", m_iNumNodes, m_iNumTris);
        m_pNodes = new node[m_iNumNodes];
        m_pTriangles = new triangle[m_iNumTris];
        iResult = -1;
            iResult = 0;
            // fseek(fIn, lPos, SEEK_SET);

            int iIn = 0;
            node *pDataN = m_pNodes;
            while ((iResult == 0) && (iIn < m_iNumNodes)) {
                gridtype iNum = m_iNumNodes - gridtype(pDataN-m_pNodes);
                if (iNum > NUM_ITEMS) {
                    iNum = NUM_ITEMS;
                }
                ulong iRead = fread(pDataN, sizeof(node), iNum, fIn);
                if (iRead == (ulong)iNum) {
                    pDataN += iNum;
                    iIn += iNum;
                } else {
                    printf("read error\n");
                    iResult = -1;
                }
            }
            iIn = 0;
            triangle *pDataT = m_pTriangles;
            while ((iResult == 0) && (iIn < m_iNumTris)) {
                gridtype iNum = m_iNumTris - (gridtype)(pDataT-m_pTriangles);
                if (iNum > NUM_ITEMS) {
                    iNum = NUM_ITEMS;
                }
                ulong iRead = fread(pDataT, sizeof(triangle), iNum, fIn);
                if (iRead == (ulong)iNum) {
                    pDataT += iNum;
                    iIn += iNum;
                } else {
                    printf("read error\n");
                    iResult = -1;
                }
            }            
            
        } else {
        // error in header read
        }
    
    if (iResult == 0) {
        lPos = ftell(fIn);
    } else {
        lPos = -1;
    }
    return lPos;
}

