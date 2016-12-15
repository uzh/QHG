#include <stdlib.h>
#include <vector>
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapUtils.h"
#include "QMapReader.cpp"
#include "QGradientFinder.h"

/**************************************************************************
   QGradientFinder
   a tool to find a flow line in a scalarfield defined by a QMAP
\*************************************************************************/

// single neighborhood directions
int aaiNeigh[8][2] = {
    { 1, 0},
    { 1, 1},
    { 0, 1},
    {-1, 1},
    {-1, 0},
    {-1,-1},
    { 0,-1},
    { 1,-1},
};


// 2-Neighboirhood directions
int aaiNeigh2[16][2] = {
    { 2, 0},
    { 2, 1},
    { 2, 2},
    { 1, 2},
    { 0, 2},
    {-1, 2},
    {-2, 2},
    {-2, 1},
    {-2, 0},
    {-2,-1},
    {-2,-2},
    {-1,-2},
    { 0,-2},
    { 1,-2},
    { 2,-2},
    { 2,-1},
};

#define NUM_NEIGH  (sizeof(aaiNeigh)/(2*sizeof(int)))
#define NUM_NEIGH2 (sizeof(aaiNeigh2)/(2*sizeof(int)))

#define EPS 0.0001

//-----------------------------------------------------------------------------
// constructor
//
QGradientFinder::QGradientFinder()
    : m_pVR(NULL),
      m_iW(-1),
      m_iH(-1),
      m_bVerbose(false) {
}

//-----------------------------------------------------------------------------
// destructor
//
QGradientFinder::~QGradientFinder() {
    if (m_pVR != NULL) {
        delete m_pVR;
    }
}


//-----------------------------------------------------------------------------
// createQGradientFinder
//
QGradientFinder *QGradientFinder::createQGradientFinder(char *pQMAPFile) {
    QGradientFinder *pG = new QGradientFinder();
    int iResult = pG->loadQMap(pQMAPFile);
    if (iResult != 0) {
        delete pG;
        pG  = NULL;
    }
    return pG;
}

