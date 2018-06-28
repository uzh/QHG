#ifndef __IQICO_OGL_H__
#define __IQICO_OGL_H__

#include "types.h"
#include "IQSurface_OGL.h"
#include "Icosahedron.h"

class IQOverlay;

class IQIco_OGL : public IQSurface_OGL {
public:
    IQIco_OGL(Icosahedron *pIco, IQOverlay *pOverlay);
    virtual void drawSurfaceLines();
    virtual void drawNodePoints();
    virtual void drawNodeFaces();
    virtual void drawNodeHex();

    //    void getCol(gridtype lNode, float fCol[4], float *pfScale);

    Icosahedron *m_pIco;

protected:
    void drawNodeFacesFlat();
    void drawNodeFacesAlt();
};



#endif
