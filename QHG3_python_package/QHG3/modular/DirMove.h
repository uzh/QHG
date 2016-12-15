#ifndef __DIRMOVE_H__
#define __DIRMOVE_H__

#include "Action.h"
#include "PolyLine.h"

#define DIRMOVE_NAME "DirMove"

template<typename T>
class DirMove : public Action<T> {
    
 public:
    DirMove(SPopulation<T> *pPop, SCellGrid *pCG);
    ~DirMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
    int tryReadParamLine(char *pLine);

 protected:
 

};

#endif
