/*=============================================================================
| LBBase
|   Abstract base class for LayerBuf-style classes
|
| Author: Jody Weissmann
\============================================================================*/

#ifndef __LBBASE_H__
#define __LBBASE_H__

#include "types.h"

// virtual base class to handle collections of MemBlocks
class LBBase {
public:
    virtual void createLayer() = 0;
    virtual void freeLayer(uint iIndex) = 0;
    virtual void freeAllLayers() = 0;

    virtual uint getNumLayers() const = 0;
    virtual uint getLayerSize() const = 0;
    virtual void elementShift(uint iTo, uint iFrom) = 0;
    virtual void moveElements(uint iToLayer, uint iToIndex, 
                              uint iFromLayer, uint iFromInde, 
                              uint iNum) = 0;

    
};

#endif
