#include <stdio.h>

#include "types.h"
#include "EQSplitter.h"
#include "EQGridCreator.h"
#include "IcoGridNodes.h"
#include "EQTriangle.h"
#include "EQsahedron.h"
#include "EQNodeClassificator.h"
#include "EQSplitter.h"
#include "EQTiling.h"


//----------------------------------------------------------------------------
// createInstance
//
EQTiling *EQTiling::createInstance(int iSubDivNodes, int iSubDivTiles, int iVerbosity) {

    EQTiling *pEQT = new EQTiling(iSubDivNodes, iSubDivTiles, iVerbosity);
    int iResult = pEQT->init();
    if (iResult != 0) {
        delete pEQT;
        pEQT = NULL;
    }

    return pEQT;
}


//----------------------------------------------------------------------------
// constructor
//
EQTiling::EQTiling(int iSubDivNodes, int iSubDivTiles, int iVerbosity)
    : BasicTiling(iSubDivNodes, iVerbosity), 
      m_iSubDivTiles(iSubDivTiles) {

}


//----------------------------------------------------------------------------
// destructor
//
EQTiling::~EQTiling() {
    /*
    if (m_apIGN != NULL) {
        for (int i = 0; i < m_iNumTiles; i++) {
            delete m_apIGN[i];
        }
        delete[] m_apIGN;
    }

    if (m_pEQNodes != NULL) {
        delete m_pEQNodes;
    }
    */
}

//----------------------------------------------------------------------------
// init
//
int EQTiling::init() {
    int iResult = -1;


    if ((m_iSubDivNodes+1)%(m_iSubDivTiles+1) == 0) {
        
        if (((m_iSubDivNodes == 0) && (m_iSubDivTiles == 0)) ||
            ((m_iSubDivNodes+1)/(m_iSubDivTiles+1) > 2)) {
            
            /*
            m_pEQNodes = EQsahedron::createInstance(m_iSubDivNodes, true, NULL);
            if (m_pEQNodes != NULL) {
            */  
            iResult = BasicTiling::init();
            if (iResult == 0) {
                iResult = split();
            }
            /*
            } else {
                printf("Couldn't create EQsahedron\n");
            }
            */
        } else {
            printf("Incompatible subdivs: (<subdivnodes>+1)/(<subdivtiles>+1) must be greater than 2, or both must be 0\n");
        }
    } else {
        printf("TileIncompatible subdivs: <subdivnodes>+1 must be divisible by <subdivtiles>+1\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// split
//
int EQTiling::split() {
    int iResult = -1;

    int iHalo = 1;

    EQNodeClassificator *pENC =  EQNodeClassificator::createInstance(m_iSubDivNodes, m_iSubDivTiles);
    if (pENC != NULL) {

        EQSplitter *pEQS = new EQSplitter(m_pEQNodes, pENC, m_iVerbosity);
        //        EQSplitter *pEQS = new EQSplitter(m_pEQNodes, m_iSubDivTiles, m_iVerbosity);
        EQGridCreator *pEGC = EQGridCreator::createInstance(m_pEQNodes, iHalo, pEQS, false/*true*/, m_iVerbosity);
        if (pEGC != NULL) {
            iResult = 0;
            printf("EQSplitter (%d,%d) gives %d regions\n", m_iSubDivNodes, m_iSubDivTiles, pEQS->getNumTiles());
            if (m_iVerbosity > LL_INFO) {
                printf("---\n");
                printf("--- EQTiling: extended display ---\n");
                printf("---\n");
                extendedEQDisplay(pENC, m_pEQNodes);
                //                extendedEQDisplay(pEQS->getENC(), m_pEQNodes);
            }


            m_apIGN = new IcoGridNodes*[pEQS->getNumTiles()];
            if (m_iVerbosity > LL_INFO) {
                printf("---\n");
                printf("--- EQTiling: zone info ---\n");
                printf("---\n");
            }
            for (int i = 0; (iResult == 0) && (i < pEQS->getNumTiles()); i++) {
                m_apIGN[i] = pEGC->getGrid(i);
                if (m_iVerbosity > LL_INFO) printf("IGN size %d: %zd\n", i, m_apIGN[i]->m_mNodes.size());
            }
             
            if (iResult == 0) {
                m_iNumTiles = pEQS->getNumTiles();
            }
            delete pEGC;
        }

        delete pEQS;
        delete pENC;
    } else {
    }
    return iResult;
}



//----------------------------------------------------------------------------
// extendedEQDisplay
//
void EQTiling::extendedEQDisplay(EQNodeClassificator *pENC, EQsahedron *pEQNodes) {
    for (int f = 0; f < ICOFACES; ++f) {
        printf("---F %02d ---\n", f);
        EQTriangle *pEQT = pEQNodes->getFaceTriangle(f);
        int iNumNodes = pEQT->getNumNodes();
        node *pNodes = pEQT->getNodes();
        int iWidth = 1+log(ICOFACES*iNumNodes+1)/log(10);
        printf("nn %d, iWidth: %d\n", iNumNodes, iWidth);
        int k = 1;
        int s = 2;
        for (int iNode = 0; iNode < iNumNodes; iNode++) {
            intset sFaceCommon;
            printf("%*d", iWidth, pNodes[iNode].lID);
            int sfnt = pENC->getFaceNodeTiles(iNode, sFaceCommon);
            if (sfnt > 0) {
                printf("f ");
            } else {
                intset sEdgeCommon;
                int sent = pENC->getEdgeNodeTiles(iNode, sEdgeCommon);
                if (sent > 0) {
                    printf("e ");
                } else {
                    intset sVertCommon;
                    int svnt = pENC->getVertNodeTiles(iNode, sVertCommon);
                    if (svnt > 0) {
                        printf("v ");
                    } else {
                        printf("? ");
                    }
                }
            }
            if (iNode == k-1) {
                k += s;
                s++;
                printf("\n");
            }
        }
    }
}
