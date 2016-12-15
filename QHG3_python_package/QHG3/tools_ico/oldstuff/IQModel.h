#ifndef __IQMODEL_H__
#define __IQMODEL_H__

#define NUM_COLORS  20
#define MAX_LU_PARAMS 5

#include <set>
#include <GL/gl.h> 
#include "types.h"
#include "Observer.h"
#include "Observable.h"
#include "icoutil.h"


class PolyFace;
class IcoFace;
class ValReader;
class LookUp;
class Vec3D;
class ProjInfo;
class ValueProvider;
class IQSurface_OGL;
class IQColorizer;
class Surface;
class SurfaceManager;

class IQModel : public Observer, public Observable {


public:
    
    static const float MAT_SPECULAR[4];
    static const float MAT_SHININESS[1];
    static const float MAT_BLACK[4];
    static const float MAT_COLORS[NUM_COLORS][4];
    
public:
    explicit IQModel(SurfaceManager *pSurfaceManager, ProjInfo *pPI);
    virtual ~IQModel();


private:
    void init_gl();

public:
    void draw();
    
    
    void setBox(bool bBox) {m_bBox = bBox;};
    void setWireFrame(bool bMode) {m_iWF = bMode?GL_LINE:GL_FILL;};
    bool getWireFrame() {return (m_iWF == GL_LINE);};
    void toggle_color();
    

    void locate(double dLon, double dLat, gridtype *piNode);
    void clearFace() { m_pFSpecial = NULL; };

    void setSurface();
    void setValueProvider(bool bForceCol);
    void setSurfaceAndValues(bool bForceCol);

    void select(int x, int y);
    void clearMarkers();


    void setPaintMode(int iDisp);
    void toggleHex();
    void toggleAxis() { m_bAxis = !m_bAxis; };
    void toggleLighting();
    void setUseLight(bool bLighting);
    void toggleAlt();
    void setUseAlt(bool bUseAlt);
    // from Observer
    void notify(Observable *pObs, int iType, const void *pCom);
private:
  
    void buildNodeFaces();
    void buildNodeHex();
  
    void outlineTriangle(PolyFace *pF);
    void drawAxis();
    void drawHitLine();
private:
    SurfaceManager *m_pSurfaceManager;
    unsigned int m_RotCount;
    
    unsigned int m_Mode;
    bool m_bColor;
    
    bool m_bInitialized;
    bool m_bHaveObject;
    Surface *m_pSurface;
    IQSurface_OGL *m_pSurfaceOGL;

    ValReader *m_pVR;
    LookUp *m_pInitLookUp;

    int m_iWF;
    bool m_bBox;
    PolyFace *m_pFSpecial;
    Vec3D *m_pvHitNode;
    std::set<gridtype> m_sNeighbors;


    ValueProvider *m_pValueProvider;
    int       m_iCurDataType;
    IQColorizer *m_pCol;


    int m_iDisp;

    ProjInfo *m_pPI;
    bool m_bDrawing;
    bool m_bHaveList;
    GLuint m_iListBase;
    bool m_bHex;
    bool m_bAxis;
    bool m_bPreSel;

    bool m_bLoading;
    int m_iFirst;

};

#endif
