#ifndef __SHPOBJECT_H__
#define __SHPOBJECT_H__

#include "shpUtils.h"


class shpObject {
public:
    virtual int read()=0;
    virtual void display(const char*pCaption)=0;
};

#endif
