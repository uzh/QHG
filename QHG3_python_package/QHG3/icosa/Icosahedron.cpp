
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stdlib.h>

#ifdef OMP
#include <omp.h>
#endif

#include "BufWriter.h"
#include "BufReader.h"
#include "Vec3D.h"
#include "utils.h"

#include "icoutil.h"
#include "IcoLoc.h"
#include "PolyFace.h"
#include "IcoFace.h"
#include "VertexLinkage.h"
#include "Icosahedron.h"
#include "IcoHeader.h"
#include "symbuf.h"
#include "IcoGridNodes.h"

#define ICO_BUF 4096

static int s_aiFaceNum[] = { 20, 8, 4 };
static int s_aiVertNum[] = { 12, 6, 4 };


#define OBR '('
#define DOT '.'
#define CBR ')'

#define PIPLUS (M_PI*1.00)
//-----------------------------------------------------------------------------
// constructor
//
Icosahedron::Icosahedron(double dRadius)
    : m_dR(dRadius),
      m_apMainVertices(NULL),
      m_apMainFaces(NULL),
      m_pVL(NULL),
      m_iNumMainFaces(0),
      m_iNumMainVerts(0),
      m_iPolyType(POLY_TYPE_NONE),
      m_iCurFace(0),
      m_iSubLevel(0),
      m_curBox(-M_PI,M_PI+1e-8,-M_PI/2, M_PI/2+1e-8),
      m_bPreSel(false),
      m_bStrict(true) {
    printf("%p: constr%s\n", this, "(normal)");

}

//-----------------------------------------------------------------------------
// destructor
//
Icosahedron::~Icosahedron() {
   
    if (m_apMainFaces != NULL) {
        for (int i = 0; i < m_iNumMainFaces; i++) {
               delete m_apMainFaces[i];
        }
        delete[] m_apMainFaces;
    }

    if(m_apMainVertices != NULL) {
        for (int i = 0; i < m_iNumMainVerts; i++) {
            delete m_apMainVertices[i];
        }
        delete[] m_apMainVertices;
    }

    if (m_pVL != NULL) {
        delete m_pVL;
    }

}

//-----------------------------------------------------------------------------
// calcSinAngle
//
double Icosahedron::calcSinAngle() {
    double dDPhi = 2*M_PI/5;
    double sps=sin(dDPhi/2);
    sps=2*sps*sps;
    double dSA = (1 - sqrt(1 - 4* sps*(1-sps)))/(2*sps);
    //    printf("dSA = %f -> (%f°)\n", dSA, 180*asin(dSA)/M_PI);
    return dSA;
}

//-----------------------------------------------------------------------------
// create
//  create an icosahedron (dRadius currntly without effect)
//
Icosahedron *Icosahedron::create(double dRadius, int iPolyType) {
    printf("create  - create%s\n", "(normal)");

    Icosahedron *pI = new Icosahedron(dRadius);
    int iResult = pI->init(iPolyType);
    if (iResult != 0) {
        delete pI;
        pI = NULL;
    }
    return pI;
}

