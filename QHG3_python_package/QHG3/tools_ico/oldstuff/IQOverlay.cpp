#include <stdio.h>
#include <vector>
#include <set>

#include "IQOverlay.h"
#include "notification_codes.h"

//-----------------------------------------------------------------------------
// constructor
//
IQOverlay::IQOverlay() {
}

//-----------------------------------------------------------------------------
// destructor
//
IQOverlay::~IQOverlay() {
}

//-----------------------------------------------------------------------------
// clear
//
void IQOverlay::clear() {
    m_sNodes.clear();
    notifyObservers(NOTIFY_LU_CHANGE, NULL);
}

//-----------------------------------------------------------------------------
// contains
//
bool IQOverlay::contains(gridtype lNode) {
    std::set<gridtype>::const_iterator it = m_sNodes.find(lNode);
    return (it != m_sNodes.end());
}

//-----------------------------------------------------------------------------
// setOverlay 
//
void IQOverlay::addData(std::vector<gridtype> vOverlays) {
    m_sNodes.insert(vOverlays.begin(), vOverlays.end());
    printf("[IQOverlay::addData] sending LU_CHANGE\n");
    notifyObservers(NOTIFY_LU_CHANGE, NULL);
}

//-----------------------------------------------------------------------------
// hasData
//
bool IQOverlay::hasData() {
    return !m_sNodes.empty();
}

//-----------------------------------------------------------------------------
// show
//
void IQOverlay::show() {
    std::set<gridtype>::const_iterator it;
    printf("[");
    for (it = m_sNodes.begin(); it != m_sNodes.end(); ++it) {
        printf("  %d", *it);
    }
    printf("]\n");
}
