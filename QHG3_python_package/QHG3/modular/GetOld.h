#ifndef __GETOLD_H__
#define __GETOLD_H__

#include "Action.h"

#define GETOLD_NAME "GetOld"

class WELL512;

template<typename T>
class GetOld : public Action<T> {
    
 public:
    GetOld(SPopulation<T> *pPop, SCellGrid *pCG);
    ~GetOld();
    int operator()(int iA, float fT);

};

#endif
