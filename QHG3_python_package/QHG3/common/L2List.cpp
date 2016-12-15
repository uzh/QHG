#include <stdio.h>
#include <string.h>

#include "L2List.h"
/**************************************************************************
 * L2List
 *
 * Object to keep track of used and unused elements in an array,
 * and provide fast access to unused indexes.
 *
 * Internally this is managed by two doubly linked lists of indexes,
 * "ACTIVE" and "PASSIVE". The nodes of both lists are  elements of 
 * a single array.
 * When a new index is used, the corresponding node is moved from 
 * the PASSIVE list to the ACTIVE list. If an index is not needed 
 * anymore, the corresponding node is moved from the "ACTIVE"
 * list to the "PASSIVE" list.
 * 
 * It is the users responsability to make sure that 
 * no element is deleted twice, as this corrupts the linkage 
 *
 ************************************************************************/


//----------------------------------------------------------------------------
// constructor
//
L2List::L2List(int iSize) 
    : m_iSize(iSize),
      m_alList(NULL),
      m_iCurHole(NIL),
      m_iCurActive(NIL) {
 
    m_alList = new L2Node[m_iSize];
    clear();
}


//----------------------------------------------------------------------------
// destructor
//
L2List::~L2List() {
    if (m_alList != NULL) {
        delete[] m_alList;
    }
}

//----------------------------------------------------------------------------
// clear
//
void L2List::clear() {
    m_alList[0].iPrev = NIL;
    if (m_iSize > 1) {
        m_alList[0].iNext = 1;
    } else {
        m_alList[0].iNext = NIL;
    }
 
    for (int i = 1; i < m_iSize-1; i++) {
        m_alList[i].iPrev = i-1;
        m_alList[i].iNext = i+1;
     }
            
    if (m_iSize > 1) {
        m_alList[m_iSize-1].iPrev = m_iSize-2;
    } else {
        m_alList[m_iSize-1].iPrev = NIL;
    }
    m_alList[m_iSize-1].iNext = NIL;
 
    m_iFirst[ACTIVE]  = NIL;
    m_iLast[ACTIVE]   = NIL;
    m_iFirst[PASSIVE] = 0;
    m_iLast[PASSIVE]  = m_iSize-1;
}

//----------------------------------------------------------------------------
// reserveSpace
//   Assumption: the list is defragmented!!
//   Assumption: iNum is less or equal to the number of PASSIVE elements!!
//
//   places iNum elements after the last active to active state
//   return index of first reserved element
//
int L2List::reserveSpace(uint iNum) {
    int iStart = NIL;
    if (m_iFirst[PASSIVE] != NIL) {
        iStart = m_iFirst[PASSIVE];
        // link the first element
        m_alList[iStart].iPrev = m_iLast[ACTIVE];
        if (m_iLast[ACTIVE] != NIL) {
            m_alList[m_iLast[ACTIVE]].iNext = iStart;
        } else {
            // no actives at all
            m_iFirst[ACTIVE] = iStart;
        }
        int iCur = iStart;
        for (uint i = 0; i < iNum-1; i++) {
            m_alList[iCur].iNext = iCur+1;
            ++iCur;
            m_alList[iCur].iPrev = iCur-1;
        }
        // do the last
        m_alList[iCur].iNext = NIL;
        m_iLast[ACTIVE] = iCur;
        if (iCur<m_iSize-1) {
            m_iFirst[PASSIVE] = iCur+1;
            m_alList[m_iFirst[PASSIVE]].iPrev = NIL;
            m_iLast[PASSIVE] = m_iSize-1;
        } else {
            m_iFirst[PASSIVE] = NIL;
            m_iLast[PASSIVE] = NIL;
        }
    }
    return iStart;
}

