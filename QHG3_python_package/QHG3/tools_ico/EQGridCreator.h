#ifndef __EQGRIDCREATOR_H__
#define __EQGRIDCREATOR_H__

#include "types.h"
#include "EQZones.h"

class VertexLinkage;
class IcoGridNodes;
class EQsahedron;
class BasicSplitter;
class BasicTile;

class EQGridCreator {

public: 
    static EQGridCreator *createInstance(EQsahedron *pEQ, int iHalo, BasicSplitter *pTS, bool bSanityCheck, int iVerbosity);
    virtual ~EQGridCreator();
    
    IcoGridNodes *getGrid(int iID);
protected:
    EQGridCreator(int iHalo, int iVerbosity);
    int init(BasicSplitter *pRS);
    int createZones(BasicTile **asTiles);
    bool zoneSanity();

    
    EQsahedron *m_pEQ;
    int m_iHalo;
    int m_iNumTiles;
    VertexLinkage *m_pVL;

    EQZones *m_pEQZ;
    intcoll   *m_asNodeIDs;
    bool m_bSanityCheck;
    int m_iVerbosity;
};


#endif