//-----------------------------------------------------------------------------
// init
//  create the vertices and triangles of a icosahedron
//
int Icosahedron::init(int iPolyType) {
    int iResult = -1;

    if (iPolyType != POLY_TYPE_NONE) {
        printf("poly type is %d\n", iPolyType);
        m_iPolyType = iPolyType;
        m_iNumMainFaces = s_aiFaceNum[m_iPolyType];
        m_iNumMainVerts = s_aiVertNum[m_iPolyType];
        m_apMainFaces    = new IcoFace*[m_iNumMainFaces];
        m_apMainVertices = new Vec3D*[m_iNumMainVerts];
        iResult = 0;

        switch (m_iPolyType) {
        case POLY_TYPE_ICO:
            iResult = initIco();
            break;
        case POLY_TYPE_OCT:
            iResult = initOct();
            break;
        case POLY_TYPE_TET:
            iResult = initTet();
            break;
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// init
//  create the vertices and triangles of a icosahedron
//
int Icosahedron::initIco() {
    
    // calculate sine and cosine of latitude at which 
    // the lowest vertices of a top triangle lie
    double dS = calcSinAngle();
    double dC = sqrt(1-dS*dS);
    // calculate vertexes
    // top and bottom
    m_apMainVertices[0] = new Vec3D(0,0,1);
    m_apMainVertices[11] = new Vec3D(0,0,-1);

    double dPhi = 0;
    // first row
    double dDPhi = 2*M_PI/5;
    for (int i = 0; i < 5; i++) {
        m_apMainVertices[i+1]=new Vec3D(dC*cos(dPhi), dC*sin(dPhi), dS);
        dPhi += dDPhi;
    }
    // second row
    dPhi = dDPhi/2;
    for (int i = 0; i < 5; i++) {
        m_apMainVertices[i+6]=new Vec3D(dC*cos(dPhi), dC*sin(dPhi), -dS);
        dPhi += dDPhi;
    }

    // create & set faces
    // top
    m_apMainFaces[0] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[1], m_apMainVertices[2]);
    m_apMainFaces[1] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[2], m_apMainVertices[3]);
    m_apMainFaces[2] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[3], m_apMainVertices[4]);
    m_apMainFaces[3] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[4], m_apMainVertices[5]);
    m_apMainFaces[4] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[5], m_apMainVertices[1]);
    // 2nd row (downpointing)
    m_apMainFaces[5] = IcoFace::createFace(m_apMainVertices[1], m_apMainVertices[6],  m_apMainVertices[2]);
    m_apMainFaces[6] = IcoFace::createFace(m_apMainVertices[2], m_apMainVertices[7],  m_apMainVertices[3]);
    m_apMainFaces[7] = IcoFace::createFace(m_apMainVertices[3], m_apMainVertices[8],  m_apMainVertices[4]);
    m_apMainFaces[8] = IcoFace::createFace(m_apMainVertices[4], m_apMainVertices[9],  m_apMainVertices[5]);
    m_apMainFaces[9] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[10], m_apMainVertices[1]);
    // 3rd row (uppointing)
    m_apMainFaces[10] = IcoFace::createFace(m_apMainVertices[1], m_apMainVertices[10], m_apMainVertices[6]);
    m_apMainFaces[11] = IcoFace::createFace(m_apMainVertices[2], m_apMainVertices[6],  m_apMainVertices[7]);
    m_apMainFaces[12] = IcoFace::createFace(m_apMainVertices[3], m_apMainVertices[7],  m_apMainVertices[8]);
    m_apMainFaces[13] = IcoFace::createFace(m_apMainVertices[4], m_apMainVertices[8],  m_apMainVertices[9]);
    m_apMainFaces[14] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[9],  m_apMainVertices[10]);
    // bottom
    m_apMainFaces[15] = IcoFace::createFace(m_apMainVertices[11], m_apMainVertices[7],  m_apMainVertices[6]);
    m_apMainFaces[16] = IcoFace::createFace(m_apMainVertices[11], m_apMainVertices[8],  m_apMainVertices[7]);
    m_apMainFaces[17] = IcoFace::createFace(m_apMainVertices[11], m_apMainVertices[9],  m_apMainVertices[8]);
    m_apMainFaces[18] = IcoFace::createFace(m_apMainVertices[11], m_apMainVertices[10], m_apMainVertices[9]);
    m_apMainFaces[19] = IcoFace::createFace(m_apMainVertices[11], m_apMainVertices[6],  m_apMainVertices[10]);
    
    return 0;
}


//-----------------------------------------------------------------------------
// initOct
//  create the vertices and triangles of an octahedron
//
int Icosahedron::initOct() {
    
    // calculate vertexes
    // top and bottom
    m_apMainVertices[0] = new Vec3D( 0, 0, 1);
    m_apMainVertices[1] = new Vec3D( 1, 0, 0);
    m_apMainVertices[2] = new Vec3D( 0, 1, 0);
    m_apMainVertices[3] = new Vec3D(-1, 0, 0);
    m_apMainVertices[4] = new Vec3D( 0,-1, 0);
    m_apMainVertices[5] = new Vec3D( 0, 0,-1);


    // create & set faces
    // top
    m_apMainFaces[0] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[1], m_apMainVertices[2]);
    m_apMainFaces[1] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[2], m_apMainVertices[3]);
    m_apMainFaces[2] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[3], m_apMainVertices[4]);
    m_apMainFaces[3] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[4], m_apMainVertices[1]);
    // bottom
    m_apMainFaces[4] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[2], m_apMainVertices[1]);
    m_apMainFaces[5] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[3], m_apMainVertices[2]);
    m_apMainFaces[6] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[4], m_apMainVertices[3]);
    m_apMainFaces[7] = IcoFace::createFace(m_apMainVertices[5], m_apMainVertices[1], m_apMainVertices[4]);
    
    return 0;
}

//-----------------------------------------------------------------------------
// initTet
//  create the vertices and triangles of an tetrahedron
//
int Icosahedron::initTet() {
    //    (±1, 0, -1/sqrt(2))
    //    (0, ±1, 1/sqrt(2)) 
    // calculate sine and cosine of latitude at which 
    // the lowest vertices of a top triangle lie
    double dS = 1.0/sqrt(2);
    // calculate vertexes
    // top and bottom
    m_apMainVertices[0] = new Vec3D( 1, 0, -dS);
    m_apMainVertices[1] = new Vec3D(-1, 0, -dS);
    m_apMainVertices[2] = new Vec3D( 0, 1,  dS);
    m_apMainVertices[3] = new Vec3D( 0,-1,  dS);


    // create & set faces
    // top
    m_apMainFaces[0] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[1], m_apMainVertices[2]);
    m_apMainFaces[1] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[2], m_apMainVertices[3]);
    m_apMainFaces[2] = IcoFace::createFace(m_apMainVertices[0], m_apMainVertices[3], m_apMainVertices[1]);
    m_apMainFaces[3] = IcoFace::createFace(m_apMainVertices[1], m_apMainVertices[3], m_apMainVertices[2]);
    
    return 0;
}



