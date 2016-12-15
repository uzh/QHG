#ifndef __LAYERBUF_CPP__
#define __LAYERBUF_CPP__

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>

#include "LayerBuf.h"

//---------------------------------------------------------------------------
// constructor
//   - iBlockSize: size of memory blocks
//   - iStrategy: or-ed combination of MB_XXX constants controlling the
//                deletion behaviour
//
template<class T>
LayerBuf<T>::LayerBuf(uint iLayerSize, int iStrategy)
    : m_iLayerSize(iLayerSize),
      m_iNumLayers(0),
      m_iStrategy(MB_ZERO_BLOCKS | iStrategy) {

}

//---------------------------------------------------------------------------
// constructor
//   - iBlockSize: size of memory blocks
//   - iStrategy: or-ed combination of MB_XXX constants controlling the
//                deletion behaviour
//
template<class T>
LayerBuf<T>::LayerBuf()
    : m_iLayerSize(0),
      m_iNumLayers(0),
      m_iStrategy(MB_ZERO_BLOCKS) {


}



//---------------------------------------------------------------------------
// destructor
//
template<class T>
LayerBuf<T>::~LayerBuf() {

    // handle used blocks
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        if (m_vUsedLayers[i] != NULL) {
            // delete array
            delete[] m_vUsedLayers[i];
        }
    }

    // handle free blocks
    for (unsigned int i = 0; i < m_vFreeLayers.size(); i++) {
        if (m_vFreeLayers[i] != NULL) {
            // delete array
            delete[] m_vFreeLayers[i];
        }
    }
}

//---------------------------------------------------------------------------
// destructor
//
template<class T>
void LayerBuf<T>::init(uint iLayerSize, int iStrategy) {
    m_iLayerSize = iLayerSize;
    m_iStrategy = MB_ZERO_BLOCKS | iStrategy;
}

/*
//---------------------------------------------------------------------------
// operator[]
//  returns reference to i-th element (readable and writable)
//  if i is beyond the existing blocks, the required number of blocks
//  is added
//
template<class T>    
T &LayerBuf<T>::operator[](int i) {
    size_t iLayer = i / m_iLayerSize;
    while (iLayer >= m_vUsedLayers.size()) {
        createLayer();
        T* pLayer = m_vUsedLayers.back();
        memset(pLayer, 0, m_iLayerSize*sizeof(T*));
    }
    return m_vUsedLayers[iLayer][i%m_iLayerSize];
}


//---------------------------------------------------------------------------
// operator[]
//  returns reference to i-th element (readable and writable)
//  if i is beyond the existing blocks, the required number of blocks
//  is added
//
template<class T>    
T &LayerBuf<T>::operator[](int i) const {
    //    size_t iLayer = i / m_iLayerSize;
    / *
    while (iLayer >= m_vUsedLayers.size()) {
        T* pLayer = createLayer();
        memset(pLayer, 0, m_iLayerSize*sizeof(T*));
    }
      * /
    return m_vUsedLayers[ i / m_iLayerSize][i%m_iLayerSize];
}
*/
//---------------------------------------------------------------------------
// size
//  returns capacity 
//
template<class T>
size_t LayerBuf<T>::size() {
    return m_iNumLayers*m_iLayerSize;
}

//---------------------------------------------------------------------------
// createLayer
//   creates new layer or reuses a free one, and returns it
//
template<class T>    
void LayerBuf<T>::createLayer() {
    T *pLayer = NULL;
    //    printf("Creating new layer\n");
    // if free blocks are available ...
    if (m_vFreeLayers.size() > 0) {
        // ... use one of them
        //        printf("Reusing block\n");
        pLayer = m_vFreeLayers.back();
        m_vFreeLayers.pop_back();
    } else {
        // ... otherwise create a new one
        //        printf("Creating block\n");
        pLayer = new T[m_iLayerSize];
        // if data is padded there may be uninitialised bytes
        // to prevent valgrind nag: initialize entire layer
        memset(pLayer, 37, m_iLayerSize*sizeof(T));
    }

    if ((m_iStrategy & MB_ZERO_BLOCKS) != 0) {
        memset(pLayer, 0, m_iLayerSize*sizeof(T));
    }
    m_vUsedLayers.push_back(pLayer);
    m_iNumLayers++;
}

