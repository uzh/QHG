#ifndef __ICOLOC_H__
#define __ICOLOC_H__

#include "types.h"
#include "icoutil.h"

class IcoLoc {
public:
    virtual ~IcoLoc() {};
    virtual gridtype findNode(double dLon, double dLat)=0;
    virtual bool   findCoords(int iNodeID,double *pdLon, double *pdLat)=0;
};


#endif
