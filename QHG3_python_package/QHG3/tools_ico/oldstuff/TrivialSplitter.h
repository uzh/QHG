#ifndef __TRIVIALSPLITTER_H__
#define __TRIVIALSPLITTER_H__

#include "RegionSplitter.h"

class Region;


class TrivialSplitter : public RegionSplitter {
public:
    TrivialSplitter();

    //    virtual ~RegionSplitter()
    virtual Region **createRegions(int *piNumTiles);


};


#endif

