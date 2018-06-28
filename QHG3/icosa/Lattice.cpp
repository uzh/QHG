#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "strutils.h"
#include "LineReader.h"

#include "IcoLoc.h"
#include "Surface.h"

#include  "GeoInfo.h"
#include  "GridProjection.h"
#include  "Projector.h"
#include  "PolyFace.h"
#include  "VertexLinkage.h"

#include "Lattice.h"
#include "TriFace.h"
#include "QuadFace.h"

#define H4 1
#define H6 (sqrt(3)/2)

//-----------------------------------------------------------------------------
// constructor
//
Lattice::Lattice() 
    : m_iNumX(-1),
      m_iNumY(-1),
      m_iNumLinks(-1),
      
      m_pGP(NULL),
      m_bDeleteGP(false),
      m_iNumV(-1),
      m_apV(NULL),
      m_pVL(NULL),
      m_iNumPolys(0),
      m_apFaces(NULL),
      m_curBox(0, 1, 0, 1) {

}


//-----------------------------------------------------------------------------
// destructor
//
Lattice::~Lattice() {

    if (m_apV != NULL) {
        for (int i = 0; i < m_iNumV; i++) {
            delete m_apV[i];
        }
        delete[] m_apV;
    }
    if (m_pVL != NULL) {
        delete m_pVL;
    }
    if (m_bDeleteGP && (m_pGP != NULL)) {
        delete m_pGP;
    }

    if (m_apFaces != NULL) {
        for (int i = 0; i < m_iNumPolys; i++) {
            if (m_apFaces[i] != NULL) {
                delete m_apFaces[i];
            }
        }
        delete[] m_apFaces;
    }

    GeoInfo::free();
}


//-----------------------------------------------------------------------------
// create
//
int Lattice::create(int iNumLinks, GridProjection *pGP) {
    if (m_bDeleteGP && (m_pGP != NULL)) {
        delete m_pGP;
    }
    m_pGP       = pGP;
    m_bDeleteGP = false;
    int iResult = create(iNumLinks);
    return iResult;
}

