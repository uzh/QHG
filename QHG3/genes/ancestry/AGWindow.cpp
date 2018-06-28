#include <stdio.h>
#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "RWAncGraph.h"
#include "AGOracle.h"
#include "AGWindow.h"



//----------------------------------------------------------------------------
// createInstance
// 
AGWindow *AGWindow::createInstance(const char *pAGFile, int iBlockSize) {
    AGWindow *pAGW = new AGWindow();
    int iResult = pAGW->init(pAGFile, iBlockSize);
    if (iResult != 0) {
        delete pAGW;
        pAGW = NULL;
    }
    return pAGW;
}

//----------------------------------------------------------------------------
// constructor
// 
AGWindow::AGWindow()
    : m_pAG(NULL),
      m_pAGO(NULL),
      m_iCurMinID(-1),
      m_iCurMaxID(-1) {
}

//----------------------------------------------------------------------------
// destructor
// 
AGWindow::~AGWindow() {
    if (m_pAGO != NULL) {
        delete m_pAGO;
    }
    if (m_pAG != NULL) {
        delete m_pAG;
    }
}


//----------------------------------------------------------------------------
// init
// 
int AGWindow::init(const char *pAGFile, int iBlockSize) {
    int iResult = -1;
    
    m_pAGO = AGOracle::createInstance(pAGFile, iBlockSize);
    if (m_pAGO != NULL) {
        m_pAG = new RWAncGraph();
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// loadBlockFor
// 
int AGWindow::loadBlockFor(idtype iID) {
    int iResult = 0;
    if ((m_iCurMinID < 0) || (m_iCurMaxID < 0) ||
        (iID <= m_iCurMinID) || (iID > m_iCurMaxID)) {
        m_iCurMinID = -1;
        m_iCurMaxID = -1;
        //        printf("loading block...\n");
        m_pAG->clear();
        iResult = m_pAGO->loadBlock(m_pAG, iID, &m_iCurMinID, &m_iCurMaxID);
        //        printf("                       \r");
    } else {
        //        printf("Block already loaded\n");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getNode
// 
AncestorNode *AGWindow::getNode(idtype iID) {
    AncestorNode *pAN = NULL;
    if (iID > 0) {
        int iResult = loadBlockFor(iID);
        if (iResult == 0) {
            pAN = m_pAG->findAncestorNode(iID);
            if (pAN == NULL) {
                printf("ID %ld not found: Either it is not in AGfile, or AGfile is not sorted\n", iID);
            }
        } else {
            printf("couldn't find block for id %ld\n", iID);
        }   
    }
    return pAN;
}

    
//----------------------------------------------------------------------------
// getFirst
// 
AncestorNode *AGWindow::getFirst(idtype iID) {
    AncestorNode *pAN = NULL;
    // make sure ffirst block is loaded
    int iResult = loadBlockFor(iID);
    if (iResult == 0) {
        const ancnodelist &mIndex = m_pAG->getMap();
        if (mIndex.size() > 0) {
            m_itCur = mIndex.begin();
            pAN = m_itCur->second;
            ++m_itCur;
        } else {
            printf("AGFile appears to be empty\n");
        }
    } else {
        printf("Couldn't read block\n");
    }
    return pAN;
}

//----------------------------------------------------------------------------
// getFirst
// 
AncestorNode *AGWindow::getFirst() {
    AncestorNode *pAN = NULL;
    // make sure ffirst block is loaded
    m_itBlock = m_pAGO->getList().begin();
    if (m_itBlock != m_pAGO->getList().end()) {
        pAN = getFirst(m_itBlock->first);
        if (pAN != NULL) {
            ++m_itBlock;
        } else {
            printf("Couldn't read node\n");
        }
    } else {
        printf("Have no blocks\n");
    }
    return pAN;
}


//----------------------------------------------------------------------------
// getNext
// 
AncestorNode *AGWindow::getNext() {
    AncestorNode *pAN = NULL;
    
    if (m_itCur ==  m_pAG->getMap().end()) {
        if (m_itBlock != m_pAGO->getList().end()) {
            pAN = getFirst(m_itBlock->first);
        } else {
            // reached end of block list
        }
    } else {
        pAN = m_itCur->second;
        ++m_itCur;
    }
    return pAN;
        
}

        
