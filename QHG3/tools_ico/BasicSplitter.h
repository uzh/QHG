#ifndef __BASICPLITTER_H__
#define __BASICPLITTER_H__

#include "icoutil.h"

class BasicTile;

class BasicSplitter {
public:
    BasicSplitter();
    virtual ~BasicSplitter();
    virtual BasicTile **createTiles(int *piNumTiles)= 0;
    void setBox( tbox *pBox) { m_pBox = pBox; };
    tbox *getBox() { return m_pBox; };
    int getNumTiles() { return m_iNumTiles;};
protected:
    int      m_iNumTiles;
    BasicTile **m_apTiles;
    tbox *m_pBox;
};


#endif

