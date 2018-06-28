#include <stdio.h>

#include "types.h"
#include "EQSplitter.h"
#include "EQGridCreator.h"
#include "IcoGridNodes.h"
#include "EQTriangle.h"
#include "EQsahedron.h"
#include "LonLatSplitter.h"
#include "LonLatTiling.h"


//----------------------------------------------------------------------------
// createInstance
//
LonLatTiling *LonLatTiling::createInstance(int iSubDivNodes, int iNLon, int iNLat, double dMaxLat, int iVerbosity) {
    LonLatTiling *pLLT = new LonLatTiling(iSubDivNodes, iNLon, iNLat, dMaxLat, iVerbosity);
    int iResult = pLLT->init();
    if (iResult != 0) {
        delete pLLT;
        pLLT = NULL;
    }
    return pLLT;
}

//----------------------------------------------------------------------------
// constructor
//
LonLatTiling::LonLatTiling(int iSubDivNodes, int iNLon, int iNLat, double dMaxLat, int iVerbosity)
    : BasicTiling(iSubDivNodes, iVerbosity), 
      m_iNLon(iNLon),
      m_iNLat(iNLat),
      m_dMaxLat(dMaxLat) {

}
//----------------------------------------------------------------------------
// destructor
//
LonLatTiling::~LonLatTiling() {
}


//----------------------------------------------------------------------------
// init
//
int LonLatTiling::init() {
    int iResult = -1;

    iResult = BasicTiling::init();
    if (iResult == 0) {
        iResult = split();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// split
//
int LonLatTiling::split() {
    int iResult = -1;

    int iHalo = 1;


    LonLatSplitter *pLLS = new LonLatSplitter(m_pEQNodes, m_iNLon, m_iNLat, m_dMaxLat);
    EQGridCreator *pEGC = EQGridCreator::createInstance(m_pEQNodes, iHalo, pLLS, false/*true*/, m_iVerbosity);
    if (pEGC != NULL) {
        iResult = 0;
        printf("LonSplitter (%d,(%d,%d:%f)) gives %d regions\n", m_iSubDivNodes, m_iNLon, m_iNLat, m_dMaxLat, pLLS->getNumTiles());
        if (m_iVerbosity > LL_INFO) {
            printf("---\n");
            printf("--- EQTiling: extended display ---\n");
            printf("---\n");
            //    extendedEQDisplay(pENC, m_pEQNodes);
            //                extendedEQDisplay(pEQS->getENC(), m_pEQNodes);
        }


        m_apIGN = new IcoGridNodes*[pLLS->getNumTiles()];
        if (m_iVerbosity > LL_INFO) {
            printf("---\n");
            printf("--- EQTiling: zone info ---\n");
            printf("---\n");
        }
        for (int i = 0; (iResult == 0) && (i < pLLS->getNumTiles()); i++) {
            m_apIGN[i] = pEGC->getGrid(i);
            if (m_iVerbosity > LL_INFO) printf("IGN size %d: %zd\n", i, m_apIGN[i]->m_mNodes.size());
        }
             
        if (iResult == 0) {
            m_iNumTiles = pLLS->getNumTiles();
        }
        delete pEGC;
    }

    delete pLLS;
    return iResult;
}
