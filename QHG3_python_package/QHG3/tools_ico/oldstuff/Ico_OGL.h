#ifndef __ICO_OGL_H__
#define __ICO_OGL_H__

#include "types.h"
#include "IcoSurface_OGL.h"
#include "Icosahedron.h"

class Ico_OGL : public IcoSurface_OGL {
public:
    Ico_OGL(Icosahedron *pIco);
    virtual void drawFaces();
    void getCol(Vec3D *pV,  float fCol[4]);

    Icosahedron *m_pIco;
};



#endif
