/*============================================================================
| LayerArrBuf
|  
|  A resizable array derived from LayerBuf, designed to hold arrays instead
|  of simple types. It's sort of an array of arrays
|
|  Allows access to individual arrays with the []-operator.
|
|  Author: Jody Weissmann
\===========================================================================*/

#include <stdio.h>
#include <string.h>

#include "LBBase.h"
#include "LayerBuf.h"
#include "LayerBuf.cpp"
#include "LayerArrBuf.h"

//---------------------------------------------------------------------------
// constructor
//
template<class T>
LayerArrBuf<T>::LayerArrBuf(uint iLayerSize, uint iArraySize, int iStrategy)
    : LayerBuf<T>(iLayerSize*iArraySize, iStrategy),
      m_iArraySize(iArraySize) {
}


//---------------------------------------------------------------------------
// constructor
//
template<class T>
LayerArrBuf<T>::LayerArrBuf()
    : LayerBuf<T>(),
      m_iArraySize(0) {
}

//---------------------------------------------------------------------------
// init
//
template<class T>
void LayerArrBuf<T>::init(uint iLayerSize, uint iArraySize, int iStrategy) {
    LayerBuf<T>::init(iLayerSize*iArraySize, iStrategy);
    m_iArraySize = iArraySize;
}


//---------------------------------------------------------------------------
// size
//
template<class T>
size_t LayerArrBuf<T>::size() { 
    return LayerBuf<T>::size()/m_iArraySize;
}

//---------------------------------------------------------------------------
// getLayerSize
//
template<class T>
uint LayerArrBuf<T>::getLayerSize() const { 
    return LayerBuf<T>::getLayerSize()/m_iArraySize;
}

//---------------------------------------------------------------------------
// elementShift
//
template<class T>
void LayerArrBuf<T>::elementShift(uint iTo, uint iFrom) { 
    iTo   *= m_iArraySize;
    iFrom *= m_iArraySize;

    size_t iLayerTo   = iTo / LayerBuf<T>::m_iLayerSize;
    size_t iLayerFrom = iFrom / LayerBuf<T>::m_iLayerSize;
    size_t iIndexTo   = iTo % LayerBuf<T>::m_iLayerSize;
    size_t iIndexFrom = iFrom % LayerBuf<T>::m_iLayerSize;

    memcpy(&(LayerBuf<T>::m_vUsedLayers[iLayerTo][iIndexTo]), 
           &(LayerBuf<T>::m_vUsedLayers[iLayerFrom][iIndexFrom]), 
           m_iArraySize*sizeof(T));
};

//---------------------------------------------------------------------------
// moveElements
//
template<class T>
void LayerArrBuf<T>::moveElements(uint iToLayer,   uint iToIndex, 
                               uint iFromLayer, uint iFromIndex, 
                               uint iNum) {

    iToIndex   *= m_iArraySize;
    iFromIndex *= m_iArraySize;
    memcpy(&(LayerBuf<T>::m_vUsedLayers[iToLayer][iToIndex]), 
           &(LayerBuf<T>::m_vUsedLayers[iFromLayer][iFromIndex]), 
          iNum*m_iArraySize*sizeof(T));

}


//---------------------------------------------------------------------------
// copyBlock
//   Assumption: block has been compacted
template<class T>
int LayerArrBuf<T>::copyBlock(uint iStart, T *pBlock, uint iSize) {
    int iResult = LayerBuf<T>::copyBlock(iStart*m_iArraySize, pBlock, iSize*m_iArraySize);
    return iResult;
}


//---------------------------------------------------------------------------
// copyLayer
//
template<class T>
int  LayerArrBuf<T>::copyLayer(int iDestLayer, const T *pData) {
    int iResult = -1;
    if ((iDestLayer >= 0) && (iDestLayer < (int)  LayerBuf<T>::m_vUsedLayers.size())) {
        memcpy(&(LayerBuf<T>::m_vUsedLayers[iDestLayer][0]), pData, LayerBuf<T>::getLayerSize()*sizeof(T));
        iResult = 0;
    }
    return iResult;
}
