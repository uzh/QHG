#ifndef __SIMPLEFACE_H__
#define __SIMPLEFACE_H__

#include "types.h"
#include "PolyFace.h"

template<int N>
class SimpleFace : public PolyFace {
public:
    SimpleFace();
    virtual ~SimpleFace();

    virtual int     getNumVertices() { return N;};
    virtual Vec3D  *getVertex(int iIndex) { return m_apVerts[iIndex]; };
    virtual gridtype  getVertexID(int iIndex){ return m_alIDs[iIndex]; };
    virtual double  getArea(){ return m_dArea;}; 
    virtual double *getLons() { return m_adLons; };
    virtual double *getLats() { return m_adLats; };
    virtual Vec3D  *getNormal() { return m_pNormal;};
    virtual int     getLevel() { return 0;};
    virtual PolyFace *contains(Vec3D *pP)=0;
    virtual Vec3D  *closestVertex(Vec3D *pP);
    virtual gridtype  closestVertexID(Vec3D *pP);
    virtual void    planify(Vec3D *pV) {pV->m_fZ = 0;};


    Vec3D  *m_apVerts[N];
    gridtype  m_alIDs[N]; 
    Vec3D  *m_pNormal;
    double  m_dArea;
    double  m_adLons[N];
    double  m_adLats[N];

};

#endif