//-----------------------------------------------------------------------------
// merge
//   merge all subfaces up to given level 
//
void Icosahedron::merge(int iNumLevels) {
    m_iSubLevel = iNumLevels;
    for (int i = 0; i < m_iNumMainFaces; i++) {
        m_apMainFaces[i]->merge(iNumLevels);
    }
    delete m_pVL;
    m_pVL = NULL;

    if (iNumLevels == 0) {
        setBox(-PIPLUS,PIPLUS,-PIPLUS/2, PIPLUS/2);
    }
}

//-----------------------------------------------------------------------------
// subdivideLand
//  subdivide all faces down to given level
//
void Icosahedron::subdivideLand(ValReader *pVR, float fMinAlt, int iNumLevels) {
    m_iSubLevel = iNumLevels;
    for (int i = 0; i < m_iNumMainFaces; i++) {
        m_apMainFaces[i]->subdivideLand(pVR, fMinAlt, iNumLevels);
    }
}


//-----------------------------------------------------------------------------
// subdivide
//  subdivide all faces down to given level
//
void Icosahedron::subdivide(int iNumLevels) {
    m_iSubLevel = iNumLevels;
    for (int i = 0; i < m_iNumMainFaces; i++) {
        m_apMainFaces[i]->subdivide(iNumLevels);
    }
}

//-----------------------------------------------------------------------------
// relinkR
//  recursive relinking of vertex connections
//
void Icosahedron::relinkR(IcoFace *pF) {
    if (pF->isSubdivided()) {
        IcoFace *pFF = pF->getFirstSubFace();
        // loop through all subfaces of current main face
        while (pFF != NULL) {
            relinkR(pFF);
            pFF = pFF->getNextSubFace();
        }
    } else {
        if (!m_bPreSel || (pF->getLevel() == m_iSubLevel)) {
            for (int i = 0; i < 3; i++) {
                double dLon;
                double dLat;
                cart2Sphere(pF->getVertex(i), &dLon, &dLat);
            }

            if (!m_bStrict || (pF->inBox(m_curBox))) {

                m_vFaceList.push_back(pF);
                //m_pVL->addFace(pF);
                (m_pVL->*m_fFaceAddVL)(pF);
            }

            /*
            printf("at level %d/%d: |", pF->getLevel(), m_iSubLevel); 
            for (int i = 0; i < 3; i++) {
                printf("(%f %f %f)|", pF->getVertex(i)->m_fX, pF->getVertex(i)->m_fY, pF->getVertex(i)->m_fZ);
            }
            printf("\n")
            */
                    } 
    }
}


//-----------------------------------------------------------------------------
// relink
//  reestablish the Vertex connections
//  start for the recursive relinking
//
void Icosahedron::relink() {
    printf("%p: relinking%s\n", this, "(normal)");
    double dTot1=0;
#ifdef OMP
    dTot1 = omp_get_wtime();
#endif

    m_fFaceAddVL = &VertexLinkage::addFace;


    m_vFaceList.clear();

    if (m_pVL != NULL) {
        delete m_pVL;
    }
    m_pVL = new VertexLinkage();
    
    for (int i = 0; i < m_iNumMainFaces; i++) {
        relinkR(m_apMainFaces[i]);
    }


#ifdef OMP
    double dTot2 = omp_get_wtime();
    printf("relinking (%d faces, %lld nodes) used %fs\n", m_pVL->getNumFaces(),m_pVL->getNumVertices(), dTot2-dTot1);
#endif
    
}


//-----------------------------------------------------------------------------
// facedisplay
//   display a face and its' subfaces (recursive)
//
void Icosahedron::facedisplay(IcoFace *pF, const char *pIndent, bool bAll) {
    if (pF != NULL) {
        
        if (bAll || !pF->isSubdivided()) {
            /*
            printf("%s(%04lld,%04lld,%04lld)\n", pIndent, 
                   m_pVL->getVertexID(pF->getVertex(0)),
                   m_pVL->getVertexID(pF->getVertex(1)),
                   m_pVL->getVertexID(pF->getVertex(2)));
            */
            printf("%s(%04d,%04d,%04d)\n", pIndent, 
                   pF->getVertexID(0),
                   pF->getVertexID(1),
                   pF->getVertexID(2));
        }
        if (pF->isSubdivided()) {
            IcoFace *pFS = pF->getFirstSubFace();
            char s[256];
            sprintf(s, "  %s", pIndent);

            while (pFS != NULL) {
                facedisplay(pFS, s, bAll);
                pFS = pFS->getNextSubFace();
            } 
        }
    }
}

