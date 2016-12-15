#include <string.h>
#include "utils.h"
#include "icoutil.h"
#include "IcoNode.h"

#define MAX_LINKS 6  //should be 6

//-----------------------------------------------------------------------------
// constructor
//
IcoNode::IcoNode(gridtype lID, double dLon, double dLat, double dArea)
    :  m_lID(lID),
       m_lTID(-1),
       m_dLon(dLon),
       m_dLat(dLat),
       m_dArea(dArea),
       m_iZone(ZONE_NONE),
       m_iRegionID(-1),
       m_iNumLinks(0),
       m_aiLinks(NULL),
       m_iMarked(0) {
    
    m_aiLinks = new gridtype[MAX_LINKS];
    m_adDists = new double[MAX_LINKS];

    memset(m_aiLinks, 0xff, MAX_LINKS*sizeof(gridtype));
}

//-----------------------------------------------------------------------------
// constructor
//
IcoNode::IcoNode(gridtype lID, gridtype lTID, double dLon, double dLat, double dArea)
    :  m_lID(lID),
       m_lTID(lTID),
       m_dLon(dLon),
       m_dLat(dLat),
       m_dArea(dArea),
       m_iZone(ZONE_NONE),
       m_iRegionID(-1),
       m_iNumLinks(0),
       m_aiLinks(NULL),
       m_iMarked(0) {
    
    m_aiLinks = new gridtype[MAX_LINKS];
    m_adDists = new double[MAX_LINKS];

    memset(m_aiLinks, 0xff, MAX_LINKS*sizeof(gridtype));
}

//-----------------------------------------------------------------------------
// constructor
//
IcoNode::IcoNode(IcoNode *pOrig) 
    :  m_lID(pOrig->m_lID),
       m_lTID(pOrig->m_lTID),
       m_dLon(pOrig->m_dLon),
       m_dLat(pOrig->m_dLat),
       m_dArea(pOrig->m_dArea),
       m_iZone(ZONE_NONE),
       m_iRegionID(pOrig->m_iRegionID),
       m_iNumLinks(pOrig->m_iNumLinks),
       m_aiLinks(NULL),
       m_iMarked(0) {
    
    m_aiLinks = new gridtype[MAX_LINKS];
    m_adDists = new double[MAX_LINKS];
    for (int i = 0; i < m_iNumLinks; i++) {
        m_aiLinks[i] = pOrig->m_aiLinks[i];
        m_adDists[i] = pOrig->m_adDists[i];
    }
    std::set<gridtype>::const_iterator it;
    for (it = pOrig->m_sDests.begin(); it !=  pOrig->m_sDests.end(); ++it) {
        m_sDests.insert(*it);
    }
}


//-----------------------------------------------------------------------------
// addLink
//
void IcoNode::addLink(gridtype iLink, double dDist) {
    m_aiLinks[m_iNumLinks] = iLink;
    m_adDists[m_iNumLinks] = dDist;
    m_iNumLinks++;
}

//-----------------------------------------------------------------------------
// destructor
//
IcoNode::~IcoNode() {

    if (m_aiLinks != NULL) {
        delete[] m_aiLinks;
    }
    if (m_adDists != NULL) {
        delete[] m_adDists;
    }
}
