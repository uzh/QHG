#ifndef __POLYFACE_H__
#define __POLYFACE_H__ 

#include "types.h"

class Vec3D;

class PolyFace {
public:
    virtual ~PolyFace(){};
    virtual int     getNumVertices()=0;
    virtual Vec3D  *getVertex(int iIndex)=0;
    virtual gridtype  getVertexID(int iIndex)=0;
    virtual double  getArea()=0;
    virtual double *getLons()=0;
    virtual double *getLats()=0;
    virtual Vec3D  *getNormal()=0;
    virtual int     getLevel()=0;
    virtual PolyFace *contains(Vec3D *pP)=0;
    virtual Vec3D *closestVertex(Vec3D *pP)=0;
    virtual gridtype  closestVertexID(Vec3D *pP)=0;
    virtual void    planify(Vec3D *pV)=0;
};


#endif