//-----------------------------------------------------------------------------
// display
//  display the icosahedrons faces
//
void Icosahedron::display() {
    if (m_pVL != NULL) {
        for (int i = 0; i < m_iNumMainFaces; i++) {
            printf("*---%02d--\n", i);
            facedisplay(m_apMainFaces[i], "  ", true);
        }
    } else {
        printf("have no linkage\n");
    }
}

//-----------------------------------------------------------------------------
// getFirstFace
//  get first face from face list
//
PolyFace *Icosahedron::getFirstFace() {
    m_iCurFace = 0;
    return getNextFace();
}


//-----------------------------------------------------------------------------
// getNextFace
//  get next face from face list
//
PolyFace *Icosahedron::getNextFace() {
    IcoFace *pF = NULL;
    if (m_iCurFace < m_vFaceList.size()) {
        pF = m_vFaceList[m_iCurFace++];
    }

    return pF;
}




//-----------------------------------------------------------------------------
// variableSubDivOMP
//    openMP version, avoiding usage of tfacelist
//
void Icosahedron::variableSubDivOMP(int iMaxLevel,  tbox &tBox, double  dDLon, double dDLat) {
    m_curBox = tBox;

    printf("Starting variableSubDivOMPB\n");
    m_iSubLevel = iMaxLevel;
#ifdef OMP
    double dTot1 = omp_get_wtime();
#endif

    int iFlatCount = m_iNumMainFaces;
    int iFlatSize  = 4*iFlatCount;
    IcoFace** apFlattened = new IcoFace*[iFlatSize];
    for (int i = 0; i < m_iNumMainFaces; i++) {
        apFlattened[i] = m_apMainFaces[i];
    }
    int iNum = 0;
    // only an experiment:            omp_set_num_threads(1);

#ifdef OMP
#pragma omp parallel default(none)              \
    shared(iNum)
        {
            if (omp_get_thread_num() == 0) {
                iNum = omp_get_num_threads();
            }
        } 
    printf("%d processes at work\n", iNum);
#else
    iNum=1;
#endif 

    
    // find level at which triangles are smaller than smallest side of box
    Vec3D v0(cos(m_curBox.dLonMin)*cos(m_curBox.dLatMin),sin(m_curBox.dLonMin)*cos(m_curBox.dLatMin),sin(m_curBox.dLatMin));
    Vec3D v1(cos(m_curBox.dLonMin)*cos(m_curBox.dLatMax),sin(m_curBox.dLonMin)*cos(m_curBox.dLatMax),sin(m_curBox.dLatMax));
    Vec3D v2(cos(m_curBox.dLonMax)*cos(m_curBox.dLatMin),sin(m_curBox.dLonMax)*cos(m_curBox.dLatMin),sin(m_curBox.dLatMin));
    Vec3D v3(cos(m_curBox.dLonMax)*cos(m_curBox.dLatMax),sin(m_curBox.dLonMax)*cos(m_curBox.dLatMax),sin(m_curBox.dLatMax));
    double d1 = v0.dist(&v1);
    double d2 = v0.dist(&v2);
    double d3 = v2.dist(&v3);

    double dMinSide = (d1 < d2)?d1:d2;
    dMinSide = (dMinSide < d3)?dMinSide:d3;
    double dBigSide = m_apMainFaces[0]->getVertex(0)->dist(m_apMainFaces[0]->getVertex(1));
    int iMaxLevel1 = (int)(-log(dMinSide/dBigSide)/log(2))+iMaxLevel/2;
    printf("Minside %f, bigside %f -> levs %d\n",  dMinSide, dBigSide, iMaxLevel1);
    //do this for "old" functionality:    iMaxLevel1 = iMaxLevel;
    //    if (iMaxLevel > 3) {
    //    iMaxLevel1 = iMaxLevel-1;
    //}

    int *aiFC = new int[iNum];
    int *aiCaps = new int[iNum];
    IcoFace ***apFaces = new IcoFace**[iNum];

    int iLevel = 0;
    // 1st phase: triangleas are larger than box
    while (iLevel < iMaxLevel1) {
        
        int k = iFlatCount;
        IcoFace **apOldFaces = new IcoFace*[iFlatCount];
        memcpy(apOldFaces, apFlattened, iFlatCount*sizeof(IcoFace*));
     

        int iCap = 64*iFlatCount;
        for (int i =0; i < iNum; i++) {
            apFaces[i] = new IcoFace*[iCap];
            aiFC[i] = 0;
            aiCaps[i] = iCap;
        }
#ifdef OMP
        double dt1 = omp_get_wtime();
#endif
        int chunk = (int)(((m_curBox.dLatMax-m_curBox.dLatMin)/dDLat)/iNum);

        printf("Doing level %d/%d on %d faces (chunk:%d)\n", iLevel, iMaxLevel, iFlatCount, chunk);

        int iMax = (int)(1+((m_curBox.dLatMax-m_curBox.dLatMin)/dDLat));
        int iFacesLeft = k;
#pragma omp parallel default(none)              \
    shared(chunk, dDLat, dDLon, apFaces, aiCaps, aiFC, apOldFaces, iMax, k, iFacesLeft)
        {
#pragma omp for schedule(static, chunk)
            for (int i = 0; (i < iMax); i++) {
#ifdef OMP                                
            int iCur = omp_get_thread_num();
#else
            int iCur = 0;
#endif

            double dLat = m_curBox.dLatMin + i*dDLat;
            double dCLat = cos(dLat);
            double dSLat = sin(dLat);
            for (double dLon = m_curBox.dLonMin; (iFacesLeft > 0) && (dLon <  m_curBox.dLonMax+dDLon); dLon += dDLon) {
                Vec3D v(dCLat*cos(dLon), dCLat*sin(dLon), dSLat);
                //                printf("Checking (%f,%f)->(%f,%f,%f)\n", dLon, dLat, v.m_fX, v.m_fY, v.m_fZ);

                for (int i = 0; i < k; i++) {
                    IcoFace *pF = apOldFaces[i];
                    
                    if (pF != NULL) {
                        if (pF->contains(&v)) {
                            //  printf("proc %d faces[%d/%d] adding %p\n", iCur, aiFC[iCur], aiCaps[iCur], pF);
                            apFaces[iCur][aiFC[iCur]++] = pF;
                            apOldFaces[i] = NULL;
                            --iFacesLeft;
                            if (iFacesLeft == 0) {
                                // force break of outer loop
                                i = iMax;
                            }
                            if (aiFC[iCur] >= aiCaps[iCur]) {
                                aiCaps[iCur] *= 64;
                                //    printf("resize %d to %d\n",iCur, aiCaps[iCur]);
                                IcoFace **ppF = new IcoFace*[aiCaps[iCur]];
                                memcpy(ppF, apFaces[iCur], aiFC[iCur]*sizeof(IcoFace*));
                                delete[] apFaces[iCur];
                                apFaces[iCur] = ppF;
                            }
                        }
                    }
                }
            }
        }
        }

        int iTotal = 0;
        for (int i = 0; i < iNum; i++) {
            iTotal +=  aiFC[i];
        }
        iTotal *= 4;
   
        if (iTotal > iFlatSize) {
            delete[] apFlattened;
            apFlattened = new IcoFace*[64*iTotal];
            printf("Resized to %d items (%zd bytes)\n", iTotal, iTotal*sizeof(IcoFace*));
            iFlatSize = 64*iTotal;
            bzero(apFlattened, iFlatSize*sizeof(IcoFace*));
        }
        iFlatCount = iTotal;
        int iC = 0;
        //        IcoFace **pCur = apFlattened;
        for (int i = 0; i < iNum; i++) {
            for (int j = 0; j < aiFC[i]; j++) {
                apFaces[i][j]->subdivide(1);
                IcoFace *pFS = apFaces[i][j]->getFirstSubFace();
                while (pFS != NULL) {
                    apFlattened[iC++] = pFS;
                    pFS = pFS->getNextSubFace();
                }
            }
        }
        if (iC != iFlatCount) {
            printf("copied %d items iinstead of %d\n", iC, iFlatCount);
        }
#ifdef OMP
        double dt2 = omp_get_wtime();
        printf("involved faces: %d (%f secs)\n", iFlatCount,  dt2-dt1);
#endif
        for (int i =0; i < iNum; i++) {
            delete[] apFaces[i];
        }

        delete[] apOldFaces;
        
        ++iLevel;
        
    }


    // now second phase (triangles are smaller than box
    printf("*** switching to phase 2 ***\n");
    
      while (iLevel < iMaxLevel) {
               int k = iFlatCount;
        IcoFace **apOldFaces = new IcoFace*[iFlatCount];
        memcpy(apOldFaces, apFlattened, iFlatCount*sizeof(IcoFace*));
     
        int chunk = k/(4*iNum);
        if (chunk == 0) {
             chunk = 1;
        }

        printf("phase2: Doing level %d/%d on %d faces (chunk:%d)\n", iLevel, iMaxLevel, iFlatCount, chunk);

        int iCap = 64*iFlatCount;
        for (int i =0; i < iNum; i++) {
            apFaces[i] = new IcoFace*[iCap];
            aiFC[i] = 0;
            aiCaps[i] = iCap;
        }
#ifdef OMP
        double dt1 = omp_get_wtime();
#endif

        int iFacesLeft = k;
       
#pragma omp parallel default(none)              \
    shared(chunk, apFaces, aiCaps, aiFC, apOldFaces, k, iFacesLeft)
        {
#pragma omp for schedule(static, chunk)
            for (int i = 0; i < k; i++) {
            IcoFace *pF = apOldFaces[i];
#ifdef OMP                                
            int iCur = omp_get_thread_num();
#else
            int iCur = 0;
#endif
            
            if (pF != NULL) {
                if (pF->vertexInBox(m_curBox)) {
                    //  printf("proc %d faces[%d/%d] adding %p\n", iCur, aiFC[iCur], aiCaps[iCur], pF);
                    apFaces[iCur][aiFC[iCur]++] = pF;
                    apOldFaces[i] = NULL;
                    --iFacesLeft;
                    if (iFacesLeft == 0) {
                        // force break of outer loop
                        i = k;
                    }
                    if (aiFC[iCur] >= aiCaps[iCur]) {
                        aiCaps[iCur] *= 64;
                        //    printf("resize %d to %d\n",iCur, aiCaps[iCur]);
                        IcoFace **ppF = new IcoFace*[aiCaps[iCur]];
                        memcpy(ppF, apFaces[iCur], aiFC[iCur]*sizeof(IcoFace*));
                        delete[] apFaces[iCur];
                        apFaces[iCur] = ppF;
                    }
                }
            }
        }
        }
#pragma omp parallel default(none)              \
    shared(iNum, apFaces, aiCaps, aiFC, apOldFaces, k, iFacesLeft, iMaxLevel, iLevel)
        {
#pragma omp for schedule(static, 1)
        for (int i = 0; i < iNum; i++) {
            for (int j = 0; j < aiFC[i]; j++) {
                if (apFaces[i][j]->inBox(m_curBox)) {
                    apFaces[i][j]->subdivide(iMaxLevel-iLevel);
                    apFaces[i][j]->setCompletionLevels(iMaxLevel-iLevel);
                    apFaces[i][j]=NULL;
                }
             }
        }
        }

        int iTotal = 0;
        for (int i = 0; i < iNum; i++) {
            iTotal +=  aiFC[i];
        }
        iTotal *= 4;
   
        if (iTotal > iFlatSize) {
            delete[] apFlattened;
            apFlattened = new IcoFace*[64*iTotal];
            printf("Resized to %d items (%zd bytes)\n", iTotal, iTotal*sizeof(IcoFace*));
            iFlatSize = 64*iTotal;
        }
        iFlatCount = iTotal;
        int iC = 0;
        //        IcoFace **pCur = apFlattened;
        for (int i = 0; i < iNum; i++) {
            for (int j = 0; j < aiFC[i]; j++) {
                if (apFaces[i][j] != NULL) {
                    apFaces[i][j]->subdivide(1);
                    IcoFace *pFS = apFaces[i][j]->getFirstSubFace();
                    while (pFS != NULL) {
                        apFlattened[iC++] = pFS;
                        pFS = pFS->getNextSubFace();
                    }
                }
            }
        }
        if (iC != iFlatCount) {
            printf("copied %d items iinstead of %d\n", iC, iFlatCount);
            iFlatCount = iC;
        }
#ifdef OMP
        double dt2 = omp_get_wtime();
        printf("involved faces: %d (%f secs)\n", iFlatCount,  dt2-dt1);
#endif
        for (int i =0; i < iNum; i++) {
            delete[] apFaces[i];
        }

        delete[] apOldFaces;
        
        ++iLevel;
        
}




    delete[] apFaces;
    delete[] aiCaps;
    delete[] aiFC;
    delete[] apFlattened;


#ifdef OMP
    double dTot2 = omp_get_wtime();
    printf("total time needed: %f secs\n",  dTot2-dTot1);
#endif

}













