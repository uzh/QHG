#ifndef __COMPONENTBUILDER_H__
#define __COMPONENTBUILDER_H__

#include <vector>
#include "types.h"

typedef std::vector<gridtype>     cellvec;
typedef std::set<gridtype>        cellset;
typedef std::map<gridtype, int>   idindex;

class SCellGrid;

class ComponentBuilder {
public:
    static ComponentBuilder *createInstance(SCellGrid *pCG, const cellvec &vSubSet);
    ~ComponentBuilder();

    int getNumComponents() { return m_iNumComponents; };
    int getComponentFor(gridtype iCellID);

protected:
    ComponentBuilder(SCellGrid *pCG, const cellvec &vSubSet);
    int init();
    int calcComponents();
    int createIdToIndex();
    int findNextFree(int iIndex);
    int calcComponentFor(int iIndex);

    SCellGrid     *m_pCG;
    const cellvec &m_vSubSet;
    int            m_iNumCells;
    int            m_iNumComponents;
    gridtype      *m_pIndexToComp;
    idindex        m_mIdToIndex;

};


#endif

