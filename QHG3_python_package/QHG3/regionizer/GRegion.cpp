#include <stdio.h>
#include <vector>
#include <algorithm>


#include "SCell.h"
#include "SCellGrid.h"

#include "GRegion.h"

//----------------------------------------------------------------------------
// constructor
//
GRegion::GRegion(int iID, SCellGrid *pCG, int *piAgentsPerCell, int *piMarks)
    : m_iID(iID),
      m_pCG(pCG),
      m_piAgentsPerCell(piAgentsPerCell),
      m_piMarks(piMarks),
      m_iTotal(0) {

}

//----------------------------------------------------------------------------
// destructor
//
GRegion::~GRegion() {
}


//----------------------------------------------------------------------------
// findNeighbors
//   valid neighbors are unmarked cells directly connected to an own cell
//
uint GRegion::findNeighbors() {
    m_vCurNeighbors.clear();

    intset::const_iterator it;
    for (it = m_vMembers.begin(); it != m_vMembers.end(); ++it) {
        const SCell &pSC = m_pCG->m_aCells[*it];
        for (int j = 0; j < pSC.m_iNumNeighbors; j++) {
            if (m_piMarks[pSC.m_aNeighbors[j]] < 0) {
                m_vCurNeighbors.insert(pSC.m_aNeighbors[j]);
            } else {
            }
        }
    }
    //    std::sort(m_vCurNeighbors.begin(), m_vCurNeighbors.end());
    return m_vCurNeighbors.size();
}

//----------------------------------------------------------------------------
// addCell
//   
uint GRegion::addCell(int iID) {
    m_vMembers.insert(iID);
    m_piMarks[iID] = m_iID;
    m_iTotal += m_piAgentsPerCell[iID];
    return m_vMembers.size();
}

//----------------------------------------------------------------------------
// addAllNeighbors
//   
uint GRegion::addAllNeighbors() {
    intset::const_iterator it;
    for (it = m_vCurNeighbors.begin(); it != m_vCurNeighbors.end(); ++it) {
        addCell(*it);
    }
    return m_vMembers.size();
}

//----------------------------------------------------------------------------
// addAllNeighbors
//   
uint GRegion::addRandomNeighbor() {
    int z = (int)((1.0*m_vCurNeighbors.size()*rand())/RAND_MAX);
    intset::const_iterator it;
    for (it = m_vCurNeighbors.begin(); (z>0) && (it != m_vCurNeighbors.end()); ++it, --z) {
        // do nothing
    }
    addCell(*it);
    return m_vMembers.size();
}

//----------------------------------------------------------------------------
// addSmallestNeighbor
//   
uint GRegion::addSmallestNeighbor() {
    int iMin = INT_MAX;
    intset::iterator itMin = m_vCurNeighbors.end();
    intset::const_iterator it;
    for (it = m_vCurNeighbors.begin(); it != m_vCurNeighbors.end(); ++it) {
        int iN = m_piAgentsPerCell[*it];
        if (iN < iMin) {
            iMin = iN;
            itMin = it;
        }
    }
    addCell(*itMin);
    return m_vMembers.size();
}

//----------------------------------------------------------------------------
// addLargestNeighbor
//   
uint GRegion::addLargestNeighbor() {
    int iMax = 0;
    intset::iterator itMax = m_vCurNeighbors.end();
    intset::const_iterator it;
    for (it = m_vCurNeighbors.begin(); it != m_vCurNeighbors.end(); ++it) {
        int iN = m_piAgentsPerCell[*it];
        if (iN > iMax) {
            iMax = iN;
            itMax = it;
        }
    }
    addCell(*itMax);
    return m_vMembers.size();
}

//----------------------------------------------------------------------------
// removeFromNeighbors
//   
int GRegion::removeFromNeighbors(intset &sRemove) {
    intset diff;
     //Create an insert_iterator for odd
    std::insert_iterator<std::set<int, std::less<int> > >
        diff_ins(diff, diff.begin());
    
    // calculate the difference
    set_difference(m_vCurNeighbors.begin(),m_vCurNeighbors.end(),
                   sRemove.begin(), sRemove.end(), diff_ins);
    
    // set neighbor set to reduced set
    m_vCurNeighbors = diff;

    return m_vCurNeighbors.size();
}

//----------------------------------------------------------------------------
// showRegion
//   
void GRegion::showRegion() {
    printf("  Region %03d: total %8u agents in %8zd cells; %8zd neighbors\n", m_iID, m_iTotal, m_vMembers.size(), m_vCurNeighbors.size());
    /*
    intset::const_iterator it;
    for (it = m_vCurNeighbors.begin(); it != m_vCurNeighbors.end(); ++it) {
        printf(" %d", *it);
    }
    printf("\n");
    */
}
