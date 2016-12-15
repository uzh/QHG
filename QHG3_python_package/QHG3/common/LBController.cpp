#include <stdio.h>
#include <algorithm>

#include "types.h"

#include "L2List.h"
#include "LBBase.h"
#include "LBController.h"

const int COMPACT_BUFFER_SIZE = 16284;

//----------------------------------------------------------------------------
// constructor
//
LBController::LBController(uint iLayerSize)
    : m_iLayerSize(iLayerSize),
      m_iNumUsed(0) {

    init(iLayerSize);
}

//----------------------------------------------------------------------------
// constructor
//
LBController::LBController()
    : m_iLayerSize(0),
      m_iNumUsed(0) {

}

//----------------------------------------------------------------------------
// init
//
int LBController::init(int iLayerSize) {
    m_iLayerSize = iLayerSize;

    return 0;
}

//----------------------------------------------------------------------------
// destructor
//
LBController::~LBController() {
    for (uint i = 0; i < m_vpL2L.size(); i++) {
        delete m_vpL2L[i];
    }
}

//----------------------------------------------------------------------------
// clear
//
void LBController::clear() {
    for (uint i = 0; i < m_vpL2L.size(); i++) {
        m_vpL2L[i]->clear();
    }
    m_iNumUsed = 0;
}

//----------------------------------------------------------------------------
// reserveSpace
//
uint LBController::reserveSpace(uint iNum) {
    // defragment all
    compactData();
    
    // default: we'll add to the last layer

    uint iUsed      = getNumUsed();
    uint iCurLayer  = iUsed/m_iLayerSize;
    
    uint iAvailable = getNumFree();
    
    // if we need more space than is available, 
    // add as many layers as needed
    if (iAvailable < iNum) {
        uint iNumRequiredLayers = 1+(iNum-iAvailable-1)/m_iLayerSize;
        for (uint i = 0; i < iNumRequiredLayers; i++) {
            addLayer();
        }
        if (iAvailable == 0) {
            // start on a new layer
            iAvailable = m_iLayerSize;
        }
    }
    // if there is space in the current layer, reserve this
    // Available > 0 
    //   Available >= iNum -> reserve iNum; done
    //   |XX-----| + YY -> |XXYY---|
    //   Available < iNum  -> reserve available; continue with rest
    //   |XX-----| + YYYYYYY -> |XXYYY--| + YY

    // calculate new number of used spaces before changing iNum
    m_iNumUsed += iNum;
    
    uint iOffset = m_iLayerSize*iCurLayer;
    uint iStartIndex = 0;

    uint iLoad = (iNum > iAvailable)?iAvailable:iNum;
    while (iNum > 0) {
        // we know reserveSpace will return positive value
        uint iTemp = m_vpL2L[iCurLayer++]->reserveSpace(iLoad);
        if (iStartIndex == 0) {
            iStartIndex = iTemp;
        }
        iNum -= iLoad;
        iLoad = (iNum > m_iLayerSize)?m_iLayerSize:iNum;
    }
    return iStartIndex+iOffset;
}

//----------------------------------------------------------------------------
// reserveSpace2
//
uint LBController::reserveSpace2(uint iNum) {
    // default: we'll add to the last layer
    int iCurLayer = m_vpL2L.size()-1;
    while ((iCurLayer >= 0) && (m_vpL2L[iCurLayer]->countOfState(ACTIVE) == 0)) {
        iCurLayer--;
    }
    
    uint iAvailable = m_vpL2L[iCurLayer]->getNumEndFree();
    // if we need more space than is available, 
    // add as many layers as needed
    if (iAvailable < iNum) {
        uint iNumRequiredLayers = 1+(iNum-iAvailable-1)/m_iLayerSize;
        for (uint i = 0; i < iNumRequiredLayers; i++) {
            addLayer();
        }
        if (iAvailable == 0) {
            // start on a new layer
            iAvailable = m_iLayerSize;
            iCurLayer++;
        }
    }
    // if there is space in the current layer, reserve this
    // Available > 0 
    //   Available >= iNum -> reserve iNum; done
    //   |XX-----| + YY -> |XXYY---|
    //   Available < iNum  -> reserve available; continue with rest
    //   |XX-----| + YYYYYYY -> |XXYYY--| + YY

    // calculate new number of used spaces before changing iNum
    m_iNumUsed += iNum;
    
    uint iOffset = m_iLayerSize*iCurLayer;
    int iStartIndex = -1;

    uint iLoad = (iNum > iAvailable)?iAvailable:iNum;
    while (iNum > 0) {
        // we know reserveSpace will return positive value
        uint iTemp = m_vpL2L[iCurLayer++]->reserveSpace2(iLoad);
        if (iStartIndex < 0) {
            iStartIndex = iTemp;
        }
        iNum -= iLoad;
        iLoad = (iNum > m_iLayerSize)?m_iLayerSize:iNum;
    }

    return iStartIndex+iOffset;
}


