#ifndef __EQ_OGL_H__
#define __EQ_OGL_H__

#include "IcoSurface_OGL.h"
#include "EQsahedron.h"

class EQ_OGL : public IcoSurface_OGL {
public:
    EQ_OGL(EQsahedron *pEQ);
    virtual void drawFaces();
    void getCol(Vec3D *pV,  float fCol[4]);

    EQsahedron *m_pEQ;
};



#endif
