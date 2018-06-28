#ifndef __PERMUTATOR_H__
#define __PERMUTATOR_H__

#include "types.h"

class WELL512;

class Permutator {
public:
    static Permutator *createInstance(uint iInitSize);
    ~Permutator();
    uint *permute(uint iNumTot, uint iNumSel, WELL512 *pWELL);

protected:
    Permutator(uint iInitSize);
    int init();
    void resize(uint iNewSize);

    uint  m_iSize;
    uint *m_aiPerm;
    uint m_iPrevTot;
};





#endif
