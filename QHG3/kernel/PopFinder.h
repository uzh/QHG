#ifndef __POPFINDER_H__
#define __POPFINDER_H__

#include "PopBase.h"

class PopFinder {
public:
    virtual PopBase *getPopByID(idtype iSpeciesID)=0;
    virtual PopBase *getPopByName(const char *pSpeciesName)=0;
};

#endif
