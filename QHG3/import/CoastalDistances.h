#ifndef __COASTALDISTANCES_H__
#define __COASTALDISTANCES_H__

#include <vector>
#include <map>
#include <set>

#include "types.h"

typedef std::vector<gridtype>        cellvec;
typedef std::set<gridtype>           cellset;
typedef std::map<gridtype, cellset>  cellrange;

typedef std::map<gridtype, int>      idindex;

typedef std::map<gridtype, double>   distlist;
typedef std::map<gridtype, distlist> distancemap;

typedef std::set<gridtype> neighbors;
typedef std::map<gridtype,neighbors> neighborlist;


typedef struct degbox {
    double dLonMin;
    double dLatMin;
    double dLonMax;
    double dLatMax;
    degbox(double dLonMin0, double dLatMin0, double dLonMax0, double dLatMax0)
        : dLonMin(dLonMin0), dLatMin(dLatMin0), dLonMax(dLonMax0), dLatMax(dLatMax0) {};
} degbox;

typedef std::vector<degbox>             boxlist;

class SCellGrid;

class CoastalDistances {
public:
    static CoastalDistances *createInstance(SCellGrid *pCG, double dMaxDist, boxlist &vBoxes, bool bVerbosity=false);
    ~CoastalDistances();
    int createNeighborList();

    void showDistances();
    void showDistances(const distancemap &mDistances);
    const distancemap  &getDistances()  { return m_mDistances;};
    const distancemap  &getReduced()    { return m_mReduced;};
    const cellvec      &getCoastCells() { return m_vCoastCells;};
    const neighborlist &getNeighbors()  { return m_mNeighbors;};
    void setVerbosity(bool bVerbose) { m_bVerbose = bVerbose;};
protected:
    CoastalDistances(SCellGrid *pCG, double dMaxDist, boxlist &vBoxes, bool bVerbosity=false);
    int init();


    bool isCoastalCell(int iIndex);
    int collectCoastCells();
    int findCoastalNeighborhoods();
    int removeConnectedComponents();
    int removeConnectedComponent(int iKey, cellset &sRange);
    int selectShortestComponentConnections();
    void getNeighborWater(int iCelliD, intset &sWater);

    SCellGrid   *m_pCG;
    cellvec      m_vCoastCells;
    int          m_iNumCells;
    double       m_dMaxDist;
    cellrange    m_mCoastRanges;
    distancemap  m_mDistances;
    distancemap  m_mReduced;
    idindex      m_mIdToIndex;
    neighborlist m_mNeighbors;
    boxlist      m_vBoxes;
    bool         m_bVerbose;
};


#endif
