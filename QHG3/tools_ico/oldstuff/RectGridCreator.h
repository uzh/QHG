#ifndef __RECTGRIDCREATOR_H__
#define __RECTGRIDCREATOR_H__

#include "GridCreator.h"
#include "icoutil.h"


class IcoNode;
class RegionSplitter;
class Vec3D;
class Lattice;

class RectGridCreator : public GridCreator {
public:
    //    static RectGridCreator *createInstance(const char *pLatticeFile,  int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder);
    static RectGridCreator *createInstance(Lattice *pLattice, double dH,  int iHalo, RegionSplitter *pRS, bool bSuperCells, bool bNodeOrder);

    virtual ~RectGridCreator();
   
 protected:
    RectGridCreator(int iHalo, bool bSuperCells);
    int init(const char *pLatticeFile, RegionSplitter *pRS, double dHalo, bool bNodeOrder);
    int init(RegionSplitter *pRS, double dHalo, bool bNodeOrder);

    int createRectVL(int iW, int iH, int iNumLinks);
    
    int createLattice(const char *pLatticeFile);
public:
    Lattice *m_pLattice;
    bool m_bDeleteLattice;
};


#endif




