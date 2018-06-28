#ifndef __ICOMODEL_H__
#define __ICOMODEL_H__

#define NUM_COLORS  20
#include <GL/glut.h>

#include "types.h"
#include "icoutil.h"

class Icosahedron;
class PolyFace;
class ValReader;
class LookUp;
class GridProjection;
class Vec3D;
class Surface;
class IcoSurface_OGL;


class IcoColorizer;


class IcoModel {


public:
    enum DisplayList {
        CUBE = 1,
        G_FORWARD,
        G_BACKWARD,
        T_FORWARD,
        T_BACKWARD,
        K_FORWARD,
        K_BACKWARD
    };
    
    static const float MAT_SPECULAR[4];
    static const float MAT_SHININESS[1];
    static const float MAT_BLACK[4];
    static const float MAT_COLORS[NUM_COLORS][4];
    
    
public:
    explicit IcoModel(const char *pFile);
    virtual ~IcoModel();

    void setSurface(Surface *pSurface, IcoSurface_OGL *pSurfaceOGL);

    IcoSurface_OGL *getSurfaceOGL() { return m_pSurfaceOGL;};
    Surface        *getSurface()    { return m_pSurface;};

private:
    void init_gl();
    

public:
    void draw();
    
    
    void setBox(bool bBox) {m_bBox = bBox;};
    void setWireFrame(bool bMode) {m_iWF = bMode?GL_LINE:GL_FILL;};
    bool getWireFrame() {return (m_iWF == GL_LINE);};
    //   void setPolyType(int iPolyType);
    void toggle_color();
    void setDisplayLevel(int iDispLevel);
    void toggleDisplayLevel(int iDispLevel);
    
    void locate(double dLon, double dLat, gridtype *piNode);
    void clearFace() { m_pFSpecial = NULL; };
    void setSubdivision(int iLevel);

    void refreshImage();
    ValReader *getValReader() { return m_pVR;};
private:
 
    void buildNodeFaces();
    void drawFaces();

    void drawFan(Vec3D &A, Vec3D &B);
    void drawDisc(double dLonMin, double dLonMax, double dLat);
    void outlineTriangle(PolyFace *pF);
    void buildObject();
private:
    
    bool m_bColor;
    
    bool m_bInitialized;
    bool m_bHaveObject;
    Surface        *m_pSurface;
    IcoSurface_OGL *m_pSurfaceOGL;

    ValReader *m_pVR;
    LookUp *m_pLookUp;

    int m_iWF;
    int m_iDispLevel;
    bool m_bBox;
    PolyFace *m_pFSpecial;

    bool m_bHaveList;
    GLuint m_iListBase;
    int m_iCurSubDivLevel;

    bool m_bDual;

    IcoColorizer *m_pCol;
    GLUquadricObj *m_pqobj;
};

#endif
