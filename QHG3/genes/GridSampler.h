#ifndef __GRIDSAMPLER_H__
#define __GRIDSAMPLER_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"

typedef std::pair<double, double> distpair;
typedef std::vector<distpair>     distpairs;

typedef std::pair<double, double> coords;
typedef std::vector<idtype> vagidx;        // agentIndexes
typedef std::map<coords, vagidx> mgridags; // <lon,lat> => vec(<agentID, agentIndex>)

class GridSampler {

public:
    static GridSampler *createInstance(const char *pQDFGeo, const char *pQDFStat, const char *pQDFPop, const char *pSpecies);
    virtual ~GridSampler();

    int findCandidates(double dDlon, double dDLat, double dR);
    int selectAtGrids(uint iNSel);
    int calcGeoGenomeDists(ulong *pRefGenome);

    int loadGenomes();
    int writeFile(const char *pOutput);
    int getGenomeSize() {return m_iGenomeSize;};
private:
    int init(const char *pQDFGeo, const char *pQDFStat, const char *pQDFPop, const char *pSpecies);

    GridSampler();
    int loadArrays();
    int loadPopArrays();
    int loadStatArrays();
    int loadGeoArrays();
    double *loadGeoArray(const char *pDataSet);
    void deleteArrays();
    void findClosestGrid(double dLon, double dLat, double *pdGLon, double *pdGLat);
    double m_dDLon;
    double m_dDLat;
    double m_dR;
     
    int m_iNumCells;
    int m_iNumAgents;
    int m_iGenomeSize;
    char *m_pQDFGeo;
    char *m_pQDFStat;
    char *m_pQDFPop;
    char *m_pSpecies;
    char *m_pRefFile;
    double *m_pdLongs;
    double *m_pdLats;
    double *m_pdAlts;
    idtype *m_pAgentIDs;
    int    *m_pCellIDs;
    double *m_pdDists;
    ulong  **m_ppGenomes;

    mgridags   m_mvSelected;
    mgridags   m_mGridAgents;
    distpairs  m_vDistPairs;
    std::vector<coords> m_vCoords;
};

#endif