//----------------------------------------------------------------------------
// reserveSpace2
//   Assumption: there are at least iNum consecutive elements after last active
//
//   places iNum elements after the last active to active state
//   return index of first reserved element
//
int L2List::reserveSpace2(uint iNum) {
    int iStart = -1;

    if ((m_iLast[ACTIVE] == NIL) || (m_iLast[ACTIVE]+(int)iNum) < m_iSize) {
        // start postion of block
        if (m_iLast[ACTIVE] == NIL) {
            iStart = m_iFirst[PASSIVE];
        } else {
            iStart = m_iLast[ACTIVE]+1;
        }
        // last position of block
        int iEnd = iStart+iNum-1;

        
        if ((iStart == 0) && (iEnd == m_iSize-1)) {
            // block is layer: no linking required
            m_iFirst[ACTIVE]  = iStart;
            m_iLast[ACTIVE]   = iEnd;
            m_iFirst[PASSIVE] = NIL;
            m_iLast[PASSIVE]  = NIL;
        
        } else {

            int iPrevActive = m_iLast[ACTIVE];
            // the passive element previous to the block begin
            int iPrevPassive = m_alList[iStart].iPrev;
            int iPostPassive = (iEnd < m_iSize-1)?iEnd+1:NIL;

                    
            // link block into active
            m_iLast[ACTIVE] = iEnd;
            m_alList[iEnd].iNext = NIL;
            m_alList[iStart].iPrev = iPrevActive;
            if (iPrevActive != NIL) {
                m_alList[iPrevActive].iNext = iStart;
            } else {
                m_iFirst[ACTIVE] = iStart;
            }
        
            
            // unlink block from passive
            if (iPrevPassive != NIL) {
                m_alList[iPrevPassive].iNext = iPostPassive;
            } else {
                m_iFirst[PASSIVE] = iPostPassive;
            }
            if (iPostPassive != NIL) {
                m_alList[iPostPassive].iPrev = iPrevPassive;
            } else {
                m_iLast[PASSIVE] = iPrevPassive;
            }

        }
    
    }
    return iStart;
}


//----------------------------------------------------------------------------
// getNumEndFree
//   get size of largest passive block touching the layer End
//   which is iLayerSize-1-lastActive
//
int L2List::getNumEndFree() {
    int iSize = -1;
    if (m_iLast[ACTIVE] != NIL) {
        iSize = m_iSize - m_iLast[ACTIVE] - 1;
    } else {
        iSize = m_iSize;
    }
    return iSize;
}


//----------------------------------------------------------------------------
// unlink
//   connect node's previous and next with each other
//   taking care of special cases (first or last).
//   The node's links are not changed
//
int L2List::unlink(uchar uState, int iE) {
    if ((iE < 0) || (iE > m_iSize)) {
        printf("unlink: iE is %d!!!!!!!!!!!!!!\n", iE);
    }
    int iP = m_alList[iE].iPrev; 
    int iN = m_alList[iE].iNext; 
 
    if (iP != NIL) {
        // link previous to next
        m_alList[iP].iNext = iN;
    } else {
        // node was first in list
        m_iFirst[uState] =  iN;
    }

    if (iN != NIL) {
        // link next to previous
        m_alList[iN].iPrev = iP;
    } else {
        // node was last in list
        m_iLast[uState] =  iP;
    }
    return iE;
}

//----------------------------------------------------------------------------
// linkAfter
//   link iE after index iTarget taking care of special cases
//
int L2List::linkAfter(uchar uState, int iE, int iTarget) {

    // iE's previous will always be iTarget (even if NIL)
    m_alList[iE].iPrev = iTarget;

    if (iTarget != NIL) {
        //  "normal" insertion 
        int iN = m_alList[iTarget].iNext; 
        if (iN != NIL) {
            // link iE to target's successor
            m_alList[iN].iPrev = iE;
        } else {
            // target was the last; now iE is the last
            m_iLast[uState] = iE;
        }
        // link target to iE
        m_alList[iTarget].iNext = iE; 
        // link iE to target's successor
        m_alList[iE].iNext = iN;

    } else {
        // target is NIL: insert iE in front

        // this node becomes the old first node's previous (if it existed)
        if (m_iFirst[uState] != NIL) {
            m_alList[m_iFirst[uState]].iPrev = iE;
        }
        // the previous first node becomes this node's nexxt
        m_alList[iE].iNext = m_iFirst[uState];

        // this node becomes the first in the list
        m_iFirst[uState] = iE;
        if (m_iLast[uState] == NIL) {
            m_iLast[uState] = iE;
        }

    }

    return iE;
}

