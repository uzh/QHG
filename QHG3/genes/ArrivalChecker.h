#ifndef __ARRIVALCHECKER_H__
#define __ARRIVALCHECKER_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "AnalysisUtils.h"


typedef std::pair<int,double>                         celldist;
typedef std::map<std::string, std::vector<celldist> > loccelldist;                 

class ArrivalChecker {
public:
    static ArrivalChecker *createInstance(const char *pQDFGrid, const char *pQDFStats, const char *pLocFile, double dDistance=0);
    ~ArrivalChecker();
    int init(const char *pQDFGrid, const char *pQDFStats, const char *pLocFile, double dDistance);
    int findClosestCandidates(bool bSpherical);
    void show(bool bFormat, bool bSort);
private:
    ArrivalChecker();
    int fillCoordMap(const char *pQDFGeoGrid);
    int readStats(const char *pQDFStats);
    int calcSphericalDistances();
    int calcCartesianDistances();

    void deleteArrays();
    
    uint m_iNumCells;
    arrpos_coords m_mCoords;
    int    *m_pCellIDs;
    loc_data m_mLocData;
    double *m_pTravelTimes;
    double *m_pTravelDists;

    loccelldist *m_pmCandidates;
    stringvec    m_vNames;
};

#endif
