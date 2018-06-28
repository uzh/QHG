#ifndef __VALUEMAPPER_H__
#define __VALUEMAPPER_H__

template<class T> class ValueMapper {
public:
    virtual ~ValueMapper() {};
    virtual T mapValue(T)=0;
};

#endif
