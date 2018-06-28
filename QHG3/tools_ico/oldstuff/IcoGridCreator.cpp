#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "smallprimes.h"
#include "VertexLinkage.h"
#include "Icosahedron.h"
#include "IcoHeader.h"
#include "GridZones.h"
#include "IcoGridNodes.h"
#include "Region.h"
#include "RegionSplitter.h"

#include "GridCreator.h"
#include "IcoGridCreator.h"

// for ico grids we need to convert vectors to sphere coords in the default way
class PolarConvSphere : public PolarConv {
public:
    virtual void conv2Polar(Vec3D *pV, double *pdLon, double *pdLat) {
        cart2Sphere(pV, pdLon, pdLat);
    }
};
           

static bool s_bVerbose = false;
//-----------------------------------------------------------------------------
// constructor
//
IcoGridCreator::IcoGridCreator(int iHalo, bool bSuperCells) 
    : GridCreator(iHalo, bSuperCells), 
      m_pI(NULL),
      m_bDeleteIco(false) {
}


//-----------------------------------------------------------------------------
// destructor
//
IcoGridCreator::~IcoGridCreator() {
    printf("[IcoGridCreator::~IcoGridCreator] have m_IP=%p\n", m_pI);
    if (m_bDeleteIco && (m_pI != NULL)) {
        delete m_pI;
    }
}

//-----------------------------------------------------------------------------
// createInstance
//
IcoGridCreator *IcoGridCreator::createInstance(const char *pIcoFile, bool bPreSel, int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder) {
    IcoGridCreator *pIGC = new IcoGridCreator(iHalo, bSuperCells);
    int iResult = pIGC->init(pIcoFile, bPreSel, pRS, bNodeOrder);
    if (iResult != 0) {
        delete pIGC;
        pIGC = NULL;
    }
    return pIGC;
}

//-----------------------------------------------------------------------------
// createInstance
//
IcoGridCreator *IcoGridCreator::createInstance(Icosahedron *pIco, bool bPreSel, int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder) {
    IcoGridCreator *pIGC = new IcoGridCreator(iHalo, bSuperCells);
    pIGC->m_pI  = pIco;
    pIGC->m_pVL = pIco->getLinkage();
    int iResult = pIGC->init(bPreSel, pRS, bNodeOrder);
    if (iResult != 0) {
        delete pIGC;
        pIGC = NULL;
    }
    return pIGC;
}

//-----------------------------------------------------------------------------
// init
//
int IcoGridCreator::init(bool bPreSel, RegionSplitter *pRS, bool bNodeOrder) {
    int iResult = 0;
    
    if (m_pI != NULL) {
        pRS->setBox(m_pI->getBox());
        Region **apTiles = pRS->createRegions(&m_iNumTiles);
        if (iResult == 0) {
            PolarConv *pPC = new PolarConvSphere();
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

    return iResult;
}
//-----------------------------------------------------------------------------
// init
//
int IcoGridCreator::init(const char *pIcoFile, bool bPreSel, RegionSplitter *pRS, bool bNodeOrder) {
    int iResult = createIco(pIcoFile, bPreSel);
    if (iResult == 0) {
        iResult = init(bPreSel, pRS, bNodeOrder);
    }

    return iResult;
}
    


//-----------------------------------------------------------------------------
// createIco
//
int IcoGridCreator::createIco(const char *pIcoFile, bool bPreSel) {
    int iResult = -1;
   
    tbox tBox;
    int iLevel = 0;

    BufReader *pBR = BufReader::createInstance(pIcoFile, 256);
    if (pBR != NULL) {
        IcoHeader *pIH = new IcoHeader();
        m_bDeleteIco = true;
        iResult = pIH->read(pBR);
        if (iResult == 0) {
            iLevel = pIH->getSubLevel();
            pIH->getBox(tBox);
            delete pIH;
            
            // do the work
            m_pI = Icosahedron::create(1, POLY_TYPE_ICO);
            if (m_pI != NULL) {
                m_pI->setStrict(true);
                m_pI->setPreSel(bPreSel);
                m_pI->setSubLevel(iLevel);
                m_pI->setBox(tBox);
                iResult = m_pI->load(pIcoFile);
                if (iResult == 0) {
                    m_pI->relink();
                    m_pVL = m_pI->getLinkage();
                    if (s_bVerbose) {
                        m_pVL->display();
                    }
                }
            } else {
                iResult = -1;
                printf("Couldn't create icosahedron\n");
            }
        } else {
            iResult = -1;
            printf("Couldn't read IcoHeader from [%s]\n", pIcoFile);
        }
        delete pBR;
    }

    return iResult;
}

