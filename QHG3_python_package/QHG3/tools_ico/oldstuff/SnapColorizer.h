#ifndef __SNAPCOLORIZER_H__
#define __SNAPCOLORIZER_H__

#include "IQColorizer.h"


class SnapColorizer : public IQColorizer {
public:
    SnapColorizer();
    virtual void getCol(double dVal, float fCol[4]); 
  
};


#endif