//-----------------------------------------------------------------------------
// save
//  save icosahedron in a declarative way:
//  ICO     ::= FACE*
//  FACE    ::= "!"<Num2>":"STRUCT
//  STRUCT  ::= "(" <complev> ")" | "()" | DIV
//  DIV     ::= "(" <STRUCT><STRUCT><STRUCT><STRUCT> ")"
//
int Icosahedron::save(const char *pFile) {
    int iResult = -1;
    BufWriter *pBW =  BufWriter::createInstance(pFile, ICO_BUF);
    if (pBW != NULL) {
        // header object
        IcoHeader *pIH = new IcoHeader(m_iPolyType, m_iSubLevel, m_curBox);
        iResult = pIH->write(pBW);

        for (int i = 0; (i < m_iNumMainFaces) && (iResult == 0); i++) {
            printf("s---%02d--\n", i);
            char sStart[5];
            m_apMainFaces[i]->calcCompletion();
            sprintf(sStart, "!%02d:", i);
            iResult = pBW->addChars(sStart, 4);
            if (iResult == 0) {
                iResult = writeFace(pBW, m_apMainFaces[i]);
            }
        }
        delete pIH;
        delete pBW;
    } else {
        iResult = -1;
        printf("[Icosahedron::save] couldn't open [%s] for reading\n", pFile);
    }
    return iResult;
}

