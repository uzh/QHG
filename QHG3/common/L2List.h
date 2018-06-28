/*=============================================================================
| L2List
| 
|  Two doubly linked lists in a single array.
|  Used to keep track of used and unused elements in aarrays such as 
|  LayerBuf or LayerArrBuf,and provide fast access to unused indexes.
| 
|  Internally this is managed by two doubly linked lists of indexes,
|  "ACTIVE" and "PASSIVE". The nodes of both lists are  elements of 
|  a single array.
|  When a new index is used, the corresponding node is moved from 
|  the PASSIVE list to the ACTIVE list. If an index is not needed 
|  anymore, the corresponding node is moved from the "ACTIVE"
|  list to the "PASSIVE" list.
|  
|  It is the users responsability to make sure that 
|  no element is deleted twice, as this corrupts the linkage 
|  
|  Author: Jody Weissmann
\============================================================================*/

#ifndef __L2LIST_H__
#define __L2LIST_H__

#include "types.h" //  uint uchar...

const int NIL = -1;
const uchar PASSIVE = 0;
const uchar ACTIVE  = 1;

const int DUMP_MODE_NONE  = -1;
const int DUMP_MODE_FLAT  =  0;
const int DUMP_MODE_SMART =  1;
const int DUMP_MODE_FREE  =  2;

typedef struct {
    int  iPrev;
    int  iNext;
} L2Node;

class L2List {
public:
    L2List(int iSize);
    L2List(const L2List *pL2L);

    ~L2List();

    void clear();
    //    int reserveSpace(uint iNum);
    int reserveSpace2(uint iNum);
    int getNumEndFree();

    // element management
    int addElement();
    int removeElement(int iEl);
    
    // list traversal
    int getFirstIndex(uchar uState) const { return m_iFirst[uState];};
    int getLastIndex(uchar uState) const { return m_iLast[uState];};
    int getNext(int iCur) const { return m_alList[iCur].iNext;};

    int countOfState(uchar uState);

    // for defragmentation
    uint collectFragInfo(uint iSize, uint *piHoles, uint *piActive);
    int defragment(uint iSize, uint *piActive);
    int setState(uchar uState, uint iNum);

    int getBufSize(int iDumpMode);
    uchar *serialize(uchar *pBuf);
    uchar *deserialize(uchar *pBuf);
    
    // for debugging
    int checkList() const;
    int calcHolyness(uchar uState, float *pfRatio) const;
    void display(uchar uState) const;
    void ddisplay(uchar uState) const;
    void displayArray(int iFirst, int iLast) const;
    bool hasState(int iState, int iIndex);
    int copy(const L2List *pL2L);
    int compare(const L2List *pL2L);

    int countActiveRegions() const;
    int *collectActiveBorders(int iCount) const;
    int createActiveRegions(int *pBorders, int iCount);
    

private:
    int unlink(uchar uState, int iEL);
    int linkAfter(uchar uState, int iEL, int iTarget);
    int findNewPrev(int iEl) const;

    int removeLastElement();

    int findInListForward(uchar uState, uint iNum) const;
    int findInListBackward(uchar uState, uint iNum) const;

    
    uchar *serializeFlat(uchar *pBuf);
    uchar *serializeSmart(uchar *pBuf);

    uchar *deserializeFlat(uchar *pBuf);
    uchar *deserializeSmart(uchar *pBuf);

    int m_iSize;
    L2Node *m_alList;
    int m_iFirst[2];
    int m_iLast[2];
 
    int m_iCurHole;
    int m_iCurActive;

    int m_iSerType;
    int m_iNumRegions;
};

#endif
