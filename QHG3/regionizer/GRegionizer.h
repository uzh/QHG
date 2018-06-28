#ifndef __GREGIONIZER_H__
#define __GREGIONIZER_H__

#include <vector>
#include <hdf5.h>
#include "QDFUtils.h"
#include "GridReader.h"

class SCellGrid;
class GRegion;

#define MODE_ALL      1
#define MODE_RAND     2
#define MODE_SMALLEST 3
#define MODE_LARGEST  4


class GRegionizer {
public:
    // start from files
    static GRegionizer *createInstance(const char *pQDFGrid, const char *pQDFPop, const char *pSpecies);
    // start if you already have the necessary data
    static GRegionizer *createInstance(SCellGrid *pCG, int *piAgentsPerCell, int *piMarks);

    ~GRegionizer();
    
    // region "seeds"
    int initializeRegions(int iNumRegions, int *piInitial, int iNumInitial);

    // one growth step
    int growStep(int iMode);

    // write cellgrid plus array of markings to QDF
    int writeRegions(const char *pOutputQDF);

    // display regions
    void showRegions();

    // timing    
    double getNFTime() { return m_dNeighborFinding;};

protected:
    GRegionizer();
    int init(const char *pQDFGrid, const char *pQDFPop, const char *pSpecies);
    int init(SCellGrid *pCG, int *piAgentsPerCell, int *piMarks);
    int setGrid(hid_t hGrid);
    int setPop(hid_t hPop, const char *pSpecies);

    bool m_bDeleteObjects;
    SCellGrid *m_pCG;
    int       *m_piAgentsPerCell;
    int       *m_piMarks;
    std::vector<GRegion*> m_vGRegions;

    bool m_bTimeSeed;
    double m_dNeighborFinding;
};



#endif
