#ifndef __SUBCOMPONENTMANAGER_H__
#define __SUBCOMPONENTMANAGER_H__

#include <map>
#include <vector>

#include "types.h"

#include "SubComponent.h"

typedef std::map<int, SubComponent*>  subcompmap;
typedef std::map<gridtype, int>       cellidtosubcompid;

typedef std::set<gridtype> neighbors;
typedef std::map<gridtype,neighbors> neighborlist;


class SCellGrid;
class SubComponentManager {
public:
    static SubComponentManager *createInstance(SCellGrid *pCG, const distancemap &mDistances, const neighborlist &mNeighbors, bool bVerbosity=false);
    virtual ~SubComponentManager();
    int init();
    void showSubComponents();
    const subcompmap &getSubComponents() { return m_mSubComponents;};
    void setVerbosity(bool bVerbose) {m_bVerbose = bVerbose;};
    void createDistanceMap(distancemap &mDistances);
protected:
    SubComponentManager(SCellGrid *pCG, const distancemap &mDistances, const neighborlist &mNeighbors, bool bVerbosity=false);
    int  createSubComponents();

    int  firstUnusedCellID();
    int  unusedNeighborIDs(intset &sIn, intset &sNeighbors);
    void splitForTarget();
    void deleteUnneeded();
  
    void eliminateDuplicates();

    SCellGrid         *m_pCG;
    // with c++11 we could have a 'const distancemap &', and use at() to access
    distancemap        m_mDistances;
    subcompmap         m_mSubComponents;
    cellidtosubcompid  m_mSubCompForCell;
    neighborlist       m_mNeighbors;
    bool               m_bVerbose;
};


#endif
