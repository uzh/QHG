
#include "dbgprint.h"
#include "EQsahedron.h"
#include "EQTile.h"
#include "EQSplitter.h"
#include "EQNodeClassificator.h"

/*
//----------------------------------------------------------------------------
// constructor
//
EQSplitter::EQSplitter(EQsahedron *pEQ, int iSubDivTiles, int iVerbosity) :  
    m_pENC(NULL),
    m_pEQ(pEQ),
    m_iVerbosity(iVerbosity) {
    
    if (m_pEQ != NULL) {
        m_pEQ->relink();
        int iSubDivNodes = m_pEQ->getSubDivs();
        m_pENC =  EQNodeClassificator::createInstance(iSubDivNodes, iSubDivTiles);
        if (m_pENC != NULL) {
            m_iNumTiles = m_pENC->getNumTiles();
            
            int iResult = m_pENC->applyToEQ(m_pEQ);
            if (iResult == 0) {
                m_pENC->distributeBorders();
                dbgprintf(m_iVerbosity, LL_DETAIL, "---EQSplitter has ENC (%d tiles)---\n", m_iNumTiles);
                if (m_iVerbosity > LL_DETAIL)  {
                    m_pENC->showFinal();
                }
            } else {
                // should fail
            }
        } else {
            // should fail
        }
    } else {
        // should fail
    }
}
*/
//----------------------------------------------------------------------------
// constructor
//
EQSplitter::EQSplitter(EQsahedron *pEQ, EQNodeClassificator *pENC, int iVerbosity)
    : m_pENC(pENC),
      m_pEQ(pEQ),
      m_iVerbosity(iVerbosity) {
    
    if (m_pEQ != NULL) {
        m_pEQ->relink();
        
        m_iNumTiles = m_pENC->getNumTiles();
            
        int iResult = m_pENC->applyToEQ(m_pEQ);
        if (iResult == 0) {
            m_pENC->distributeBorders();
            dbgprintf(m_iVerbosity, LL_DETAIL, "---EQSplitter has ENC (%d tiles)---\n", m_iNumTiles);
            if (m_iVerbosity > LL_DETAIL)  {
                m_pENC->showFinal();
            }
        } else {
            // should fail
        }
        
    } else {
        // should fail
    }
}




//----------------------------------------------------------------------------
// destructor
//
EQSplitter::~EQSplitter() {
    /*@@@@@@@@
    if (m_pENC != NULL) {
        delete m_pENC;
    }
    */
    if (m_asTileNodes != NULL) {
        for (int i = 0; i < m_iNumTiles; i++) { 
            delete m_asTileNodes[i];
        }
        delete[] m_asTileNodes;
    }
}

//----------------------------------------------------------------------------
// createTiles
//
BasicTile **EQSplitter::createTiles(int *piNumTiles) {
    BasicTile **pTiles = NULL;
    if (m_pENC != NULL) {
        
        m_iNumTiles = (m_pENC->getSubDivTiles()>0)?m_iNumTiles:1;
        *piNumTiles = m_iNumTiles;
        
        m_asTileNodes = new BasicTile*[m_iNumTiles];

        for (int i = 0; i < m_iNumTiles; i++) { 
            m_asTileNodes[i] = new EQTile(i, m_pENC->getFinal(i));
        }
        pTiles = m_asTileNodes;
    } else {
        *piNumTiles = 0;
    }
    return pTiles;
}
