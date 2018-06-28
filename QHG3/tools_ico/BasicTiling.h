#ifndef __BASICTILING_H__
#define __BASICTILING_H__

#include "dbgprint.h"

class IcoGridNodes;
class EQsahedron;


class BasicTiling {
public:
    BasicTiling(int iSubDivNodes, int iVerbosity);
    
    virtual ~BasicTiling();
    int getNumTiles() { return m_iNumTiles;};
    IcoGridNodes **getIGNs() {return m_apIGN;};    
    IcoGridNodes  *getIGN(int iTileID);    
    EQsahedron *getEQsahedron() { return m_pEQNodes;};


    int getSubDivNodes() { return m_iSubDivNodes;};
protected:
    virtual int init();
    int m_iSubDivNodes;
    int m_iNumTiles;
    IcoGridNodes **m_apIGN;    
    EQsahedron *m_pEQNodes;
    int m_iVerbosity;
};

#endif