//----------------------------------------------------------------------------
// findNewPrev
//  find closest previous index with other state.
//  if the distance between a node E and its previous
//  node is greater than one, the node immediately before E
//  is of the other list.
//
int L2List::findNewPrev(int iE) const {
    // find a jump > 1
    int iP = m_alList[iE].iPrev;
    while ((iP != NIL) && (iP == iE -1)) {
        iE = iP;
        if ((iE < 0) || (iE > m_iSize)) printf("FindNewPrev: iE is %d!!!!!!!!!!!!!!\n", iE);
        iP = m_alList[iE].iPrev;
    }

    // we found a large jump or reached the beginning of the list
    if (/*(iP != NIL) ||*/ (iE > 0)) {
        // the node immediately before iE is of the other state
        iP = iE-1;
    }
    return iP;
}

//----------------------------------------------------------------------------
// removeElement
//   - unlink from ACTIVE list
//   - find new previous in PASSIVE list
//   - link after new previous in PASSIVE list
//
int L2List::removeElement(int iE) {

    unlink(ACTIVE, iE);
    int iTarget = findNewPrev(iE);
    linkAfter(PASSIVE, iE, iTarget);

    return iTarget;
}

//----------------------------------------------------------------------------
// removeLastElement
//   remove last active element (must not be NULL)
//
int L2List::removeLastElement() {
    int iResult = NIL;
    iResult =  removeElement(m_iLast[ACTIVE]);
    return iResult;
}

//----------------------------------------------------------------------------
// addElement
//   if PASSIVE list not empty:
//   - unlink from PASSIVE list
//   - find new previous in ACTIVE list
//   - link after new previous in ACTIVE list$
//
int L2List::addElement() {

    int iE = m_iFirst[PASSIVE];
    
    if (iE != NIL) {
        // there is at least one free space: remove it from the passive list
        unlink(PASSIVE, iE);
        // find closest previous in ACTIVE list
        int iTarget = findNewPrev(iE);
        // link it
        linkAfter(ACTIVE, iE, iTarget);
        
    }
    return iE;
}



//----------------------------------------------------------------------------
// countOfState
//
int L2List::countOfState(uchar uState) {
    int iCount = 0;
    int iCur = m_iFirst[uState];
    while (iCur != NIL) {
        iCount++;
        iCur =m_alList[iCur].iNext;
    }
    return iCount;
}


