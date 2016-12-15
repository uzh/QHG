#ifndef __ICOSURFACE_OGL_H__
#define __ICOSURFACE_OGL_H__

#include "IcoColorizer.h"

class IcoSurface_OGL {
public:
    IcoSurface_OGL() : m_pCol(NULL) { }
    virtual ~IcoSurface_OGL() {}
    virtual void drawFaces()=0;
    virtual void setColorizer(IcoColorizer *pCol) { m_pCol = pCol;};
    IcoColorizer   *m_pCol;

};



#endif
