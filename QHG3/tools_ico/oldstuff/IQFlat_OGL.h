#ifndef __IQFLAT_OGL_H__
#define __IQFLAT_OGL_H__

#include <set>
#include "types.h"
#include "IQSurface_OGL.h"
#include "Lattice.h"

class IQOverlay;

class IQFlat_OGL : public IQSurface_OGL {
public:
    IQFlat_OGL(Lattice *pLattice, IQOverlay *pOverlay);
    virtual void drawSurfaceLines();
    virtual void drawNodePoints();
    virtual void drawNodeFaces();
    virtual void drawNodeHex();

    //    void getCol(gridtype lNode, float fCol[4], float *pfScale);

    Lattice *m_pLattice;

    void drawNodeHexHoriPlane(VertexLinkage *pVL, double dOffsX, double dOffs, bool bTop);
    void drawNodeHexSides(VertexLinkage *pVL, double dOffsX, double dOffs);


};



#endif
