#ifndef __SIMPLEMAPPER_H__
#define __SIMPLEMAPPER_H__

#include <map>
#include "ValueMapper.h"


template<class T> class SimpleMapper : public ValueMapper<T> {
public:
    typedef typename std::map<T,T> TRANS_MAP;

    SimpleMapper(TRANS_MAP &mapTrans, T defval); 
    SimpleMapper(const char *pTranslationFile, T defval); 
    virtual ~SimpleMapper(){};

    virtual T mapValue(T t);
private:
    TRANS_MAP m_mapTrans;
    T m_tDefVal;
};

#endif

