#ifndef __ICOGRIDCREATOR_H__
#define __ICOGRIDCREATOR_H__

#include "icoutil.h"
#include "GridZones.h"
#include "GridCreator.h"

class IcoGridNodes;
class Icosahedron;
class Region;
class RegionSplitter;

class IcoGridCreator : public GridCreator {

public: 
    static IcoGridCreator *createInstance(const char *pIcoFile, bool bPreSel, int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder);
    static IcoGridCreator *createInstance(Icosahedron *pIco, bool bPreSel, int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder);
    virtual ~IcoGridCreator();
    
    //@@    IcoGridNodes *getGrid(int iID);
    //    static void createGridPoints(tbox &tBox, int iNX, int iNY, double *pdLons, double *pdLats);

protected:
    IcoGridCreator(int iHalo, bool bSuperCells);
    int init(const char *pIcoFile, bool bPreSel, RegionSplitter *pRS, bool bNodeOrder);
    int init(bool bPreSel, RegionSplitter *pRS, bool bNodeOrder);

    int createIco(const char *pIcoFile, bool bPreSel);
    
    //@@    void createZones(Region **apTiles);
    

    Icosahedron *m_pI;
    bool m_bDeleteIco;
    int m_iNX;
    int m_iNY;
    //    int m_iNumTiles;
    int m_iHalo;

    GridZones *m_pGZ;

    intcoll   *m_asNodeIDs;
};


#endif
