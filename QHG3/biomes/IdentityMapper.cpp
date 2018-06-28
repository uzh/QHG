#include "ValueMapper.h"
#include "IdentityMapper.h"


template<class T>
IdentityMapper<T>::IdentityMapper() {
}

template<class T>
IdentityMapper<T>::~IdentityMapper(){
}

template<class T>
T IdentityMapper<T>::mapValue(T t) { 
    return t;
}
