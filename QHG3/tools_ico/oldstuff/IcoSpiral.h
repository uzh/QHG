#ifndef __ICOSPIRAL_H__
#define __ICOSPIRAL_H__

#include <map>

#include "icoutil.h"

class VertexLinkage;
class IcoNode;

class IcoSpiral {
public:
    static IcoSpiral *createInstance(int iNumNodes, IcoNode **m_apNodes);
    ~IcoSpiral();
    IcoNode **getOrderedNodes() { return m_apNodesOut;};

protected:
    IcoSpiral(int iNumNodes, IcoNode **m_apNodes);
    int init(); // get nodes, create output array

    gridtype fillMap(); // fills ID to index map, returns id with highest latitude
    
    int trackNorthChain(gridtype IDHighest);
    gridtype findHighestUnmarked();

    int                       m_iNumNodes;
    std::map<gridtype, gridtype>  m_mID2IndexIn;


    IcoNode                 **m_apNodesIn;
    IcoNode                 **m_apNodesOut;
    int                       m_iCurIndex;
};


#endif
