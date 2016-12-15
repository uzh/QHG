#ifndef __EQTILELINKS_H__
#define __EQTILELINKS_H__

#include <map>

#include "types.h"
#include "EQTileLinks.h"

class IcoGridNodes;

typedef std::map<gridtype, intset>         tTileCommUnits;
typedef std::map<gridtype, tTileCommUnits> tGlobalCommUnits;

class EQTileLinks {
public:
    static EQTileLinks *createInstance(IcoGridNodes **apIGN, int iNumTiles);
    static EQTileLinks *createInstance(const char *pInput);

    int getNumTiles() {return m_iNumTiles;};

    const tTileCommUnits   &getBorderUnits(gridtype iTile) { return m_mmLinkUnits[0][iTile];};
    const tTileCommUnits   &getHaloUnits(gridtype iTile)   { return m_mmLinkUnits[1][iTile];};

    const tGlobalCommUnits &getGlobalBorderUnits() { return m_mmLinkUnits[0];};
    const tGlobalCommUnits &getGlobalHaloUnits()   { return m_mmLinkUnits[1];};

    int write(const char *pOut);
    int read(const char *pOut);

protected:
    EQTileLinks();
    EQTileLinks(IcoGridNodes **apIGN, int iNumTiles);
    int init();

    IcoGridNodes **m_apIGN;
    int            m_iNumTiles;

    tGlobalCommUnits m_mmLinkUnits[2];
    
};


#endif
