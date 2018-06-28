#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "Navigation.h"


//-----------------------------------------------------------------------------
// constructor
//
Navigation::Navigation()
    : m_iNumPorts(0),
      m_iNumDests(0),
      m_iNumDists(0),
      m_dSampleDist(0) {
}


//-----------------------------------------------------------------------------
// setData
//
int Navigation::setData(const distancemap &mDests, double dSampleDist) {
    m_dSampleDist   = dSampleDist;
    m_mDestinations = mDests;
    
    // set port count & calculate dist and dest  counts
    m_iNumPorts     = m_mDestinations.size();
    m_iNumDists     = 0;
    distancemap::const_iterator it;
    for (it = m_mDestinations.begin(); it != m_mDestinations.end(); ++it) {
        m_iNumDists += it->second.size();
    }
    m_iNumDests = m_iNumDists + m_iNumPorts;
    
    //@@    printf("[Navigation]numports %u, numdists %u, numdests: %u\n", m_iNumPorts, m_iNumDists, m_iNumDests);
    
    return 0;
}


//-----------------------------------------------------------------------------
// destructor
//
Navigation::~Navigation() {
}


//-----------------------------------------------------------------------------
// checkSizes
//
int Navigation::checkSizes(uint iNumPorts, uint iNumDests, uint iNumDists) {
    int iResult = -1;
    if ((iNumPorts == m_iNumPorts) &&
        (iNumDests == m_iNumDests) &&
        (iNumDists == m_iNumDists)) {
        iResult = 0;
    }
    return iResult;
}