char sBrack[]="(.)";
//-----------------------------------------------------------------------------
// writeFace
//
int Icosahedron::writeFace(BufWriter *pBW, IcoFace *pF) {
    int iResult = 0;
    
    if (pF->isSubdivided()) {
        // open bracket
        //        pBW->addChars(sBrack, 1);
        pBW->addChar(OBR);

        int iCompletion = pF->getCompletionLevels();
        if ((iCompletion > 1) && (iCompletion < 128)) {
            char c = (char)(0x80 + iCompletion);
            pBW->addChar(c);

        } else {
        
            IcoFace *pFS = pF->getFirstSubFace();
            bool bLeaf = true;
            while (bLeaf && (pFS != NULL)) {
                bLeaf = !pFS->isSubdivided();
                pFS = pFS->getNextSubFace(); 
            }
            if (bLeaf) {
                // do nothing
            } else {
                // write subfaces
                pFS = pF->getFirstSubFace();
                while ((iResult == 0) && (pFS != NULL)) {
                    iResult =writeFace(pBW, pFS);
                    pFS = pFS->getNextSubFace(); 
                } 
            }
        }
        // close bracket
        //        pBW->addChars(sBrack+2, 1);
        pBW->addChar(CBR);
    } else {
        // just a dot
        pBW->addChar(DOT);
        //        pBW->addChars(sBrack+1, 1);
    }
    return iResult;
} 



