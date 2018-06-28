#ifndef __NAVIGATION_H__
#define __NAVIGATION_H__

#ifdef OMP_A
#include <omp.h>
#endif

#include "types.h"

typedef std::map<gridtype, double>   distlist;
typedef std::map<gridtype, distlist> distancemap;

class Navigation {
public:
    Navigation();
    virtual ~Navigation();
    int setData(const distancemap &mDests, double dSampleDist);
    int checkSizes(uint iNumPorts, uint iNumDests, uint iNumDists);
 
    uint     m_iNumPorts;
    uint     m_iNumDests;
    uint     m_iNumDists;
    double   m_dSampleDist;

    distancemap m_mDestinations;
    
};



#endif
