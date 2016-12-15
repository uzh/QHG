#ifndef __L2LIST_H__
#define __L2LIST_H__

#include "types.h" //  uint uchar...

const int NIL = -1;
const uchar PASSIVE = 0;
const uchar ACTIVE  = 1;

typedef struct {
    int  iPrev;
    int  iNext;
} L2Node;

class L2List {
public:
    L2List(int iSize);
    ~L2List();

    void clear();
    int reserveSpace(uint iNum);
    int reserveSpace2(uint iNum);
    int getNumEndFree();

    // element management
    int addElement();
    int removeElement(int iEl);
    int removeLastElement();
    
    // list traversal
    int getFirstIndex(uchar uState) const { return m_iFirst[uState];};
    int getLastIndex(uchar uState) const { return m_iLast[uState];};
    int getNext(int iCur) const { return m_alList[iCur].iNext;};

    int countOfState(uchar uState);

    // for defragmentation
    uint collectFragInfo(uint iSize, uint *piHoles, uint *piActive);
    int defragment(uint iSize, uint *piActive);
    int setState(uchar uState, uint iNum);

    // for debugging
    int checkList() const;
    void display(uchar uState) const;
    void ddisplay(uchar uState) const;
    void displayArray(int iFirst, int iLast) const;

    int findInListForward(uchar uState, uint iNum) const;
    int findInListBackward(uchar uState, uint iNum) const;
protected:
    int unlink(uchar uState, int iEL);
    int linkAfter(uchar uState, int iEL, int iTarget);
    int findNewPrev(int iEl) const;

    int m_iSize;
    L2Node *m_alList;
    int m_iFirst[2];
    int m_iLast[2];
 
    int m_iCurHole;
    int m_iCurActive;
};

#endif
