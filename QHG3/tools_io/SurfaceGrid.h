#ifndef __SURFACEGRID_H__
#define __SURFACEGRID_H__

class SCellGrid;
class Surface;

class SurfaceGrid {
public:
    SurfaceGrid();
    static SurfaceGrid *createInstance(const char *pQDF);

    virtual ~SurfaceGrid();

    SCellGrid *getCellGrid() { return m_pCG;};
    Surface *getSurface() { return m_pSurf;};

protected:
    int init(const char *pQDF); 


    int createCellGrid(const char *pQDF);

    int createSurface();

    SCellGrid *m_pCG;
    Surface   *m_pSurf;
};


#endif
