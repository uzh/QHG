#include <set>
#include "types.h"
#include "IcoNode.h"
#include "BasicTile.h"
#include "EQTile.h"


EQTile::EQTile(int iID, intset &sNodeIDs)
    : BasicTile(iID),
      m_sNodeIDs(sNodeIDs) {
}

bool EQTile::contains(IcoNode *pBC) {
    intset_cit it = m_sNodeIDs.find(pBC->m_lID);
    return (it != m_sNodeIDs.end());
}

/*
bool EQTile::contains(gridtype iID) {
    intset_cit it = m_sNodeIDs.find(iID);
    return (it != m_sNodeIDs.end());
}
*/