//----------------------------------------------------------------------------
// addBuffer
//
int LBController::addBuffer(LBBase *pLB) {
    int iResult = -1;
    if (pLB->getLayerSize() == m_iLayerSize) {
        // the first LayerBuf determines the iitial size of the list
        if ((pLB->getNumLayers() == m_vpL2L.size()) || (m_vpL2L.size() == 0)) {
            if (m_vpL2L.size() == 0) {
                while  (m_vpL2L.size() < pLB->getNumLayers()) {
                    m_vpL2L.push_back(new L2List(m_iLayerSize));
                }
            }
            m_vpLB.push_back(pLB);
            iResult = 0;
        } else {
            // number of layers doesn't match
            iResult = -3;
        }
    } else {
        // blocksize doesn't match
        iResult = -2;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// removeBuffer
//
int LBController::removeBuffer(LBBase *pLB) {
    int iResult = -1;
    std::vector<LBBase *>::iterator it;
    it = std::find(m_vpLB.begin(), m_vpLB.end(), pLB);
    if (it != m_vpLB.end()) {
        m_vpLB.erase(it);
        iResult = 0;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// addLayer
//  add a MemBlock layer
//
int LBController::addLayer() {
    m_vpL2L.push_back(new L2List(m_iLayerSize));
    //    printf("create a layer iin all %zd bufs\n",  m_vpLB.size());
    for (uint i = 0; i < m_vpLB.size(); i++) {
        m_vpLB[i]->createLayer();
    }
    return 0;
}

//----------------------------------------------------------------------------
// removeLayer
//  remove a MemBlock layer
//
int LBController::removeLayer(uint iLayer) {
    m_vpL2L.erase(m_vpL2L.begin()+iLayer);
    delete m_vpL2L[iLayer];

    for (uint i = 0; i < m_vpLB.size(); i++) {
        m_vpLB[i]->freeLayer(iLayer);
    }
    return 0;
}


//----------------------------------------------------------------------------
// getFreeIndex
//  find next free space in agent array and mark as used
//
uint LBController::getFreeIndex() {
    uint iIndex = 0;
    bool bSearching = true;
    for (uint i = 0; bSearching && (i < m_vpL2L.size()); i++) {
        int i0 = m_vpL2L[i]->addElement();
        if (i0 >= 0) {
            iIndex = i*m_iLayerSize+i0;
            bSearching = false;
        }
    }
    
    // nothing found: new layer
    if (bSearching) {
        addLayer();
        int i0 = m_vpL2L.back()->addElement();
        iIndex = (uint)((m_vpL2L.size()-1)*m_iLayerSize + i0);
    }

    m_iNumUsed++;
    return iIndex;
}

//----------------------------------------------------------------------------
// deleteElement
//   delete agent at specified indes
//
int LBController::deleteElement(uint lIndex) {
    int iResult = -1;
    uint iLayer = lIndex/m_iLayerSize;
    uint iIndex = lIndex%m_iLayerSize;
    
    if (iLayer < m_vpL2L.size()) {
        m_vpL2L[iLayer]->removeElement(iIndex);
        iResult = 0;
        m_iNumUsed--;
    } else {
        // bad lIndex (too big)
        iResult = -2;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// compactLayers
//  fill up holes in each layer by moving data at highest indices to holes
//
int LBController::compactLayers() {
    uint iN = COMPACT_BUFFER_SIZE;
    uint *piHoles   = new uint[iN];
    uint *piActives = new uint[iN];

    int iOffset = 0;
    for (uint i = 0; i < m_vpL2L.size(); i++) {

        // get array of holes and used indexes in this layer
        uint iRes = m_vpL2L[i]->collectFragInfo(iN, piHoles, piActives);

        // we might have to do this repeatedly if our array is too small 
        //  (see return codes of collectFragInfo())
        while (iRes > 0) {
            uint iNum = iRes;
            if (iNum > iN) {
                // iN+1 is returned if array is full, but more elements are waiting
                iNum = iN;
            }
 
            m_vpL2L[i]->defragment(iNum, piActives);

            // shift around data elements accordingly
            for (uint j = 0; j < iNum; j++) {
                for (uint k = 0; k < m_vpLB.size(); k++) {
                    m_vpLB[k]->elementShift(piHoles[j]+iOffset, piActives[j]+iOffset);
                }
            }        
            
            // do we have to go again?
            if (iRes > iN) {
                // yes
                iRes = m_vpL2L[i]->collectFragInfo(iN, piHoles, piActives);
            } else {
                // no, we're done
                iRes = 0;
            }
        }
        // continue in next layer: adjust offset
        iOffset += m_iLayerSize;

    }

    delete[] piHoles;
    delete[] piActives;

    return 0;
}

//----------------------------------------------------------------------------
// compactData
//  fill holes in a lower layer with agents from a higher layer
//  
int LBController::compactData() {
    // all layers must be compactified first
    int iResult = compactLayers();

    //    printf("inter layer shifting\n");
    if ((iResult == 0) && (m_vpL2L.size() > 0)) {
        uint iFF = 0;
        uint iFFF = 0;
      
        uint iLU  = (uint)m_vpL2L.size()-1;
        uint iLUU = 0;
        
        // find first layer with hole
        while ((iFF < m_vpL2L.size()) && (iFFF == 0)) {
            iFFF =  m_vpL2L[iFF]->countOfState(PASSIVE);
            if (iFFF == 0) {
                iFF++;
            }
        }

        // if we have no holes, we are done
        if (iFF < m_vpL2L.size()) {
            // find last layer with used indices
            while ((iLU > iFF) && (iLUU == 0)) {
                iLUU =  m_vpL2L[iLU]->countOfState(ACTIVE);
                if (iLUU == 0) {
                    iLU--;
                }
            }

            // we only exchange between different layers
            // as long as  there are "low" holes and "high" agents
            while (iFF < iLU) {
                uint N = 0;
                
                // move as many objects as possible to the holes 
                if (iLUU > iFFF) {
                    // can only move part of objects
                    N = iFFF;
                } else {
                    // can move all objects
                    N = iLUU;
                }
                
                // auxiliary variable
                uint iFFU = m_iLayerSize - iFFF;
                // make appropriate changes in the list:
                m_vpL2L[iFF]->setState(ACTIVE, N);
                m_vpL2L[iLU]->setState(PASSIVE, N);
                // do the actual moves for each MemBlock object
                // (this requires the layers to be compacted beforehand)
                for (uint i = 0; i < m_vpLB.size(); i++) {
                    m_vpLB[i]->moveElements(iFF, iFFU, iLU, iLUU-N, N);
                }

                // adjust number of holes in low layer 
                iFFF -= N;
                // adjust number of used indices in high layer 
                iLUU -= N;
                
                // find next layer with holes
                while ((iFF < iLU) && (iFFF == 0)) {
                    iFFF =  m_vpL2L[iFF]->countOfState(PASSIVE);
                    if (iFFF == 0) {
                        // no holes: try next layer
                        iFF++;
                    }
                }
                               
                // find next layer with used indices
                while ((iLU > iFF) && (iLUU == 0)) {
                    iLUU =  m_vpL2L[iLU]->countOfState(ACTIVE);
                    if (iLUU == 0) {
                        // no objects: try next layer
                        iLU--;
                    }
                }
            }
        }
    
        // remove unused layers
        std::vector<uint> vKill;	
        for (uint i = 0; i < (uint)m_vpL2L.size(); i++) {
            if (m_vpL2L[i]->countOfState(ACTIVE) == 0) {
				fprintf(stderr,"layer %u empty\n",i);
                vKill.push_back(i);
            }
        }
        for (int i = (int)vKill.size()-1; i >= 0; i--) {
			fprintf(stderr,"removing layer %u\n",i);
            removeLayer((uint)vKill[i]);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// getFirstIndex
//  iterator method 
//    returns the first index with given state (PASSIVE or ACTIVE)
//
int LBController::getFirstIndex(uchar uState) const { 
    int iFirst   = NIL;
    uint iLayer  = 0;
    uint iNumLayers = (uint)m_vpL2L.size();

    while ((iLayer < iNumLayers) && (iFirst == NIL)) {
        //    while ((iLayer < m_vpL2L.size()) && (iFirst == NIL)) {
        iFirst =  m_vpL2L[iLayer]->getFirstIndex(uState);
        if (iFirst == NIL) {
            iLayer++;
        }
    }
    if (iFirst != NIL) {
        iFirst += iLayer*m_iLayerSize;
    }
    return iFirst;
}

//----------------------------------------------------------------------------
// getLastIndex
//  iterator method 
//    returns the last index with given state (PASSIVE or ACTIVE)
//
int LBController::getLastIndex(uchar uState) const { 
    int iLast   = NIL;
    int iLayer  = (uint)m_vpL2L.size()-1;

    while ((iLayer >= 0) && (iLast == NIL)) {
        iLast =  m_vpL2L[iLayer]->getLastIndex(uState)+iLayer*m_iLayerSize;
        if (iLast == NIL) {
            iLayer--;
        }
    }
    return iLast;
}

//----------------------------------------------------------------------------
// getNextIndex
//  iterator method
//    returns the next index after iCur with given state (PASSIVE or ACTIVE)
//
int LBController::getNextIndex(uchar uState, int iCur) const {
    uint iLayer = iCur/m_iLayerSize;
    uint iIndex = iCur%m_iLayerSize;
    int iNext = m_vpL2L[iLayer]->getNext(iIndex);
    
    if (iNext == NIL) {
        iLayer++;
        while ((iLayer < m_vpL2L.size()) && (iNext == NIL)) {
            iNext =  m_vpL2L[iLayer]->getFirstIndex(uState);
            if (iNext == NIL) {
                iLayer++;
            }  
        }
    }
    if (iNext != NIL) {
        iNext += iLayer*m_iLayerSize;
    } 
    return iNext;
}


//----------------------------------------------------------------------------
// checkLists
//  check all Lists for consistency 
//
int LBController::checkLists() {
    int iResult = 0;
    for (uint i = 0; (iResult == 0) && (i < m_vpL2L.size()); i++) {
        iResult = m_vpL2L[i]->checkList();

        if (iResult != 0) {
            printf("  (errors in layer #%d\n", i);
            displayArray(i, 0, getLayerSize());
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// displayArray
//  display specified layer from index iFirst to indes iLast
//  (not following the  links)
//  (iFirst=NIL: from start, iLast=NIL: to end)
//
void LBController::displayArray(uint iLayer, int iFirst, int iLast) {
    if (iLayer < m_vpL2L.size()) {
        m_vpL2L[iLayer]->displayArray(iFirst, iLast);
    }
}

//----------------------------------------------------------------------------
// findInListsForward
//
int LBController::findInListsForward(uchar uState, uint iBigIndex) const {
    int iResult = -1;
    uint iLayer = iBigIndex/m_iLayerSize;
    uint iIndex = iBigIndex%m_iLayerSize;
    if (iLayer < m_vpL2L.size()) {
        iResult = m_vpL2L[iLayer]->findInListForward(uState, iIndex);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// findInListsBackward
//
int LBController::findInListsBackward(uchar uState, uint iBigIndex) const {
    int iResult = -1;
    uint iLayer = iBigIndex/m_iLayerSize;
    uint iIndex = iBigIndex%m_iLayerSize;
    if (iLayer < m_vpL2L.size()) {
        iResult = m_vpL2L[iLayer]->findInListForward(uState, iIndex);
    }
    return iResult;
}

