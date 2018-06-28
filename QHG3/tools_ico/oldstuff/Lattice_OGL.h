#ifndef __LATTICE_OGL_H__
#define __LATTICE_OGL_H__

#include "IcoSurface_OGL.h"
#include "Lattice.h"

class Lattice_OGL : public IcoSurface_OGL {
public:
    Lattice_OGL(Lattice *pLattice);
    virtual void drawFaces();
    void getCol(Vec3D *pV,  float fCol[4]);

    Lattice *m_pLattice;
};



#endif
