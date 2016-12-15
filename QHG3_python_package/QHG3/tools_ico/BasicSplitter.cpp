#include <stddef.h>
#include "BasicTile.h"
#include "BasicSplitter.h"

//-----------------------------------------------------------------------------
// constructor
//
BasicSplitter::BasicSplitter() 
    : m_iNumTiles(0),
      m_apTiles(NULL) {

}

//-----------------------------------------------------------------------------
// destructor
//
BasicSplitter::~BasicSplitter() {
    if (m_apTiles != NULL) {
        for (int i = 0; i < m_iNumTiles; i++) {
            if (m_apTiles[i] != NULL) {
                delete m_apTiles[i];
            }
        }
        delete[] m_apTiles;
    }
}