//----------------------------------------------------------------------------
// collectFragInfo
//   get indexes of all holes which can be filled by trailing actives
//   example
//     0123456789012345678901234567890                              
//     xxoxooxxxxoxooooxoxxxoooxxooooo
//   piHoles:
//      2  4  5 10 12 13
//   piActive
//     35 34 30 29 28 16
//   
//  We start from searching for holes from low indexes up,
//  and searching for actives starting fromn high indexes down.
//  As soon as we run out of holes or actives, or if the arrays are full,
//  searching stops and we decide if the search has to continue or 
//  if the next search has to start from zero.
//  m_iCurHole and m_iCurActive are state variables for the search
//  m_iCurHole   == NIL : start hole search from first
//  m_iCurACTIVE == NIL : start active search from last
//
//  Returns  value
//   iSize     if all indexes fit exactly into arrays(iSize)
//   iSize+1   if arrays are filled, but there is more
//   > 0        number of found holes/actives; no more to be found
//   0         no holes or no actives: nothing to move
// 
uint L2List::collectFragInfo(uint iSize, uint *piHoles, uint *piActive) {
    uint iResult = 0;

    // start search from first or continue where we stopped last time?
    if (m_iCurHole == NIL) {
        m_iCurHole = m_iFirst[PASSIVE];
    }
    // start search from last or continue where we stopped last time?
    if (m_iCurActive == NIL) {
        m_iCurActive = m_iLast[ACTIVE];
    }

    // search holes/actives
    uint iC = 0;

    // we continue searching as long as the array is not full and none of the searches reached the end (NIL)
    // and the hole index is smaller tha the active index 
    while ((iC < iSize) && (m_iCurHole < m_iCurActive) && (m_iCurHole != NIL) && (m_iCurActive != NIL)) {
        // add current indexes to array
        piHoles[iC]  = (uint) m_iCurHole;
        piActive[iC] = (uint) m_iCurActive;

        iC++;
        // move on
        m_iCurHole   = m_alList[m_iCurHole].iNext;
        m_iCurActive = m_alList[m_iCurActive].iPrev;
    }

    // searching is over: decide on return value
    if (iC == 0) {
        // nothing saved in the array
        if ((m_iCurHole == NIL)  && (m_iCurActive != NIL)) {
            // no holes - layer is completely filled
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;

        } else if ((m_iCurHole != NIL)  && (m_iCurActive == NIL)) {
            // only holes - layer is completely empty
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        } else if ((m_iCurHole != NIL)  && (m_iCurActive != NIL)) {
            // probably: m_iCurHole > m_iCurActive
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        }
    
    } else {
        // "normal" case
        if (iC < iSize) {
            // everything fit into arrays

            iResult = iC;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        } else {
            // if array is full, there are two possibilities
            if  (m_iCurHole < m_iCurActive) {
                // there is more
                iResult = iC+1;
            } else {
                // everything fit into array perfectly (nothing more to find)
                iResult = iC;
                // start new collection next time
                m_iCurHole   = NIL;
                m_iCurActive = NIL;
            }
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
// defragment
//   move all actives to frontal position
//   With the data from collectFragInfo() this 
//     xxoxooxxxxoxooooxoxxxoooxxooooo
//   results in this pattern:
//     xxxxxxxxxxxxxxooooooooooooooooo
//
int L2List::defragment(uint iSize, uint *piActive) {
 
    for (uint i = 0; i < iSize; i++) {
        removeElement(piActive[i]);
        addElement(); // will fill lowest hole
    }
    return 0;
}


//----------------------------------------------------------------------------
// setState
//   sets the first PASSIVE indexes to ACTIVE, or
//   sets the last ACTIVE indexes to PASSIVE
//
int L2List::setState(uchar uState, uint iNum) {
    if (uState == ACTIVE) {
        for (uint  i = 0; i < iNum; i++) {
            addElement();
        }
    } else {
        for (uint  i = 0; i < iNum; i++) {
            removeLastElement();
        }
    }
    return 0;
}

//----------------------------------------------------------------------------
// findInListForward
//
int L2List::findInListForward(uchar uState, uint iNum) const {
    int iResult = 0;
    int k = m_iFirst[uState];
    for (int i = 0; (i < m_iSize) && (k != NIL) && (k != (int) iNum) ; i++) {
        k = m_alList[k].iNext;
    }
    if (k != (int) iNum) {
        //printf("%d is not contained in %s forward list\n", iNum, uState?"ACTIVE":"PASSIVE");
        iResult = -1;
        
    } else {
    }
    return iResult;
}

//----------------------------------------------------------------------------
// findInListForward
//
int L2List::findInListBackward(uchar uState, uint iNum) const {
    int iResult = 0;
    int k = m_iLast[uState];
    for (int i = 0; (i < m_iSize) && (k != NIL) && (k != (int)iNum); i++) {
        k = m_alList[k].iPrev;
    }
    if (k != (int)iNum) {
        //printf("%d is not contained in %s backward list\n", iNum, uState?"ACTIVE":"PASSIVE");
        iResult = -1;
    } else {
    }
    return iResult;
}


const uint BAD_FIRST               = 0x00000002;
const uint BAD_LAST                = 0x00000004;
const uint BAD_FIRST_VAL_PASSIVE   = 0x00000010;
const uint BAD_FIRST_VAL_ACTIVE    = 0x00000018;
const uint BAD_LAST_VAL_PASSIVE    = 0x00000040;
const uint BAD_LAST_VAL_ACTIVE     = 0x00000060;
const uint BAD_FIRST_PREV_PASSIVE  = 0x00000100;
const uint BAD_FIRST_PREV_ACTIVE   = 0x00000180;
const uint BAD_LAST_NEXT_PASSIVE   = 0x00000400;
const uint BAD_LAST_NEXT_ACTIVE    = 0x00000600;
const uint BAD_PREV_VAL            = 0x00000800;
const uint BAD_NEXT_VAL            = 0x00001000;
const uint BAD_BALANCE             = 0x00002000;
const uint BAD_FORWARD_PASSIVE     = 0x00010000;
const uint BAD_FORWARD_ACTIVE      = 0x00020000;
const uint BAD_BACKWARD_PASSIVE    = 0x00040000;
const uint BAD_BACKWARD_ACTIVE     = 0x00080000;

//----------------------------------------------------------------------------
// checkList
//   check integrity of list:
//     either first[ACTIVE] or first[PASSIVE] must be 0
//     either last[ACTIVE] or last[PASSIVE] must be size-1
//     last[X].next = NIL and first[X] = NIL
//     prev and next values must be in [-1, size-1]
//     every index must appear once as prev and once as next
//
int L2List::checkList() const {
    int iResult = 0;
    if ((m_iFirst[PASSIVE] != 0) && (m_iFirst[ACTIVE] != 0)) {
        iResult |= BAD_FIRST;
        printf("BAD_FIRST\n");fflush(stdout);
    }
    if ((m_iLast[PASSIVE] != m_iSize-1) && (m_iLast[ACTIVE] != m_iSize-1)) {
        iResult |= BAD_LAST;
        printf("BAD_LAST\n");fflush(stdout);
    }
    if (m_iFirst[PASSIVE] != NIL) {
        if (m_alList[m_iFirst[PASSIVE]].iPrev != NIL) {
            iResult |= BAD_FIRST_PREV_PASSIVE;
            printf("BAD_FIRST_PREV_PASSIVE: %d\n", m_alList[m_iFirst[PASSIVE]].iPrev);fflush(stdout);
        }
    }
    if (m_iFirst[ACTIVE] != NIL) {
        if (m_alList[m_iFirst[ACTIVE]].iPrev != NIL) {
            iResult |= BAD_FIRST_PREV_ACTIVE;
            printf("BAD_FIRST_PREV_ACTIVE: %d\n", m_alList[m_iFirst[ACTIVE]].iPrev);fflush(stdout);
        }
    }
    if (m_iLast[PASSIVE] != NIL) {
        if (m_alList[m_iLast[PASSIVE]].iNext != NIL) {
            iResult |= BAD_LAST_NEXT_PASSIVE;
            printf("BAD_LAST_NEXT_PASSIVE: %d\n", m_alList[m_iLast[PASSIVE]].iNext);fflush(stdout);
        }
    }
    if (m_iLast[ACTIVE] != NIL) {
        if (m_alList[m_iLast[ACTIVE]].iNext != NIL) {
            iResult |= BAD_LAST_NEXT_ACTIVE;
            printf("BAD_LAST_NEXT_ACTIVE: %d\n", m_alList[m_iLast[ACTIVE]].iNext);fflush(stdout);
        }
    }
    
    int *aIndexes = new int[m_iSize];
    memset(aIndexes, 0, m_iSize*sizeof(int));
    for (int i = 0; i < m_iSize;i++) {
        if ((m_alList[i].iPrev < -1) || 
            (m_alList[i].iPrev >= m_iSize) || 
            (m_alList[i].iPrev == i)) {
            iResult |= BAD_PREV_VAL;
            printf("BAD_PREV_VAL %d at %d\n", m_alList[i].iPrev, i);fflush(stdout);
        } else {
            if (m_alList[i].iPrev >= 0) {
                aIndexes[m_alList[i].iPrev]++;
            }
        }
        if ((m_alList[i].iNext < -1) || 
            (m_alList[i].iNext >= m_iSize) || 
            (m_alList[i].iNext == i)) {
            iResult |= BAD_NEXT_VAL;
            printf("BAD_NEXT_VAL %d at %d\n", m_alList[i].iNext, i);fflush(stdout);
        } else {
            if (m_alList[i].iNext >= 0) {
                aIndexes[m_alList[i].iNext]++;
            }
        }
    }
    
    if (m_iFirst[PASSIVE] != NIL) {
        aIndexes[m_iFirst[PASSIVE]]++;
    }
    if (m_iFirst[ACTIVE] != NIL) {
        aIndexes[m_iFirst[ACTIVE]]++;
    }
    if (m_iLast[PASSIVE] != NIL) {
        aIndexes[m_iLast[PASSIVE]]++;
    }
    if (m_iLast[ACTIVE] != NIL) {
        aIndexes[m_iLast[ACTIVE]]++;
    }


    for (int i = 0; i < m_iSize;i++) {
        if (aIndexes[i] != 2) {
            printf("BAD_BALANCE at %d: %d\n", i, aIndexes[i]);fflush(stdout);
            iResult |= BAD_BALANCE;
        }
    }

    int iResult2 = findInListForward(PASSIVE, m_iLast[PASSIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_FORWARD_PASSIVE;
    }

    iResult2 = findInListForward(ACTIVE,  m_iLast[ACTIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_FORWARD_ACTIVE;
    }

    iResult2 = findInListBackward(PASSIVE,  m_iFirst[PASSIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_BACKWARD_PASSIVE;
    }

    iResult2 = findInListBackward(ACTIVE,  m_iFirst[ACTIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_BACKWARD_ACTIVE;
    }

    /*
    int k  = -1;
    int k2 = -1;
    k = m_iFirst[PASSIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iNext;
    }
    if (k == NIL) {
        if (k2 != m_iLast[PASSIVE]) {
            printf("end of passive list (%d) does not match m_iLast[PASSIVE] (%d)\n", k2, m_iLast[PASSIVE]);
            iResult |= BAD_FORWARD_PASSIVE;
        }
    } else {
        printf("unfinished forward passive list\n");
        iResult |= BAD_FORWARD_PASSIVE;
    }

    k = m_iFirst[ACTIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iNext;
    }
    if (k == NIL) {
        if (k2 != m_iLast[ACTIVE]) {
            printf("end of active list (%d) does not match m_iLast[ACTIVE] (%d)\n", k2, m_iLast[ACTIVE]);
            iResult |= BAD_FORWARD_ACTIVE;
        }
    } else {
        printf("unfinished forward active list\n");
        iResult |= BAD_FORWARD_ACTIVE;
    }

    k = m_iLast[PASSIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iPrev;
    }
    if (k == NIL) {
        if (k2 != m_iFirst[PASSIVE]) {
            printf("beginning of passive list (%d) does not match m_iFirst[PASSIVE] (%d)\n", k2, m_iFirst[PASSIVE]);
            iResult |= BAD_FORWARD_PASSIVE;
        }
    } else {
        printf("unfinished backward passive list\n");
        iResult |= BAD_FORWARD_PASSIVE;
    }

    k = m_iLast[ACTIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iPrev;
    }
    if (k == NIL) {
        if (k2 != m_iFirst[ACTIVE]) {
            printf("beginning of active list (%d) does not match m_iFirst[ACTIVE] (%d)\n", k2, m_iFirst[ACTIVE]);
            iResult |= BAD_FORWARD_ACTIVE;
        }
    } else {
        printf("unfinished backward active list\n");
        iResult |= BAD_FORWARD_ACTIVE;
    }
    */

    delete[] aIndexes;
    return iResult;
}


//----------------------------------------------------------------------------
// display
//
void L2List::display(uchar uState) const {
    printf("[%d][%d]|", m_iFirst[uState], m_iLast[uState]);
    int i = m_iFirst[uState];
    bool bGoOn = true;
    while (bGoOn && (i >= 0) && (i < m_iSize)) {
        
        printf("%d|",i);
        i = m_alList[i].iNext;
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// ddisplay
//
void L2List::ddisplay(uchar uState) const {
    printf("[%d][%d]| ", m_iFirst[uState], m_iLast[uState]);
    int i = m_iFirst[uState];
    bool bGoOn = true;
    while (bGoOn && (i >= 0) && (i < m_iSize)) {
        char sN[5];
        if (m_alList[i].iNext < 0) {
            strcpy(sN, "*");
        } else {
            sprintf(sN, "%d", m_alList[i].iNext);
        }
        char sP[5];
        if (m_alList[i].iPrev < 0) {
            strcpy(sP, "*");
        } else {
            sprintf(sP, "%d", m_alList[i].iPrev);
        }

        printf("%s:(%d):%s | ", sP, i, sN);
        i = m_alList[i].iNext;
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// displayArray
//
void L2List::displayArray(int iFirst,int iLast) const {
    if (iFirst == NIL) {
        iFirst = 0;
    }
    if (iLast == NIL) {
        iLast = m_iSize-1;
    }

    int i = iFirst;    
    printf("FP[%d]LP[%d]|", m_iFirst[PASSIVE], m_iLast[PASSIVE]);
    printf("FA[%d]LA[%d]|", m_iFirst[ACTIVE], m_iLast[ACTIVE]);

    while  (i < m_iSize) {
       char sN[5];
        if (m_alList[i].iNext < 0) {
            strcpy(sN, "*");
        } else {
            sprintf(sN, "%d", m_alList[i].iNext);
        }
        char sP[5];
        if (m_alList[i].iPrev < 0) {
            strcpy(sP, "*");
        } else {
            sprintf(sP, "%d", m_alList[i].iPrev);
        }

        printf("%s:(%d):%s | ",sP, i, sN);

        i++;
    }
    printf("\n");
}
