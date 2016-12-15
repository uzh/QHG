#ifndef __MOVESTATS_H__
#define __MOVESTATS_H__

#ifdef OMP_A
#include <omp.h>
#endif

#include "types.h"

class MoveStats {
public:
    MoveStats();
    MoveStats(uint iNumCells);
    virtual ~MoveStats();

    int init(int iNumCells);
    uint m_iNumCells;
    
    int    *m_aiHops;
    double *m_adDist;
    double *m_adTime;
    
#ifdef OMP_A
	omp_lock_t *m_aStatLocks;
#endif
};

#endif