//-----------------------------------------------------------------------------
// parseSubFace
//  look at stream of symbols and perform appropriate subdivisions of the
//  current face (recursive!)
//    LEV: "(<num>)" do multiple subdivisione (number from psm->m_iCurNum)
//    CBR: "()"      do final simple subdivision
//    other symbols (OBR, DOT) refer to subfaces
//
int Icosahedron::parseSubFace(symbuf *psm, IcoFace *pF) {
    int iResult = 0;
    pF->subdivide(1);
    int iSym = psm->getNextSym();

    // first check for comp
    if (iSym == SYM_LEV) {
        // should subdivide F1 multiple times
        pF->subdivide(psm->getCurNum());
        pF->setCompletionLevels(psm->getCurNum());
        iSym = psm->getNextSym();
    } else if (iSym == SYM_CBR) {
        // final simple subdivision
        pF->subdivide(1);
        // do not get next symbol!!!
    } else {  
        IcoFace *pF1 = pF->getFirstSubFace();
        // a "subface" starts with a with an opening bracket, or is simply a dot
        while ((iResult == 0) && (pF1 != NULL) && (iSym != SYM_CBR)) {
            if (iSym == SYM_OBR) {
                iResult = parseSubFace(psm, pF1);
            } else if (iSym == SYM_DOT) {
                // do nothing with face
            } else {
                printf("Unexpected symbol: [%s](%d)\n", psm->getSymName(iSym), iSym);
                iResult = -1;
            }
            if (iResult == 0) {
                iSym = psm->getNextSym();
                pF1 = pF1->getNextSubFace();
            }
        }
    }
    // it must end with a closing bracket
    if (iSym != SYM_CBR) {
        iResult = -1;
        printf("Expected CBR instead of [%s]\n",   psm->getSymName(iSym));
    }
    return iResult;
        
}