//---------------------------------------------------------------------------
// freeAllLayers
//  removes all used layers from vector of used layers.
//  depending on strategy, layers are destroyed or saved for reuse in vector 
//  of free layers
//
template<class T>
void LayerBuf<T>::freeAllLayers() {
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        freeLayer(i);
    }
}

//---------------------------------------------------------------------------
// freeLayer
//  removes layer from vector of used layerss.
//  depending on strategy, layer is destroyed or saved for reuse in vector 
//  of free layer
//
template<class T>
void LayerBuf<T>::freeLayer(uint iIndex) {
    if (iIndex < m_vUsedLayers.size()) {
        T* pLayer = m_vUsedLayers[iIndex];
        //        printf("freeing u %p\n", pLayer); 
        
        bool bDestroy = false;
        unsigned int iDelay = (m_iStrategy & MB_DESTROY_DELAY_MASK);
        if ((iDelay !=  MB_DESTROY_LAZY) && (m_vFreeLayers.size() >= iDelay)) {
            bDestroy = true;
        }

        if (bDestroy) {
            //            printf("RealDestroy\n");
            delete[] pLayer;
        } else {
            m_vFreeLayers.push_back(pLayer);
        }
        m_vUsedLayers.erase(m_vUsedLayers.begin() + iIndex);
        m_iNumLayers--;
    }
}

//---------------------------------------------------------------------------
// elementShift
//
template<class T>
void LayerBuf<T>::elementShift(uint iTo, uint iFrom) {

    size_t iLayerTo   = iTo / m_iLayerSize;
    size_t iLayerFrom = iFrom / m_iLayerSize;
    size_t iIndexTo   = iTo % m_iLayerSize;
    size_t iIndexFrom = iFrom % m_iLayerSize;

    memcpy(&(m_vUsedLayers[iLayerTo][iIndexTo]), 
           &(m_vUsedLayers[iLayerFrom][iIndexFrom]), 
           sizeof(T));
}


//---------------------------------------------------------------------------
// moveElements
//
template<class T>
void LayerBuf<T>::moveElements(uint iToLayer,   uint iToIndex, 
                               uint iFromLayer, uint iFromIndex, 
                               uint iNum) {

    memcpy(&(m_vUsedLayers[iToLayer][iToIndex]), 
           &(m_vUsedLayers[iFromLayer][iFromIndex]), 
          iNum* sizeof(T));

}

//---------------------------------------------------------------------------
// copyBlock
//   Assumption: block has been compacted
template<class T>
int LayerBuf<T>::copyBlock(uint iStart, T *pBlock, uint iSize) {
    int iResult = 0;
    T *pCur = pBlock;

    uint iLayer = iStart / m_iLayerSize;
    uint iPos   = iStart % m_iLayerSize;

    uint iNum = m_iLayerSize-iPos;

    if (iSize < iNum) {
        iNum = iSize;
    }
    iSize -= iNum;

    //    printf("First Layer (%d) has %d free spaces\n", iLayer, m_iLayerSize-iPos);
    // do the first few
    //    printf("copy %u items of size %zd from %p (%p+%d) to layer %u pos %u (size %ld)\n", iNum, sizeof(T), pCur, pBlock, 0, iLayer, iPos, m_iLayerSize);
    memcpy(&(m_vUsedLayers[iLayer][iPos]), pCur, iNum*sizeof(T));
    
    while (iSize > 0) {
        pCur += iNum;
        iLayer++;
        iNum = (iSize < m_iLayerSize) ? iSize : m_iLayerSize;
        iSize -= iNum;
        //        printf("copy %u items of size %zd from %p (%p+%ld) to layer %u/%u pos 0 (size %ld)\n", iNum, sizeof(T), pCur, pBlock, pCur-pBlock, iLayer, getNumLayers(), m_iLayerSize);fflush(stdout);
        memcpy(&(m_vUsedLayers[iLayer][0]), pCur, iNum*sizeof(T));
    }
    return iResult;
}

//---------------------------------------------------------------------------
// showUsedLayers
//
template<class T>
void LayerBuf<T>::showUsedLayers() {
    printf("Used:");
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        printf("%p ", m_vUsedLayers[i]);
    }
    printf("\n");

}

//---------------------------------------------------------------------------
// showFreeLayers
//
template<class T>
void LayerBuf<T>::showFreeLayers() {
    printf("Free:");
    for (unsigned int i = 0; i < m_vFreeLayers.size(); i++) {
        printf("%p ", m_vFreeLayers[i]);
    }
    printf("\n");
}

#endif


