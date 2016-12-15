#ifndef __SURFACE_H__
#define __SURFACE_H__

#include <map>
#include <set>
#include "icoutil.h"
#include "IcoLoc.h"

class IcoGridNodes;
class Surface_OGL;
class PolyFace;
class Vec3D;

class Surface  {
public:
    virtual ~Surface() {};
    virtual int            load(const char *pFileName)=0;
    virtual int            save(const char *pFileName)=0;
        
    virtual gridtype findNode(double dLon, double dLat)=0;
    virtual gridtype findNode(Vec3D *pv)=0;
    virtual PolyFace *findFace(double dLon, double dLat)=0;
    virtual Vec3D *getVertex(gridtype lID)=0; 
    virtual int collectNeighborIDs(gridtype iID, int iDist, std::set<gridtype> & sIds)=0;
    virtual tbox *getBox()=0;

    virtual void display()=0;

};


#endif

