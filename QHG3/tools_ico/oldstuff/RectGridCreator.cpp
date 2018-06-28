#include <stddef.h>
#include <math.h>

#include "Vec3D.h"

#include "GridProjection.h"

#include "Lattice.h"
#include "IcoFace.h"
#include "QuadFace.h"
#include "VertexLinkage.h"
#include "GridProjection.h"
#include "Region.h"
#include "RegionSplitter.h"
#include "RectGridCreator.h"

#define SQUEEZE (sqrt(3)/2)

class PolarConvGrid : public PolarConv {
public:
    PolarConvGrid(GridProjection *pGP) : m_pGP(pGP) {};
    virtual ~PolarConvGrid() {};
    virtual void conv2Polar(Vec3D *pV, double *pdLon, double *pdLat) {
        m_pGP->gridToSphere(pV->m_fX, pV->m_fY, *pdLon, *pdLat);
    }
private:
    GridProjection *m_pGP;
};

//-----------------------------------------------------------------------------
// createInstance
//
RectGridCreator *RectGridCreator::createInstance(Lattice *pLattice, double dH, int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder) {
    RectGridCreator *pRGC = new RectGridCreator(iHalo, bSuperCells);
    pRGC->m_pLattice = pLattice;
    pRGC->m_pVL      = pLattice->getLinkage();
    pRGC->m_pGP      = pLattice->getGridProjection();
    int iResult = pRGC->init(pRS, dH, bNodeOrder);
    if (iResult != 0) {
        delete pRGC;
        pRGC = NULL;
    }
    return pRGC;
}


//-----------------------------------------------------------------------------
// constructor
//
RectGridCreator::RectGridCreator(int iHalo, bool bSuperCells) 
    : GridCreator(iHalo, bSuperCells), 
      m_pLattice(NULL),
      m_bDeleteLattice(false) {
}

//-----------------------------------------------------------------------------
// destructor
//
RectGridCreator::~RectGridCreator() {
    if (m_bDeleteLattice) {
        delete m_pLattice;
    }
}

//-----------------------------------------------------------------------------
// init
//
int RectGridCreator::init(const char *pLatticeFile, RegionSplitter *pRS, double dH, bool bNodeOrder) {
    int iResult = createLattice(pLatticeFile);
    if (iResult == 0) {
        iResult = init(pRS, dH, bNodeOrder);
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// init
//
int RectGridCreator::init(RegionSplitter *pRS, double dH, bool bNodeOrder) {
    int iResult = 0;
    if (m_pLattice != NULL) {
        const ProjGrid *pPG = m_pLattice->getGridProjection()->getProjGrid();

        tbox box(0, pPG->m_iGridW+1, 0, m_pLattice->getNumY());
        pRS->setBox(&box);
        Region **apTiles = pRS->createRegions(&m_iNumTiles);
        if (apTiles != NULL) {

            if (iResult == 0) {
                PolarConv *pPC = new PolarConvGrid(m_pLattice->getGridProjection());
                createZones(apTiles, pPC, bNodeOrder);
                delete pPC;
            }
            // RegionSPlitter cleans up tiles
            /*
            for (int i = 0; i < m_iNumTiles; i++) {
                delete apTiles[i];
            }
            delete[] apTiles;
            */
        }
    }

    return iResult;
}
    
//-----------------------------------------------------------------------------
// createLattice
//
int RectGridCreator::createLattice(const char *pLatticeFile) {
    int iResult = -1;
    if ((m_pLattice != NULL) && m_bDeleteLattice) {
        delete m_pLattice;
    }
    m_pLattice = new Lattice();
    m_bDeleteLattice = true;
    iResult = m_pLattice->load(pLatticeFile);
    return iResult;
}
