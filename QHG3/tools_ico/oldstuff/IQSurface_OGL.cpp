#include <GL/gl.h>
#include <GL/glut.h>

#include "utils.h"
#include "IQOverlay.h"
#include "IQSurface_OGL.h"


const float IQSurface_OGL::MAT_COLORS[NUM_COLORS][4] = {
    { 1.0, 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0 },
    { 0.0, 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 0.0, 1.0, 1.0, 1.0 },
    { 1.0, 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0, 1.0 },
    { 1.0, 0.5, 0.0, 1.0 },
    { 0.5, 0.0, 1.0, 1.0 },
    { 0.7, 1.0, 0.0, 1.0 },
    { 0.7, 0.0, 1.0, 1.0 },
    { 0.7, 0.7, 0.7, 1.0 },
    { 0.5, 0.5, 0.5, 1.0 },
    { 0.65, 0.15, 0.5, 1.0 },
    { 0.7, 1.0, 0.6, 1.0 },
    { 1.0, 0.9, 0.2, 1.0 },
    { 0.5, 0.4, 0.1, 1.0 },
    { 0.2, 0.3, 0.1, 1.0 },
    { 0.9, 0.6, 0.6, 1.0 },
    { 0.5, 0.8, 1.0, 1.0 },
};

//-----------------------------------------------------------------------------
// constructor
//
IQSurface_OGL::IQSurface_OGL(IQOverlay *pOverlay) 
    : m_pCol(NULL), 
      m_pOverlay(pOverlay),
      m_pVP(NULL), 
      m_fAltFactor(1), 
      m_fMinLevel(0), 
      m_fMaxLevel(1),
      m_bUseAlt(false),
      m_bUseLight(false),
      m_iMatType(GL_EMISSION) {
    

}

//-----------------------------------------------------------------------------
// setAltData
//
void IQSurface_OGL::setAltData(float fAltFactor, double fMinLevel, double fMaxLevel) { 
    m_fAltFactor = fAltFactor; 
    m_fMinLevel = fMinLevel; 
    m_fMaxLevel = fMaxLevel;
}

//-----------------------------------------------------------------------------
//  toggle_lighting
//
void IQSurface_OGL::toggleLighting() {
    setUseLight(!m_bUseLight);
}

//-----------------------------------------------------------------------------
//  toggleAlt
//
void IQSurface_OGL::toggleAlt() {
    setUseAlt(!m_bUseAlt);
}


//-----------------------------------------------------------------------------
// setUseLight
//
void IQSurface_OGL::setUseLight(bool bUseLight) {
    
    m_bUseLight = bUseLight;
    float fCol[] = {0, 0, 0, 1};
    if (m_bUseLight) {
        glEnable(GL_LIGHT0);
        m_iMatType = GL_AMBIENT_AND_DIFFUSE;
        glMaterialfv(GL_FRONT, GL_EMISSION, fCol);

    } else {
        glDisable(GL_LIGHT0);
        m_iMatType = GL_EMISSION;
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, fCol);
    }

}

//-----------------------------------------------------------------------------
//  setUseAlt
//
void IQSurface_OGL::setUseAlt(bool bUseAlt) {
    m_bUseAlt = bUseAlt;
}

//----------------------------------------------------------------------------
// getCol
//
void IQSurface_OGL::getCol(gridtype lNode, float fCol[4], float *pfScale) {
    double dVal = dNaN; 
    if (m_pOverlay->hasData() && (m_pOverlay->contains(lNode))) {
        fCol[0]  = 0; // overlaycolR
        fCol[1]  = 0; // overlaycolG  
        fCol[2]  = 0; // overlaycolB
        fCol[3]  = 1.0; // overlaycolB
    } else {
        if (m_pVP != NULL) {
            dVal = m_pVP->getValue(lNode);
        }
        
        if (!isnan(dVal) && m_bUseAlt) {
            *pfScale = 1+m_fAltFactor*(dVal - m_fMinLevel)/(m_fMaxLevel-m_fMinLevel);
        } else {
            *pfScale = 1;
        }
        
        m_pCol->getCol(dVal, fCol);
    }
}
