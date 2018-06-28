/*============================================================================
| LBController
|  
|  Uses an L2List to keep track of used and unused indexes in a collection of 
|  LBBase objects.
|  The LBBase objects must have the same layer size.
|  
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __LBCONTROLLER_H__
#define __LBCONTROLLER_H__

#include "types.h"
#include "L2List.h"

class LBBase;


class LBController {
public:
    LBController(uint iLayerSize);
    LBController();
    ~LBController();

    int init(int iLayerSize);

    int addBuffer(LBBase *pLB);
    int removeBuffer(LBBase *pLB);

    // layer managenent
    int addLayer();
    int removeLayer(uint iLayer);

    // element management
    uint getFreeIndex();
    int  deleteElement(uint lIndex);

    int compactData();
    void clear();
    uint reserveSpace2(uint iNum);

    // iterating
    int getFirstIndex(uchar uState) const;
    int getNextIndex(uchar uState, int iCur) const;
    int getLastIndex(uchar uState) const;

    // info
    uint getLayerSize() const {return m_iLayerSize;};
    uint getNumLayers() const {return (uint) m_vpL2L.size();};
    uint getNumUsed() const   {return m_iNumUsed;};
    uint getNumFree() const   {return (uint)m_vpL2L.size()*m_iLayerSize-m_iNumUsed;};

    // info for hdf5 
    uint getNumUsed(int i) const   {return m_vpL2L[i]->countOfState(ACTIVE);};
    uint getNumUnused(int i) const   {return m_vpL2L[i]->countOfState(PASSIVE);};

    // for debugging
    int checkLists(bool bDisplayArray=false);
    void calcHolyness();
    void displayArray(uint iLayer, int iFirst, int iLast);
    bool hasState(int iState, int iIndex);

    const L2List *getL2List(uint iLayer);
    int setL2List(const L2List *pL2List, uint iLayer);

    int    getBufSize(int iDumpMode);
    uchar *serialize(uchar *pBuf);
    int    deserialize(uchar *pBuf);
    
protected:
    uint m_iLayerSize;
    int compactLayers();

    uint m_iNumUsed;

    std::vector<L2List *> m_vpL2L;
    std::vector<LBBase *> m_vpLB;
};

#endif
