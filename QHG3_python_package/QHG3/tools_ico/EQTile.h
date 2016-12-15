#ifndef __EQTILE_H__
#define __EQTILE_H__


#include "types.h"
#include "IcoNode.h"
#include "BasicTile.h"

class EQTile : public BasicTile  {
public:
    EQTile(int iID, intset &sNodeIDs);
    virtual ~EQTile() {};

    virtual bool contains(IcoNode *pBC);
    //    virtual bool contains(gridtype iID);
    virtual void display() {};
    virtual unsigned char *serialize() {return NULL;};
    virtual int deserialize(unsigned char *pBuffer) {return 0;};

protected:
    intset m_sNodeIDs;
    
};


#endif
