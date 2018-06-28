#ifndef __ANCMERGESORT_H__
#define __ANCMERGESORT_H__

#include "types.h"

class AncMergeSorter {

public:
    static AncMergeSorter *createInstance(long *pBuf, uint iNum, uint iRecSize);
    ~AncMergeSorter();
    int start();
    
protected:
    AncMergeSorter();
    int init(long *pBuf, uint iNumRecs, uint iRecSize);
    int splitMerge(uint iBegin, uint iEnd);
    int merge(uint iBegin, uint iMiddle, uint iEnd);
    int copyArray(uint iBegin, uint iEnd);
    

    long *m_pBuf;
    uint  m_iNumRecs;
    uint  m_iRecSize;
    long *m_pAux;

};


#endif
