#ifndef __IRREGREGION_H__
#define __IRREGREGION_H__

#include <map>
#include <set>
#include <queue>
#include "types.h"
#include "icoutil.h"

class IcoNode;


// regionNodes
typedef std::map<gridtype, IcoNode *>   nodelist;

typedef std::set<gridtype>              nodeset;
typedef std::map<gridtype, nodeset>     regionnodes; // region ID -> nodes


typedef std::map<gridtype, std::set<gridtype> > conflictnodes; // node -> conflicting regions

typedef std::map<gridtype, int>               nodecounts;
typedef std::map<gridtype, nodecounts >        regionnodecounts;
typedef std::map<gridtype, regionnodecounts > weightedneighbors;

typedef std::map<gridtype, bool>               regionmarks;


class IrregRegion {
public: 

    IrregRegion(nodelist &listNodes);

    int add(gridtype iRegion, gridtype iNode);
    int add(gridtype iRegion, nodeset &sN);

    void unmarkBorderNodes(gridtype iRegion);
    void markBorderNodes(gridtype iRegion);

    int collectFreeNeighbors(gridtype iNode, nodeset &sNeighbors);
    int collectFreeNeighbors(nodeset &sNodes, nodeset &sNeighbors);

    int collectNeighbors(gridtype iRegion, gridtype iNode);

    int grow();
    int equalize(bool bHigher);

    void listSizes();
    void listN(bool bWithLinks=false);
    void listWN();

    int collectNeighborsPerRegion();

    bool doesNotSplitRegion(gridtype iRegion, gridtype iNode, int iConnectivity);
    
protected:
    bool findPathDijk(gridtype iStart, gridtype iEnd, std::vector<gridtype> &vPath);
    void findBestTransferNodes(gridtype iRPrev, gridtype iRCur, int iSendNum, nodeset &sN);
    int  swapNodes(int iSendNum, std::queue<gridtype> &vPath); 

    int  collectNodeNeighbors(gridtype iNode, nodeset &sN, gridtype iAvoid);
    int  removeBadNeighbors(nodeset &sN, gridtype iRegion);
    int  countComponents(nodeset &sN);
    regionnodes   m_curNodes;
    regionnodes   m_curBorders;
    conflictnodes m_curConflicts;    

    regionnodes   m_newBorders;
    nodelist      m_listNodes;

    weightedneighbors m_wNeighbors;
    regionmarks   m_Unused;

};




#endif
