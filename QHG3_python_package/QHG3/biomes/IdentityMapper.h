#ifndef __IDENTITYMAPPER_H__
#define __IDENTITYMAPPER_H__

#include <map>
#include "ValueMapper.h"


template<class T> class IdentityMapper : public ValueMapper<T> {
public:

    IdentityMapper();
    virtual ~IdentityMapper();

    virtual T mapValue(T t);
};

#endif

