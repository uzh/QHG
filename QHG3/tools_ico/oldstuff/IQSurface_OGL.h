#ifndef __IQSURFACE_OGL_H__
#define __IQSURFACE_OGL_H__

#include "types.h"
#include "IQColorizer.h"
#include "ValueProvider.h"


#define COL_RED      0
#define COL_GREEN    1
#define COL_BLUE     2
#define COL_YELLOW   3
#define COL_CYAN     4
#define COL_MAGENTA  5
#define COL_WHITE    6
#define COL_ORANGE   7
#define COL_PURPLE   8
#define COL_LEMON    9
#define COL_LILA    10
#define COL_GRAY    11
#define COL_DGRAY   12
#define COL_WINE    13
#define COL_LGREEN  14
#define COL_BUTTER  15
#define COL_BROWN   16
#define COL_DGREEN  17
#define COL_SKIN    18
#define COL_SKY     19

#define NUM_COLORS  20
class IQOverlay;

class IQSurface_OGL {
public:
    IQSurface_OGL(IQOverlay *pOverlay);
    virtual ~IQSurface_OGL() {}
    virtual void drawSurfaceLines()=0;
    virtual void drawNodePoints()=0;
    virtual void drawNodeFaces()=0;
    virtual void drawNodeHex()=0;

    virtual void setColorizer(IQColorizer *pCol) { m_pCol = pCol;};
    virtual void setValueProvider(ValueProvider *pVP) { m_pVP = pVP;};
    virtual void setAltData(float fAltFactor, double fMinLevel, double fMaxLevel);
    virtual void getCol(gridtype lNode, float fCol[4], float *pfScale);

    void toggleLighting();
    void setUseLight(bool bUseLight);
    void toggleAlt();
    void setUseAlt(bool bUseAlt);

    IQColorizer   *m_pCol;
    IQOverlay     *m_pOverlay;
    ValueProvider *m_pVP;
    float          m_fAltFactor;
    double         m_fMinLevel;
    double         m_fMaxLevel;

    bool m_bUseAlt;
    bool m_bUseLight;
    int m_iMatType;

    static const float MAT_COLORS[NUM_COLORS][4];

};



#endif
