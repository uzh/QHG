#ifndef __FULLREGION_H__
#define __FULLREGION_H__

#include "Region.h"

class IcoNode;

class FullRegion: public Region {
public:
    FullRegion(int iID);
    virtual ~FullRegion() {};

    virtual bool contains(IcoNode *pBC) { return true;};
    virtual void display();
    virtual unsigned char *serialize();
    virtual int deserialize(unsigned char *pBuffer);
    virtual int dataSize() { return Region::dataSize(); };

};


#endif

