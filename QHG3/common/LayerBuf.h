/*============================================================================
| LayerBuf
|  
|  Resizable arrays of any type, implemented as std::vector of arrays of T,
|  so-called "layers". When more space is needed, new layers are added to the
|  vector.
|
|  Allows element access with the []-operator.
|
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __LAYERBUF_H__
#define __LAYERBUF_H__

#include <glob.h>
#include <vector>
#include <map>
#include "types.h"
#include "LBBase.h"

const uint MB_DESTROY_EAGER      = 0x00000000;
const uint MB_DESTROY_DELAY      = 0x00000001;
const uint MB_DESTROY_DELAY1     = 0x00000002;
const uint MB_DESTROY_LAZY       = 0x00000080;
const uint MB_DESTROY_DELAY_MASK = 0x000000ff;

const uint MB_ZERO_BLOCKS    = 0x00001000;


template<class T>
class LayerBuf : public LBBase {
public:
    LayerBuf(uint iLayerSize, int iStrategy=MB_DESTROY_DELAY1);
    LayerBuf();

    virtual ~LayerBuf();
    
    void init(uint iLayerSize, int iStrategy=MB_DESTROY_DELAY1);


    virtual inline
    T &operator[](uint i) const { typename std::vector<T*>::const_iterator it = m_vUsedLayers.begin()+(i/m_iLayerSize); return (*it)[i%m_iLayerSize];};


    ///    virtual inline
    ///    T &operator[](int i) const { return m_vUsedLayers[i/m_iLayerSize][i%m_iLayerSize];};

    virtual size_t size();

    void freeLayer(uint iIndex);
    void freeAllLayers();
       
    // debugging
    void showUsedLayers();
    void showFreeLayers();

    void createLayer();

    uint getNumLayers() const {return m_iNumLayers;};
    virtual uint  getLayerSize() const {return m_iLayerSize;};
    virtual void elementShift(uint iTo, uint iFrom);
    virtual void moveElements(uint iToLayer,   uint iToIndex, 
                              uint iFromLayer, uint iFromIndex, 
                              uint iNum);
    // for hdf5 writing
    size_t getNumUsedLayers() const {return m_vUsedLayers.size();};
    virtual int copyBlock(uint iStart, T *pBlock, uint iSize);

    virtual const T *getLayer(uint i) { return m_vUsedLayers[i];};
    virtual int copyLayer(int iDestLayer, const T *pData);
protected:
     
    uint             m_iLayerSize;
    uint             m_iNumLayers;
    std::vector<T*>  m_vUsedLayers;
    std::vector<T*>  m_vFreeLayers;
    int              m_iStrategy;
};

#endif
