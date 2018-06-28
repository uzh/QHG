#ifndef __SEQUENCE_PROVIDER_H__
#define __SEQUENCE_PROVIDER_H__

#include "types.h"

template<typename T>
class SequenceProvider {
public:

    virtual int getSequenceSize() = 0;
    virtual const T *getSequence(idtype iID) = 0;
};

#endif
