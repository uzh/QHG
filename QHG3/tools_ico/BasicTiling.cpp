#include <stdio.h>

#include "types.h"
#include "EQSplitter.h"
#include "EQGridCreator.h"
#include "IcoGridNodes.h"
#include "EQTriangle.h"
#include "EQsahedron.h"
#include "BasicTiling.h"

//----------------------------------------------------------------------------
// constructor
//
BasicTiling::BasicTiling(int iSubDivNodes, int iVerbosity)
    : m_iSubDivNodes(iSubDivNodes),
      m_iNumTiles(0),
      m_apIGN(NULL),
      m_iVerbosity(iVerbosity) {

}


//----------------------------------------------------------------------------
// destructor
//
BasicTiling::~BasicTiling() {
    if (m_apIGN != NULL) {
        for (int i = 0; i < m_iNumTiles; i++) {
            delete m_apIGN[i];
        }
        delete[] m_apIGN;
    }

    if (m_pEQNodes != NULL) {
        delete m_pEQNodes;
    }
}

//----------------------------------------------------------------------------
// init
//
int BasicTiling::init() {
    int iResult = -1;

    m_pEQNodes = EQsahedron::createInstance(m_iSubDivNodes, true, NULL);
    if (m_pEQNodes != NULL) {
        iResult = 0;
    } else {
        printf("Couldn't create EQsahedron\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getIGN
//
IcoGridNodes  *BasicTiling::getIGN(int iTileID) {
    IcoGridNodes *pIGN = NULL;
    if ((iTileID >=0) && (iTileID < m_iNumTiles)) {
        pIGN = m_apIGN[iTileID];
    }
    return pIGN;
}
