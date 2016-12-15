#ifndef __IQEQICO_OGL_H__
#define __IQEQICO_OGL_H__

#include "types.h"
#include "IQSurface_OGL.h"
#include "EQsahedron.h"

class IQOverlay;

class IQEQIco_OGL : public IQSurface_OGL {
public:
    IQEQIco_OGL(EQsahedron *pEQ, IQOverlay *pOverlay);
    virtual void drawSurfaceLines();
    virtual void drawNodePoints();
    virtual void drawNodeFaces();
    virtual void drawNodeHex();

    //    void getCol(gridtype lNode, float fCol[4], float *pfScale);

    EQsahedron *m_pEQ;

protected:
    void drawNodeFacesFlat();
    void drawNodeFacesAlt();
};



#endif
