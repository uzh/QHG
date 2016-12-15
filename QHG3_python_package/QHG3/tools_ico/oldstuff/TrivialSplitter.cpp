#include <stdio.h>
#include <math.h>
#include <algorithm>


#include "FullRegion.h"
#include "TrivialSplitter.h"


//-----------------------------------------------------------------------------
// constructor
//
TrivialSplitter::TrivialSplitter() {

}

//-----------------------------------------------------------------------------
// createRegions
//
Region **TrivialSplitter::createRegions(int *piNumTiles) {
    Region **pRegions = NULL;

    pRegions = new Region*[1];
    pRegions[0] = new FullRegion(0);
    *piNumTiles = 1;

    return pRegions;
}
