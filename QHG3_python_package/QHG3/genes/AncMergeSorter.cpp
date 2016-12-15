#include <stdio.h>
#include <string.h>
#include "AncMergeSorter.h"

//----------------------------------------------------------------------------
// createInstance
//
AncMergeSorter *AncMergeSorter::createInstance(long *pBuf, uint iNumRecs, uint iRecSize) {
    AncMergeSorter *pAMS = new AncMergeSorter();
    int iResult = pAMS->init(pBuf, iNumRecs, iRecSize);
    if (iResult != 0) {
        delete pAMS;
        pAMS = NULL;
    }
    return pAMS;
}

//----------------------------------------------------------------------------
// constructor
//
AncMergeSorter::AncMergeSorter() 
    : m_pBuf(NULL),
      m_iNumRecs(0),
      m_iRecSize(0),
      m_pAux(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
AncMergeSorter::~AncMergeSorter() {
    if (m_pAux != NULL) {
        delete[] m_pAux;
    }
}

//----------------------------------------------------------------------------
// start
//
int AncMergeSorter::start() {
    return splitMerge(0, m_iNumRecs);
}
    
//----------------------------------------------------------------------------
// init
//
int AncMergeSorter::init(long *pBuf, uint iNumRecs, uint iRecSize) {
    int iResult = -1;

    m_pBuf = pBuf;
    m_iNumRecs = iNumRecs;
    m_iRecSize = iRecSize;
    if (m_iNumRecs*m_iRecSize > 0) {
        m_pAux = new long[iNumRecs*iRecSize];
        iResult = 0;
    } else {
        printf("Bad values for sizes\n");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// splitMerge
//
int AncMergeSorter::splitMerge(uint iBegin, uint iEnd) {
    int iResult = 0;

    if(iEnd - iBegin >= 2) {                       // if run size == 1
        
    // recursively split runs into two halves until run size == 1,
    // then merge them and return back up the call chain
        uint iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
        splitMerge(iBegin,  iMiddle);  // split / merge left  half
        splitMerge(iMiddle, iEnd);  // split / merge right half
        merge(iBegin, iMiddle, iEnd);  // merge the two half runs
        copyArray(iBegin, iEnd);              
    }
    return iResult;
}

//----------------------------------------------------------------------------
// merge
//
int AncMergeSorter::merge(uint iBegin, uint iMiddle, uint iEnd) {
    int iResult = 0;

    uint i0 = iBegin;
    uint i1 = iMiddle;
    
    // While there are elements in the left or right runs
    for (uint j = iBegin; j < iEnd; j++) {
        // If left run head exists and is <= existing right run head.
        if (i0 < iMiddle && (i1 >= iEnd || m_pBuf[i0*m_iRecSize] <= m_pBuf[i1*m_iRecSize])) {
            
            memcpy(m_pAux+j*m_iRecSize, m_pBuf+i0*m_iRecSize, m_iRecSize*sizeof(long));
            /*
            for (uint k = 0; k < m_iRecSize; k++) {
                m_pAux[j*m_iRecSize+k] = m_pBuf[i0*m_iRecSize+k];
            }
            */
            i0 = i0 + 1;
        } else {
            
            memcpy(m_pAux+j*m_iRecSize, m_pBuf+i1*m_iRecSize, m_iRecSize*sizeof(long));
            /*
            for (uint k = 0; k < m_iRecSize; k++) {
                m_pAux[j*m_iRecSize+k] = m_pBuf[i1*m_iRecSize+k];
            }
            */
            i1 = i1 + 1;    
        }
    } 

    return iResult;
}

//----------------------------------------------------------------------------
// copyArray
//
int AncMergeSorter::copyArray(uint iBegin, uint iEnd) {
    int iResult = 0;
    
    memcpy(m_pBuf+iBegin*m_iRecSize, m_pAux+iBegin*m_iRecSize, (iEnd-iBegin)*m_iRecSize*sizeof(long));
    /*
    
    for (uint i = iBegin; i < iEnd; i++) {
        for (uint k = 0; k < m_iRecSize; k++) {
            m_pBuf[i*m_iRecSize+k] = m_pAux[i*m_iRecSize+k];
        }
    }
    */
    return iResult;
}




// forwards
int splitMerge(long *pBufIn, int iBegin, int iEnd, long *pBufOut, int iRecSize);
int merge(long *pBufIn, int iBegin, int iMiddle, int iEnd, long *pBufOut, int iRecSize);
int copyArray(long *pBufOut, int iBegin, int iEnd, long *pBufIn, int iRecSize);


int AncMergeSort(long *pBufIn, long *pBufOut, int iNum, int iRecSize) {
    int iResult = splitMerge(pBufIn, 0, iNum, pBufOut, iRecSize);
    return iResult;
}

int splitMerge(long *pBufIn, int iBegin, int iEnd, long *pBufOut, int iRecSize) {
    int iResult = 0;
    printf("splitmerge: b %d, e %d\n", iBegin, iEnd);
    if(iEnd - iBegin >= 2) {                       // if run size == 1
     
    // recursively split runs into two halves until run size == 1,
    // then merge them and return back up the call chain
        int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
        splitMerge(pBufIn, iBegin,  iMiddle, pBufOut, iRecSize);  // split / merge left  half
        splitMerge(pBufIn, iMiddle, iEnd,    pBufOut, iRecSize);  // split / merge right half
        merge(pBufIn, iBegin, iMiddle, iEnd, pBufOut, iRecSize);  // merge the two half runs
        copyArray(pBufOut, iBegin, iEnd, pBufIn, iRecSize);              
    }
    return iResult;
}


int merge(long *pBufIn, int iBegin, int iMiddle, int iEnd, long *pBufOut, int iRecSize) {
    int iResult = 0;

    int i0 = iBegin;
    int i1 = iMiddle;
    
    // While there are elements in the left or right runs
    for (int j = iBegin; j < iEnd; j++) {
        // If left run head exists and is <= existing right run head.
        if (i0 < iMiddle && (i1 >= iEnd || pBufIn[i0*iRecSize] <= pBufIn[i1*iRecSize])) {
            for (int k = 0; k < iRecSize; k++) {
                pBufOut[j*iRecSize+k] = pBufIn[i0*iRecSize+k];
            }
            i0 = i0 + 1;
        } else {
            for (int k = 0; k < iRecSize; k++) {
                pBufOut[j*iRecSize+k] = pBufIn[i1*iRecSize+k];
            }
            i1 = i1 + 1;    
        }
    } 

    return iResult;
}


int copyArray(long *pBufOut, int iBegin, int iEnd, long *pBufIn, int iRecSize) {
    int iResult = 0;
    /*
    memcpy(pBufIn+iBegin*iRecSize, pBufOut, (iBegin-iEnd)*iRecSize*sizeof(long));
    */
    for (int i = iBegin; i < iEnd; i++) {
        for (int k = 0; k < iRecSize; k++) {
            pBufIn[i*iRecSize+k] = pBufOut[i*iRecSize+k];
        }
    }
    return iResult;
}
