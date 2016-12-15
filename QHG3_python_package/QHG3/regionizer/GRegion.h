#ifndef __GREGION_H__
#define __GREGION_H__

#include <vector>
#include <set>
#include "types.h"

typedef std::set<int> intset;
class SCellGrid;

class GRegion {
public:
    GRegion(int iID, SCellGrid *pCG, int *piAgentsPerCell, int *piMarks);
    ~GRegion();

    uint findNeighbors();
    uint addCell(int iID);

    void showRegion();
    int  getTotal() { return m_iTotal;};
    int  getNumCells() { return m_vMembers.size();};
    int  getNumNeighbors() { return m_vCurNeighbors.size();};
    intset &getNeighbors() { return m_vCurNeighbors;};
    int removeFromNeighbors(intset &sRemove);
    
    uint addAllNeighbors();
    uint addRandomNeighbor();
    uint addSmallestNeighbor();
    uint addLargestNeighbor();
    
protected:
    int        m_iID;
    SCellGrid *m_pCG;
    int       *m_piAgentsPerCell;
    int       *m_piMarks;
    uint       m_iTotal;
    intset     m_vMembers;
    intset     m_vCurNeighbors;
};

#endif
