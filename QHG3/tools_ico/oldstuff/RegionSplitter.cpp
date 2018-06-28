#include <stddef.h>
#include "Region.h"
#include "RegionSplitter.h"

//-----------------------------------------------------------------------------
// constructor
//
RegionSplitter::RegionSplitter() 
    : m_iNumRegions(0),
      m_apRegions(NULL) {

}

//-----------------------------------------------------------------------------
// destructor
//
RegionSplitter::~RegionSplitter() {
    if (m_apRegions != NULL) {
        for (int i = 0; i < m_iNumRegions; i++) {
            if (m_apRegions[i] != NULL) {
                delete m_apRegions[i];
            }
        }
        delete[] m_apRegions;
    }
}