//-----------------------------------------------------------------------------
// loadQMap
//
int QGradientFinder::loadQMap(char *pQMAPFile) {
    int iResult = -1;
    m_pVR = QMapUtils::createValReader(pQMAPFile, false);
    if (m_pVR != NULL) {
        m_iW = m_pVR->getNRLon();
        m_iH = m_pVR->getNRLat();
        iResult = 0;
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// findGradient
//   find direction leading to lowest value
//   iX,iY  : current location
//   vDirs  : indices of possible directions (i.e. not into nan.Areas, non-selfintersecting)
//   dMag   : (out) value difference
//
int QGradientFinder::findGradient(unsigned int iX, unsigned int iY, std::vector<int> &vDirs, double &dMag) {
    int iDir = -1;
    int iDir2 = -1;

    dMag = dPosInf;
    double dTemp = dNegInf;

    double dValCur = m_pVR->getDValue(iX, iY);
    if (m_bVerbose) {
        printf("Values around(%d,%d) [%f]\n", iX, iY, m_pVR->getDValue(iX, iY));
    }
    // check all directions
    for (unsigned int k = 0; k < vDirs.size(); k++) {
        int i = vDirs[k];

        unsigned int iX1;
        int iXT = (int)iX + aaiNeigh[i][0];

        // wrap-over
        if (iXT < 0) {
            iX1 = m_iW - 1;
        } else if ((unsigned int)iXT >= m_iW) {
            iX1 = 0;
        } else {
            iX1 = iXT;
        }

        unsigned int iY1;
        int iYT = (int)iY + aaiNeigh[i][1];

        // wrap-over
        if (iYT < 0) {
            iY1 = m_iH - 1;
        } else if ((unsigned int)iYT >= m_iH) {
            iY1 = 0;
        } else {
            iY1 = iYT;
        }

        /*   normalgradient (steep)
        //        printf("  %d (%d,%d):%f\n", i, iX1, iY1, m_pVR->getDValue(iX1, iY1));
        // get difference
        double dG =  m_pVR->getDValue(iX1, iY1) - m_pVR->getDValue(iX, iY);
        if (dG < dMag) {
            dMag = dG;
            iDir = i;
        }

        */
       
        // flat gradient
        double dValNew = m_pVR->getDValue(iX1, iY1);
        if (m_bVerbose) {
            printf("  %d (%d,%d):%f\n", i, iX1, iY1, dValNew);
        }
        if (dValNew < dValCur) {
            if (dValNew > dTemp) {
                dTemp = dValNew;
                iDir = i;
            }
        } else if (dValNew == dValCur) {
            iDir2 = i;
        }
    }

    if (iDir == -1) {
        iDir = iDir2;
    }
    return iDir;
}

//-----------------------------------------------------------------------------
// getPossibleDirsN
//   find all possible directions in the given N-Neighborhood that do not lead into
//   the path itself or into a nan-area
//   iX, iY  : current location
//   vPath   : thr path found so far
//   vDirs   : (out) vector of indices of possible directions
//   returns # of possible directions
// 
//
int QGradientFinder::getPossibleDirsN(unsigned int iX, unsigned int iY, 
                                 int aaiNeighN[][2], unsigned int iNumNeigh,
                                 std::vector<Vec3D> &vPath, std::vector<int> &vDirs) {

    vDirs.clear();
    for (unsigned int i = 0; i < iNumNeigh; i++) {
        int iXs = iX+aaiNeigh[i][0];
        int iYs = iY+aaiNeigh[i][1];
        unsigned int iX1 = iXs;
        unsigned int iY1 = iYs;
        
        if (iXs < 0) {
            iX1 = m_iW-1;
        } else if (iX1 >= m_iW) {
            iX1 = 0;
        }
        if (iYs < 0) {
            iY1 = m_iH-1;
        } else if (iY1 >= m_iH) {
            iY1 = 0;
        }
        bool bSearching = !isnan(m_pVR->getDValue(iX1, iY1));
        for (unsigned int j = 0; bSearching && (j < vPath.size()); j++) {
            if (( vPath[j].m_fX == iX1) && (vPath[j].m_fY == iY1)) {
                bSearching = false;
            }
        }
        if (bSearching) {
            vDirs.push_back(i);
        }
    }
    return vDirs.size();
} 
/*
//-----------------------------------------------------------------------------
// collectPointOld
//   find best gradient in permitted directions and add point to path
//
void QGradientFinder::collectPointOld(unsigned int &iX0, unsigned int &iY0, 
                              std::vector<Vec3D> &vPath, std::vector<int> &vDirs) {
    double dMag;
    
    int iDir = findGradient(iX0, iY0, vDirs, dMag);
   
    if (iDir >= 0) {
        int iXT = iX0 + aaiNeigh[iDir][0];
        if (iXT < 0) {
            iX0 = iXT +m_iW;
        } else if ((unsigned int)iXT >= m_iW) {
            iX0 = iXT-m_iW;
        } else {
            iX0 = iXT;
        }
        
        int iYT = iY0 + aaiNeigh[iDir][1];
        if (iYT < 0) {
            iY0 = iYT +m_iH;
        } else if ((unsigned int) iYT >= m_iH) {
            iY0 = iYT-m_iH;
        } else {
            iY0 = iYT;
        }
        Vec3D A(iX0, iY0, m_pVR->getDValue(iX0, iY0));
        vPath.push_back(A);
    }
}

//-----------------------------------------------------------------------------
// calcFlowOld
//
int QGradientFinder::calcFlowOld(unsigned int iX, unsigned int iY, double dMin, std::vector<Vec3D> &vPath) {
    int iResult = 0;
    unsigned int iX0 = iX;
    unsigned int iY0 = iY;

    unsigned int iXP = -1;
    unsigned int iYP = -1;
    int iC = 0;
    int iDir = 0;
    std::vector<int> vDirs;
    int iNumDirs = NUM_NEIGH;

    Vec3D A(iX0, iY0, m_pVR->getDValue(iX0, iY0));
    vPath.push_back(A);

    while ( (m_pVR->getDValue(iX0, iY0) > dMin) && !close(iX0,iY0, iXP,iYP) && (iNumDirs > 0) && (iC++ < 3000)) {
        iXP = iX0;
        iYP = iY0;
        
        iNumDirs = getPossibleDirsN(iX0, iY0, aaiNeigh, NUM_NEIGH, vPath, vDirs);
        if (iNumDirs <= 0) {
            if (m_bVerbose) {
                printf("Ran out of directions, trying 2\n");
            }
            iNumDirs = getPossibleDirsN(iX0, iY0, aaiNeigh2, NUM_NEIGH2, vPath, vDirs);
            if (iNumDirs <= 0) {
                if (m_bVerbose) {
                    printf("Ran out of directions, done\n");
                }
            }
        }
        if (iNumDirs > 0) {
            collectPoint(iX0, iY0, vPath, vDirs);
        }

    }
    if (m_bVerbose) {
        printf("locval %f, close:%s, numdir %d, iC %d\n", m_pVR->getDValue(iX0, iY0), close(iX0,iY0, iXP,iYP)?"yes":"no", iNumDirs, iC);
    }
    if (iDir < 0) {
        if (m_bVerbose) {
            printf("0 gradient\n");
        }
        iResult = -1;
    }

    return iResult;
}
*/

//-----------------------------------------------------------------------------
// collectPoint
//   find best gradient in permitted directions and add point to path
//
void QGradientFinder::collectPoint(unsigned int &iX0, unsigned int &iY0, 
                              Vec3D &vA, std::vector<int> &vDirs) {
    double dMag;
    
    int iDir = findGradient(iX0, iY0, vDirs, dMag);
   
    if (iDir >= 0) {
        int iXT = iX0 + aaiNeigh[iDir][0];
        if (iXT < 0) {
            iX0 = iXT +m_iW;
        } else if ((unsigned int)iXT >= m_iW) {
            iX0 = iXT-m_iW;
        } else {
            iX0 = iXT;
        }
        
        int iYT = iY0 + aaiNeigh[iDir][1];
        if (iYT < 0) {
            iY0 = iYT +m_iH;
        } else if ((unsigned int) iYT >= m_iH) {
            iY0 = iYT-m_iH;
        } else {
            iY0 = iYT;
        }
        vA.m_fX=iX0;
        vA.m_fY=iY0;
        vA.m_fZ=m_pVR->getDValue(iX0, iY0);
        
    }
}

//-----------------------------------------------------------------------------
// calcFlow
//
int QGradientFinder::calcFlow(unsigned int iX, unsigned int iY, double dMin, std::vector<Vec3D> &vPath) {
    int iResult = 0;
    unsigned int iX0 = iX;
    unsigned int iY0 = iY;

    unsigned int iXP = -1;
    unsigned int iYP = -1;
    int iC = 0;
    int iDir = 0;
    std::vector<int> vDirs;
    int iNumDirs = NUM_NEIGH;

    Vec3D A(iX0, iY0, m_pVR->getDValue(iX0, iY0));
    vPath.push_back(A);

    while ( (m_pVR->getDValue(iX0, iY0) > dMin) && !close(iX0,iY0, iXP,iYP) && (iNumDirs > 0) && (iC++ < 3000)) {
        iXP = iX0;
        iYP = iY0;
        bool bTry2=false;
        Vec3D A1;
        iNumDirs = getPossibleDirsN(iX0, iY0, aaiNeigh, NUM_NEIGH, vPath, vDirs);
        if (iNumDirs <= 0) {
            if (m_bVerbose) {
                printf("Ran out of directions, trying 2\n");
            }
            bTry2=true;
        } else {
            collectPoint(iX0, iY0, A1, vDirs);
            if (A1.m_fZ < dMin) {
                if (m_bVerbose) {
                    printf("Only found sublow val %f, trying 2\n", A1.m_fZ);
                }
                bTry2=true;
            }
        }
        if (bTry2) {
            if (m_bVerbose) {
                printf("Trying larger neighborhood\n");
            }
            iX0 = iXP;
            iY0 = iYP;

            iNumDirs = getPossibleDirsN(iX0, iY0, aaiNeigh2, NUM_NEIGH2, vPath, vDirs);
            if (iNumDirs <= 0) {
                if (m_bVerbose) {
                    printf("Ran out of directions, done\n");
                }
            } else {
                collectPoint(iX0, iY0, A1, vDirs);
            }
        }          

        if (iNumDirs > 0) {
            if (m_bVerbose) {
                printf("Adding (%f,%f,%f)\n", A1.m_fX, A1.m_fY, A1.m_fZ);
            }
            vPath.push_back(A1);
        }

    }
    if (m_bVerbose) {
        printf("locval %f, close:%s, numdir %d, iC %d\n", m_pVR->getDValue(iX0, iY0), close(iX0,iY0, iXP,iYP)?"yes":"no", iNumDirs, iC);
    }
    if (iDir < 0) {
        if (m_bVerbose) {
            printf("0 gradient\n");
        }
        iResult = -1;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// close
//
bool QGradientFinder::close(unsigned int iX1, unsigned int iY1,
                       unsigned int iX2, unsigned int iY2) {
    bool bClose = false;
    if ((iX1 >= 0) && (iY1 >= 0) &&
        (iX2 >= 0) && (iY2 >= 0)) {
        if ((abs(iX1-iX2)<= 0) && (abs(iY1-iY2)<= 0)) {
            bClose = true;
        }
    }
    return bClose;
}
