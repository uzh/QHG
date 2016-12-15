#ifndef __ICONODE_H__
#define __ICONODE_H__

#include <set>
#include "types.h"

class IcoNode  {
public:
    IcoNode(IcoNode *pOrig);
    IcoNode(gridtype lID, double dTheta, double dPhi, double dArea);
    IcoNode(gridtype lID, gridtype lTID, double dTheta, double dPhi, double dArea);
   ~IcoNode();

    void addLink(gridtype iLink, double dDist);

    gridtype m_lID;  // global ID
    gridtype m_lTID; // tiled ID

    double m_dLon;
    double m_dLat;
    double m_dArea;

    unsigned int  m_iZone;
    int    m_iRegionID;

    int       m_iNumLinks;
    gridtype   *m_aiLinks;
    double   *m_adDists;
    std::set<gridtype> m_sDests;

    int m_iMarked;
};

#endif

