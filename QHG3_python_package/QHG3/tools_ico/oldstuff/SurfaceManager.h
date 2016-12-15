#ifndef __SURFACEMANAGER_H__
#define __SURFACEMANAGER_H__

class Surface;
class ValueProvider;

class IQOverlay;
class IQSurface_OGL;

#include "Observable.h"

#define STYPE_NONE -1
#define STYPE_ICO   0
#define STYPE_FLAT  1
#define STYPE_REG   2

class SurfaceManager : public Observable {

public:
    static SurfaceManager *createInstance(const char *pFile, const char *pDataFile, IQOverlay *pOverlay, bool bPreSel);
    SurfaceManager();
    ~SurfaceManager();

    Surface       *getSurface() { return m_pSurface;};
    IQSurface_OGL *getSurfaceOGL() { return m_pSurfaceOGL;};
    ValueProvider *getValueProvider() { return m_pValueProvider;};
    int           getType() { return m_iType;};

    int loadSurface(const char *pFile, const char *pDataFile, bool bPreSel);

    int loadData(const char *pFile, bool bForceCol);
    int addData(const char *pFile);
    void clearData();

    void setOverlay(IQOverlay *pOverlay) { m_pOverlay = pOverlay;};

protected:
    int loadSurfaceQDF(const char *pFile);
    int loadSurfaceSnap(const char *pFile, bool bPreSel);

    int loadIco(const char *pFile,bool bPreSel);
    int loadFlat(const char *pFile);
    int loadEQ(const char *pFile);

    Surface       *m_pSurface;
    IQSurface_OGL *m_pSurfaceOGL;
    ValueProvider *m_pValueProvider;
    int            m_iType;
    IQOverlay     *m_pOverlay;
    bool           m_bQDF;
};

#endif