//-----------------------------------------------------------------------------
// create
//
int Lattice::create(int iNumLinks, const char *pProjType, const char *pProjGrid) {
    int iResult = -1;
    if (m_bDeleteGP && (m_pGP != NULL)) {
        delete m_pGP;
    }
    if ((*pProjType != '\0') &&  (*pProjGrid != '\0')) {
        ProjType *pPT = new ProjType();
        ProjGrid *pPG = new ProjGrid();
        iResult = pPT->fromString(pProjType, true);
        if (iResult == 0) {
            iResult = pPG->fromString(pProjGrid);
            if (iResult == 0) {
                Projector *pProj = GeoInfo::instance()->createProjector(pPT);
                m_pGP = new GridProjection(pPG, pProj, true, true);
                m_bDeleteGP = true;
                iResult = create(iNumLinks);
            }
        }
        // pPT is not needed anymore (in pProj or otherwise)
        delete pPT;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// create
//
int Lattice::create(int iNumLinks) {
    int iResult = 0;

    if (m_apV != NULL) {
        for (int i = 0; i < m_iNumV; i++) {
            delete m_apV[i];
        }
        delete[] m_apV;
    }

    m_iNumLinks = iNumLinks;


                    printf("GP: G %dx%d R %fx%f O%f+%f\n",
                           m_pGP->getProjGrid()->m_iGridW,
                           m_pGP->getProjGrid()->m_iGridH,
                           m_pGP->getProjGrid()->m_dRealW,
                           m_pGP->getProjGrid()->m_dRealH,
                           m_pGP->getProjGrid()->m_dOffsX,
                           m_pGP->getProjGrid()->m_dOffsY);

    double dH = (m_iNumLinks == 4)?H4:H6;

    m_iNumX     = m_pGP->getProjGrid()->m_iGridW + 1;
    m_iNumY     = m_pGP->getProjGrid()->m_iGridH + 1;

    m_iNumV = m_iNumX *m_iNumY;
    printf("lattice has %dx%d=%d nodes\n", m_iNumX, m_iNumY, m_iNumV);

    m_curBox.dLonMin = 0;
//    m_curBox.dLonMax = m_iNumX;
    m_curBox.dLonMax = m_pGP->getProjGrid()->m_dRealW;
    m_curBox.dLatMin = 0;
//    m_curBox.dLatMax = m_iNumY;
    m_curBox.dLatMax = m_pGP->getProjGrid()->m_dRealH;

    m_apV   = new Vec3D*[m_iNumV];
    int iID = 0;
    for (int y = 0; y < m_iNumY; y++) {
        
        double dY = y * dH;
        for (int x = 0; x < m_iNumX; x++) {
            double dX = x;
            if (m_iNumLinks == 6) {
                dX += (y%2)*0.5;
            }
            m_apV[iID++] = new Vec3D(dX, dY, 0);
        }
    }
    printf("created %d nodes\n", iID);
    iResult = createRectVL();
    
    return iResult;
}


//-----------------------------------------------------------------------------
// createRectVL
//
int Lattice::createRectVL() {
    int iResult = 0;
    if (m_pVL != NULL) {
        delete m_pVL;
    }
    m_pVL = new VertexLinkage();

    if (m_apFaces != NULL) {
        for (int i = 0; i < m_iNumPolys; i++) {
            if (m_apFaces[i] != NULL) {
                delete m_apFaces[i];
            }
        }
        delete[] m_apFaces;
        m_apFaces=NULL;
    }

    switch (m_iNumLinks) {
    case 4:
        link4();
        break;
    case 6:
        link6();
        break;
    default:
        printf("No idea\n");
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// link4
//
void Lattice::link4() {
    m_iNumPolys = (m_iNumX-1)*(m_iNumY-1);
    printf("NumPolys(4): %d\n", m_iNumPolys);
    m_apFaces = new PolyFace*[m_iNumPolys];
    int aIDQ[4];
    int i = 0;
    for (int y = 0; y < m_iNumY-1; y++) {
        for (int x = 0; x < m_iNumX-1; x++) {
            aIDQ[0] = x+m_iNumX*y;
            aIDQ[1] = x+1+m_iNumX*y;
            aIDQ[2] = x+1+m_iNumX*(y+1);
            aIDQ[3] = x+m_iNumX*(y+1);

            m_apFaces[i] = QuadFace::createFace(m_apV[aIDQ[0]],
                                                m_apV[aIDQ[1]],
                                                m_apV[aIDQ[2]],
                                                m_apV[aIDQ[3]]);
            m_pVL->addPolyFace2(4,  m_apFaces[i], aIDQ);
            i++;
        }
    }
}

//-----------------------------------------------------------------------------
// link6
//
void Lattice::link6() {
    m_iNumPolys = 2*(m_iNumX-1)*(m_iNumY-1);
    printf("NumPolys(6): %d\n", m_iNumPolys);
    m_apFaces = new PolyFace*[m_iNumPolys];
    int aIDQ[3];
    int i = 0;

    for (int y = 0; y < m_iNumY-1; y++) {
        for (int x = 0; x < m_iNumX-1; x++) {
            if (y%2==0) {
                aIDQ[0] = x+m_iNumX*y;
                aIDQ[1] = x+1+m_iNumX*y;
                aIDQ[2] = x+m_iNumX*(y+1); 
                
                m_apFaces[i] = TriFace::createFace(m_apV[aIDQ[0]],
                                                   m_apV[aIDQ[1]],
                                                   m_apV[aIDQ[2]]);
                
                m_pVL->addPolyFace2(3, m_apFaces[i], aIDQ);
                i++;
                
                aIDQ[0] = x+1+m_iNumX*y;
                aIDQ[1] = x+1+m_iNumX*(y+1);
                aIDQ[2] = x+m_iNumX*(y+1);
                
                m_apFaces[i] = TriFace::createFace(m_apV[aIDQ[0]],
                                                   m_apV[aIDQ[1]],
                                                   m_apV[aIDQ[2]]);
                
                m_pVL->addPolyFace2(3, m_apFaces[i], aIDQ);
                i++;
            } else {
                aIDQ[0] = x+m_iNumX*y;
                aIDQ[1] = x+1+m_iNumX*(y+1);
                aIDQ[2] = x+m_iNumX*(y+1);
                     
                m_apFaces[i] = TriFace::createFace(m_apV[aIDQ[0]],
                                                   m_apV[aIDQ[1]],
                                                   m_apV[aIDQ[2]]);
           
                m_pVL->addPolyFace2(3, m_apFaces[i], aIDQ);
                i++;
                
                aIDQ[0] = x+m_iNumX*y;
                aIDQ[1] = x+1+m_iNumX*y;
                aIDQ[2] = x+1+m_iNumX*(y+1);
                
                m_apFaces[i] = TriFace::createFace(m_apV[aIDQ[0]],
                                                   m_apV[aIDQ[1]],
                                                   m_apV[aIDQ[2]]);
                m_pVL->addPolyFace2(3, m_apFaces[i], aIDQ);
                i++;
            }
        }
    }
}



   
//-----------------------------------------------------------------------------
// save
//   from Surface
//
int Lattice::save(const char *pFile) { 
    int iResult = -1;
    if ((m_pGP != NULL) && (m_iNumLinks > 0)) {
        FILE *fOut = fopen(pFile, "wt");
        if (fOut != NULL) {
            const ProjGrid  *pPG = m_pGP->getProjGrid();
            const Projector *pPR = m_pGP->getProjector();
            ProjType PT(pPR->getID(), pPR->getLambda0(), pPR->getPhi0(), 0, NULL);
            fprintf(fOut, "LTC3\n");
            fprintf(fOut, "NUMLINKS %d\n", m_iNumLinks);
            fprintf(fOut, "PROJT %s\n", PT.toString(true));
            fprintf(fOut, "PROJG %s\n", pPG->toString());
            fclose(fOut);
            iResult = 0;
        } else {
            printf("Couldn't open [%s]\n", pFile);
        }
    } else {
        printf("No Lattice defined\n");
    }

    if (iResult == 0) {
        printf("+++ successfully saved surface [%s]\n", pFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// load
//   from Surface
//
int Lattice::load(const char *pFile) { 
    int iResult = -1;

    LineReader *pLR = LineReader_std::createInstance(pFile, "rt");
    if (pLR != NULL) {
        ProjType *pPT = new ProjType();
        ProjGrid *pPG = new ProjGrid();
        int iNumLinks = -1;
        char *pLine = pLR->getNextLine();
        if (strncmp(pLine, "LTC3", 4) == 0) {
            iResult = 0;
            while ((iResult == 0) && (!pLR->isEoF())) {
                pLine = pLR->getNextLine();
                if (pLine != NULL) {
                    if (strncmp(pLine, "NUMLINKS", 8) == 0) {
                        pLine = trim(pLine+8);
                        iNumLinks = atoi(pLine);
                    } else if (strncmp(pLine, "PROJT", 5) == 0) {
                        pLine = trim(pLine+5);
                        iResult = pPT->fromString(pLine, true);
                    } else if (strncmp(pLine, "PROJG", 5) == 0) {
                        pLine = trim(pLine+5);
                        iResult = pPG->fromString(pLine);
                    }
                }
            }
            if (iResult == 0) {
                if (iNumLinks > 0) {

                    printf("PT: %s\n", pPT->toString(true));
                    printf("PG: %s\n", pPG->toString());

                    if (m_bDeleteGP && (m_pGP != NULL)) {
                        delete m_pGP;
                    }

                    Projector *pProj = GeoInfo::instance()->createProjector(pPT);
                    m_pGP = new GridProjection(pPG, pProj, true, true);
                    m_bDeleteGP = true;
                    printf("GP: G %dx%d R %fx%f O%f+%f\n",
                           m_pGP->getProjGrid()->m_iGridW,
                           m_pGP->getProjGrid()->m_iGridH,
                           m_pGP->getProjGrid()->m_dRealW,
                           m_pGP->getProjGrid()->m_dRealH,
                           m_pGP->getProjGrid()->m_dOffsX,
                           m_pGP->getProjGrid()->m_dOffsY);
                    //Don't need to delete pPG - GridProvider does
                    //this
                    printf("creating %d-lattice\n", iNumLinks);
                    iResult = create(iNumLinks);
                } else {
                    printf("Bad value for num links [%s]\n", pLine);
                }

            } else {
                printf("Bad format for ProjType or projGrid [%s]\n", pLine);
            }
        } else {
            printf("Bad magic number [%s]\n", pLine);
        }
        delete pPT;
        delete pLR;
    } else {
        printf("Couldn't open [%s]\n", pFile);
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// findNode
//  IcoLoc implementation
//  fin id of node closest to coords
//  
gridtype Lattice::findNode(Vec3D *pv) {
    gridtype iID = -1;
    double dX = pv->m_fX;
    double dY = pv->m_fY;
//    printf("finding node for %f, %f\n", dX, dY);
    if (m_iNumLinks == 6) {
//        dY /= H6;
        if (((int)(floor(dY)))%2 == 0) {
            dX -= 0.5;
        }

    }
    if (dX <= -0.5) {
        dX = 0;
    }
    if (dX >= m_iNumX-0.5) {
        dX = m_iNumX-1;
    }
    if (dY <= -0.5) {
        dY = 0;
    }
    if (dY >= m_iNumY-0.5) {
        dY = m_iNumY-1;
    }
    //    if ((dX > -0.5) && (dX < m_iNumX-0.5) && (dY > -0.5) && (dY < m_iNumY-0.5)) {
    int j = (int)round(dX);
    int i = (int)round(dY);
    iID = j +i*m_iNumX;
        //    }
    return iID;
}

//-----------------------------------------------------------------------------
// findNode
//  IcoLoc implementation
//  fin id of node closest to coords
//  
gridtype Lattice::findNode(double dLon, double dLat) {
    double dX;
    double dY;
    m_pGP->sphereToGrid(dLon, dLat, dX, dY);
    Vec3D v(dX, dY, 0);

    return findNode(&v);
}

//-----------------------------------------------------------------------------
// findCoords
//   IcoLoc implementation
//   find longitude and latitude of node with given ID
//
bool Lattice::findCoords(int iNodeID, double *pdLon, double *pdLat) {
    bool bOK = false;
    Vec3D *pV = m_pVL->getVertex(iNodeID);
    if (pV != NULL) {
        m_pGP->gridToSphere(pV->m_fX, pV->m_fY, *pdLon, *pdLat); 
        bOK = true;
    }
    return bOK;
}

#define QUADOF(N) ((N)-(N)/m_iNumX)
//-----------------------------------------------------------------------------
// findFace
//  IcoLoc implementation
//  fin id of node closest to coords
//  
PolyFace *Lattice::findFace(double dLon, double dLat) {
    PolyFace *pPF = NULL;
    double dX;
    double dY;
    m_pGP->sphereToGrid(dLon, dLat, dX, dY);
    Vec3D v(dX, dY, 0);

    gridtype iID = findNode(&v);
    int iIndex[6];
    for (int i = 0; i < 6; i++) {
        iIndex[i] = -1;
    }
    int iX = iID%m_iNumX;
    int iY = iID/m_iNumX;

    if (m_iNumLinks == 4) {
        if ((iY < m_iNumY-1) && (iX < m_iNumX-1)) {
            iIndex[0] = QUADOF(iID);
        }
        if ((iY < m_iNumY-1) && (iX > 0)) {
            iIndex[1] = QUADOF(iID-1);
        }
        if ((iY > 0) && (iX > 0)) {
            iIndex[2] = QUADOF(iID-m_iNumX-1);
        }
        if ((iY > 0) && (iX < m_iNumX-1)) {
            iIndex[3] = QUADOF(iID-m_iNumX);
        }

        /*
        if (iY < m_iNumY-1) {
            iIndex[0] = QUADOF(iID);
        

            if (iY > 0) {
                iIndex[3] = QUADOF(iID-m_iNumX);
            }
        }

        if (iX > 0) {
            if (iY < m_iNumY-1) {
                iIndex[1] = QUADOF(iID-1);
            }
            
            if (iY > 0) {
                iIndex[2] = QUADOF(iID-m_iNumX-1);
            }
        } 
        */
    } else {
        int iS = 2*QUADOF(iID);
        if (iX%2 == 1) {
            // x odd

            // all possible candidates
            iIndex[0] = 2*iS;
            iIndex[1] = 2*iS-1;
            iIndex[2] = 2*iS-2;
            iIndex[3] = 2*(iS-(m_iNumX-1));
            iIndex[4] = 2*(iS-(m_iNumX-1))+1;
            iIndex[5] = 2*(iS-(m_iNumX-1))+2;

            // remove top and bottom edge outliers
            if (iY == 0) {
                iIndex[3] = -1;
                iIndex[4] = -1;
                iIndex[5] = -1;
            } else if (iY == m_iNumY-1) {
                iIndex[0] = -1;
                iIndex[1] = -1;
                iIndex[2] = -1;
            } 

            // remove left and right outliers
            if (iX == 0) {
                iIndex[1] = -1;
                iIndex[2] = -1;
                iIndex[3] = -1;
                iIndex[4] = -1;
            } else if (iX == m_iNumX) {
                iIndex[0] = -1;
                iIndex[5] = -1;
            }
        } else {
            // x even

            // all possible candidates
            iIndex[0] = 2*iS+1;
            iIndex[1] = 2*iS;
            iIndex[2] = 2*iS-1;
            iIndex[3] = 2*(iS-(m_iNumX-1))-1;
            iIndex[4] = 2*(iS-(m_iNumX-1));
            iIndex[5] = 2*(iS-(m_iNumX-1))+1;

            // remove top and bottom edge outliers
            if (iY == 0) {
                iIndex[3] = -1;
                iIndex[4] = -1;
                iIndex[5] = -1;
            } else if (iY == m_iNumY-1) {
                iIndex[0] = -1;
                iIndex[1] = -1;
                iIndex[2] = -1;
            }

            // remove left and right outliers
            if (iX == 0) {
                iIndex[2] = -1;
                iIndex[3] = -1;
            } else if (iX == m_iNumX) {
                iIndex[0] = -1;
                iIndex[1] = -1;
                iIndex[4] = -1;
                iIndex[5] = -1;
            }

        }
    }
    
    // check remaining candidates
    for (int i = 0; (pPF == NULL) &&(i < m_iNumLinks); i++) {
        if (iIndex[i] >= 0) {
            if (m_apFaces[iIndex[i]]->contains(&v)) {
                pPF = m_apFaces[iIndex[i]];
            }
        }
    }
    return pPF;
    
}


//-----------------------------------------------------------------------------
// display
//
void Lattice::display() {
    for (int i = 0; i < m_iNumPolys; i++) {
        PolyFace *pPF = m_apFaces[i];
        printf("%03d (%d) :", i, pPF->getNumVertices());
        for (int j = 0; j < pPF->getNumVertices(); j++) {
            //            printf(" %03lld", m_pVL->getVertexID(pPF->getVertex(j)));
            printf(" %03d", pPF->getVertexID(j));
        }
        printf("\n");
    }
}