//-----------------------------------------------------------------------------
//  getStartPoints
//  find points in file where a new face starts
//
int getStartPoints(const char *pFile, int *aiLocs) {
    ulong iCurStart = 0;
    int iNumLocs = 0;
    char sData[8192];

    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        while (!feof(fIn)) {
            ulong iRead = fread(sData, 1, 8192, fIn);
            char *p = sData;
            for (uint i = 0; i < iRead; i++) {
                if (*p == '!') {
                    aiLocs[iNumLocs++] = (int)(iCurStart+i);
                }
                ++p;
            }
            iCurStart += iRead;
        }        
        fseek(fIn, 0, SEEK_END);
        aiLocs[iNumLocs++] = (int)ftell(fIn);
        fclose(fIn);
    } else {
        printf("Couldn't open [%s]\n", pFile);
    }
    return iNumLocs;
}

  
//-----------------------------------------------------------------------------
// load
//  parse data from given file and perform appropriate subdiivisions
//    ICO     ::= FACE*
//    FACE    ::= "!"<Num2>":"STRUCT
//    STRUCT  ::= "(" <complev> ")" | "()" | DIV
//    DIV     ::= "(" <STRUCT><STRUCT><STRUCT><STRUCT> ")"
//
int Icosahedron::load(const char *pFile) {
    int iResult = 0;
    double dTot1=0;

    /*
    double dTot0a=0;
    double dTot0b=0;

#ifdef OMP
    dTot0a = omp_get_wtime();
#endif
    
    int aiLocs[21];
    int iNumLocs = getStartPoints(pFile, aiLocs);
#ifdef OMP
    dTot0b = omp_get_wtime();
#endif

    printf("finding %d starts used %fs\n", iNumLocs, dTot0b-dTot0a);
    for (int i =0; i < 20; i++) {
        printf("  %d (%d)\n", aiLocs[i], aiLocs[i+1]-aiLocs[i]);
    }
    */


#ifdef OMP
    dTot1 = omp_get_wtime();
#endif

    printf("ico[%p] Loading [%s]\n", this, pFile);
    BufReader *pBR = BufReader::createInstance(pFile, ICO_BUF);
    if (pBR != NULL) {
        printf("Doing header (%s)...\n", m_bStrict?"strict":"free");
        IcoHeader *pIH = new IcoHeader();
        iResult = pIH->read(pBR);
        // do something with the data?
        m_iSubLevel = pIH->getSubLevel();
        int iPolyType = pIH->getPolyType();
        if (m_bStrict) {
            pIH->getBox(m_curBox);
        }
        delete pIH;
        if (iPolyType != m_iPolyType) {
            for (int i = 0; i < m_iNumMainFaces; i++) {
                delete m_apMainFaces[i];
            }
            delete[] m_apMainFaces;
            for (int i = 0; i < m_iNumMainVerts; i++) {
                delete m_apMainVertices[i];
            }
            delete[] m_apMainVertices;

            init(iPolyType);
        }
        m_iPolyType = iPolyType;
        IcoFace *pF = NULL;
        symbuf *psm = new symbuf(pBR);
        int iSym = psm->getNextSym();
        //        printf("ld:New symbol: [%s](%d)\n", psm->getSymName(iSym), iSym);
        while ((iResult == 0) &&(iSym == SYM_NUM)) {
            int iNum = psm->getCurNum();printf("   N=%02d\n", iNum);
            if ((iNum >= 0) && (iNum < m_iNumMainFaces)) {
                pF = m_apMainFaces[iNum];
                iSym = psm->getNextSym();
                switch(iSym) {
                case SYM_DOT:
                    // nothing to do
                    // this shouldn't happen: no DOT right after a number
                    iSym = psm->getNextSym();
                    break;
                case SYM_OBR:
                    parseSubFace(psm, pF); // subdiv and select first
                    iSym = psm->getNextSym();
                    break;
                case SYM_NUL:
                    break;
                case SYM_ERR:
                case SYM_CBR:
                case SYM_NUM:
                    // these should not occur here: errors
                    break;
                }
            } else {
                printf("invalid face number:%d\n", iNum);
                iResult = -1;
            }
        }

        if (iSym == SYM_NUL) {
            printf("Icosahedron loaded\n");
            iResult = 0;
        } else {
            printf("[Exit]Unexpected symbol: [%s](%d)\n",  psm->getSymName(iSym), iSym);
            iResult = -1;
        }
        

        delete psm;
        delete pBR;
#ifdef OMP
    double dTot2 = omp_get_wtime();
    printf("loading  used %fs\n", dTot2-dTot1);
#endif
    } else {
        iResult =-1;
        printf("Couldn't open [%s] for reading\n", pFile);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// findNode
//  IcoLoc implementation
//  fin id of node closest to coords
//  
gridtype Icosahedron::findNode(Vec3D *pv) {
    gridtype iID = -1;
    
    PolyFace *pF0 = NULL;
    for (int i =0; (pF0 == NULL) && (i < m_iNumMainFaces); i++) {
        PolyFace *pF = m_apMainFaces[i]->contains(pv);
        if (pF != NULL) {
            pF0 = pF;
        }
    }
    if (pF0 != NULL) {
        iID = pF0->closestVertexID(pv);
    }
    return iID;
}

//-----------------------------------------------------------------------------
// findNode
//  IcoLoc implementation
//  fin id of node closest to coords
//  
gridtype Icosahedron::findNode(double dLon, double dLat) {
    dLat = dLat * M_PI / 180.0;
    dLon = dLon * M_PI / 180.0;
    double c1=cos(dLat);
    Vec3D v(c1*cos(dLon), c1*sin(dLon), sin(dLat));
    
    return findNode(&v);
}

//-----------------------------------------------------------------------------
// findCoords
//   IcoLoc implementation
//   find longitude and latitude of node with given ID
//
bool Icosahedron::findCoords(int iNodeID, double *pdLon, double *pdLat) {
    bool bOK = false;
    Vec3D *pV = m_pVL->getVertex(iNodeID);
    if (pV != NULL) {
        cart2Sphere(pV, pdLon, pdLat);
        bOK = true;
    }
    return bOK;
}


//-----------------------------------------------------------------------------
// findFace
//  IcoLoc implementation
//  fin id of node closest to coords
//  
PolyFace *Icosahedron::findFace(double dLon, double dLat) {
    double c1=cos(dLat);
    Vec3D v(c1*cos(dLon), c1*sin(dLon), sin(dLat));
    
    PolyFace *pF0 = NULL;
    for (int i =0; (pF0 == NULL) && (i < m_iNumMainFaces); i++) {
        PolyFace *pF = m_apMainFaces[i]->contains(&v);
        if (pF != NULL) {
            pF0 = pF;
        }
    }
    return pF0;
}

